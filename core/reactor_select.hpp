#ifndef __NET_CORE_REACTOR_SELECT_HPP__
#define __NET_CORE_REACTOR_SELECT_HPP__

#include "socket_type.hpp"
#include "socket_event_type.hpp"
#include "socket_interrupter.hpp"
#include "socket_initializer.hpp"

namespace mininet { namespace core {

class select_type {
    friend class reactor_type;
public:
    explicit select_type(socket_event_pool_type& socket_events)
        : socket_events_(socket_events)
    { }
public:
    void update_timer(uint64_t ms, bool again)
    {
        interrupter_.interrupt();
    }
    bool insert(socket_event_type* event)
    {
        max_fd_ = event->fd() > max_fd_ ? event->fd() : max_fd_;
        interrupter_.interrupt();
        return false;
    }
    bool update(socket_event_type* event)
    {
        interrupter_.interrupt();
        return false;
    }
    bool cancel(socket_event_type* event)
    {
        interrupter_.interrupt();
        return false;
    }
    void run(int64_t timeout)
    {
        timeval tv = { 0, 0 };
        tv.tv_sec = static_cast<long>(timeout) / 1000;
        tv.tv_usec = static_cast<long>(timeout) % 1000 * 1000;
        reset_fd_sets__();
        select(max_fd_ + 1, &read_fds_, &write_fds_, &except_fds_, &tv);
        check_fd_sets__();
    }
private:
    inline void init_fd_set__()
    {
        FD_ZERO(&read_fds_);
        FD_ZERO(&write_fds_);
        FD_ZERO(&except_fds_);
    }
    inline void reset_fd_set__(socket_event_ptr socket)
    {
        if (socket->readable()) FD_SET(socket->fd(), &read_fds_); 
        if (socket->writable()) FD_SET(socket->fd(), &write_fds_);
        FD_SET(socket->fd(), &except_fds_);
    }
    inline void reset_fd_sets__()
    {
        init_fd_set__();
        for (auto it = socket_events_.begin(); it != socket_events_.end(); ++it) {
            reset_fd_set__(it->second);
        }
    }
    inline bool check_fd_set__(socket_event_ptr socket)
    {
        uint32_t events = 0;
        if (socket->fd() == interrupter_) return true;
        if (FD_ISSET(socket->fd(), &read_fds_)) events |= socket_event_in;
        if (FD_ISSET(socket->fd(), &write_fds_)) events |= socket_event_out;
        if (FD_ISSET(socket->fd(), &except_fds_)) events |= socket_event_has_error;
        return socket->update(events);
    }
    inline void check_fd_sets__()
    {
        for (auto it = socket_events_.begin(); it != socket_events_.end(); ) {
            if (check_fd_set__(it->second)) ++it;
            else it = socket_events_.erase(it);
        }
    }
private:
    socket_initializer initializer_;
    socket_interrupter interrupter_;
    socket_event_pool_type& socket_events_;
    fd_set read_fds_, write_fds_, except_fds_;
    fd_type max_fd_ = 0;
};

}}

#endif
