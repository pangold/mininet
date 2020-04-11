#ifndef __NET_CORE_REACTOR_EPOLL_HPP__
#define __NET_CORE_REACTOR_EPOLL_HPP__

#include "socket.hpp"
#include "socket_type.hpp"
#include "socket_event_type.hpp"

namespace mininet { namespace core {

class epoll_type {
    friend class reactor_type;
public:
    explicit epoll_type(socket_event_pool_type& socket_events)
        : epoll_fd_(epoll_create(1024)) 
        , socket_events_(socket_events)
    { }
public:
    void update_timer(uint64_t ms, bool again)
    {
    }
    bool insert(socket_event_type* event)
    {
        struct epoll_event ev = { 0 };
        ev.data.fd = event->fd();
        ev.data.ptr = event;
        ev.events = get_epoll_events__(event->events());
        return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event->fd(), &ev) != -1;
    }
    bool update(socket_event_type* event)
    {
        struct epoll_event ev = { 0 };
        ev.data.fd = event->fd();
        ev.data.ptr = event;
        ev.events = get_epoll_events__(event->events());
        return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->fd(), &ev) != -1;
    }
    bool cancel(socket_event_type* event)
    {
        struct epoll_event ev = { 0 };
        ev.data.fd = event->fd();
        ev.data.ptr = event;
        ev.events = get_epoll_events__(event->events());
        return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event->fd(), &ev) != -1;
    }
    void run(int64_t timeout)
    {
        std::vector<struct epoll_event> events(socket_events_.size());
        auto n = epoll_wait(epoll_fd_, events.data(), events.size(), timeout);
        for (int i = 0; i < n; ++i) {
            check_read_event__(events[i]);
        }
    }
private:
    inline int get_epoll_events__(uint32_t event)
    {
        int events = 0;
        if (event & socket_event_in) {
            events |= EPOLLIN | EPOLLPRI;
        }
        if (event & socket_event_out) {
            events |= EPOLLOUT;
        }
        if (events & socket_event_has_error) {
            events |= EPOLLERR;
        }
        return events;
    }
    inline bool check_ready_event__(socket_event_type* event, const struct epoll_event& ev)
    {
        uint32_t events = 0;
        if (ev.events & (EPOLLIN | EPOLLPRI)) {
            events |= socket_event_in;
        }
        if (ev.events & EPOLLOUT) {
            events |= socket_event_out;
        }
        if (ev.events & (EPOLLERR)) {
            events |= socket_event_has_error;
        }
        return event->update(events);
    }
    inline bool check_read_event__(const struct epoll_event& ev)
    {
        socket_event_type* event = (socket_event_type*)ev.data.ptr;
        if (!check_ready_event__(event, ev)) {
            // cancel(event);
            // socket_events_.erase(event->socket().get());
        }
        return true;
    }
private:
    fd_type epoll_fd_;
    socket_event_pool_type& socket_events_;
};

}}

#endif
