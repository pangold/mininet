#ifndef __NET_CORE_REACTOR_KQUEUE_HPP__
#define __NET_CORE_REACTOR_KQUEUE_HPP__

#include "socket.hpp"
#include "socket_type.hpp"
#include "socket_event_type.hpp"

namespace mininet { namespace core {

class kqueue_type {
public:
    explicit kqueue_type(socket_event_pool_type& socket_events)
        : socket_events_(socket_events)
        , kqueue_fd_(::kqueue())
    { }
public:
    void update_timer(uint64_t ms, bool again)
    {
    }
    bool insert(socket_event_type* event)
    {
        int flags = 0;
        struct kevent changes[2];
        flags = EV_ADD | (event->readable() ? EV_ENABLE : EV_DISABLE);
        EV_SET(&changes[0], event->fd(), EVFILT_READ, flags, 0, 0, event);
        flags = EV_ADD | (event->writable() ? EV_ENABLE : EV_DISABLE);
        EV_SET(&changes[1], event->fd(), EVFILT_WRITE, flags, 0, 0, event);
        return kevent(kqueue_fd_, changes, 2, NULL, 0, NULL) != -1;
    }
    bool update(socket_event_type* event)
    {
        int flags = 0;
        struct kevent changes[2];
        flags = EV_ADD | (event->readable() ? EV_ENABLE : EV_DISABLE);
        EV_SET(&changes[0], event->fd(), EVFILT_READ, flags, 0, 0, event);
        flags = EV_ADD | (event->writable() ? EV_ENABLE : EV_DISABLE);
        EV_SET(&changes[1], event->fd(), EVFILT_WRITE, flags, 0, 0, event);
        return kevent(kqueue_fd_, changes, 2, NULL, 0, NULL) != -1;
    }
    bool cancel(socket_event_type* event)
    {
        struct kevent changes[2];
        EV_SET(&changes[0], event->fd(), EVFILT_READ, EV_DELETE, 0, 0, 0);
        EV_SET(&changes[1], event->fd(), EVFILT_WRITE, EV_DELETE, 0, 0, 0);
        return kevent(kqueue_fd_, changes, 2, NULL, 0, NULL) != -1;
    }
    void run(int64_t timeout)
    {
        std::vector<struct kevent> events(socket_events_.size());
        int ret = kevent(kqueue_fd_, NULL, 0, events.data(), events.size(), NULL);
        for (int i = 0; i < ret; ++i) {
            check_ready_event__(events[i]);
        }
    }
private:
    inline bool check_ready_event__(socket_event_type* event, const struct kevent& ev)
    {
        // int fd = ev.ident;
        // int data = ev.data;
        uint32_t events = 0;
        if (ev.flags & EV_ERROR) events |= socket_event_has_error;
        if (ev.filter == EVFILT_READ) events |= socket_event_in;
        if (ev.filter == EVFILT_WRITE) events |= socket_event_out;
        return event->update(events);
    }
    inline bool check_ready_event__(const struct kevent& ev)
    {
        socket_event_type* event = (socket_event_type*)ev.udata;
        if (!check_ready_event__(event, ev)) {
            // cancel(event);
            // socket_events_.erase(event->socket().get());
        }
        return true;
    }
private:
    socket_event_pool_type& socket_events_;
    fd_type kqueue_fd_;
};

}}

#endif
