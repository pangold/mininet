#ifndef __NET_CORE_REACTOR_HPP__
#define __NET_CORE_REACTOR_HPP__

#include "timer_queue.hpp"
#include "socket_event_type.hpp"

#if defined (LINUX)
#include "reactor_epoll.hpp"
#elif defined (APPLE)
#include "reactor_kqueue.hpp"
#else
#include "reactor_select.hpp"
#endif

namespace mininet { namespace core {

#if defined (LINUX)
typedef epoll_type  impl_type;
#elif defined (APPLE)
typedef kqueue_type impl_type;
#else
typedef select_type impl_type;
#endif

class reactor_type {
public:
    typedef timer_queue timer_queue_type;
    typedef timer_queue_type::timer_type::impl_type time_type;
    typedef timer_queue_type::timeout_callback_type timeout_callback_type;
public:
    reactor_type() : impl_(socket_events_) { }
public:
    void add_timer_at(int64_t ms, timeout_callback_type cb)
    {
        timer_queue_.add_timer_at(ms, cb);
        impl_.update_timer(ms, false);
    }
    void add_timer_after(int64_t ms, timeout_callback_type cb)
    {
        timer_queue_.add_timer_after(ms, cb);
        impl_.update_timer(time_type::now() + ms, false);
    }
    void add_timer_every(int64_t ms, timeout_callback_type cb)
    {
        timer_queue_.add_timer_every(ms, cb);
        impl_.update_timer(time_type::now() + ms, true);
    }
    void add(socket_base::ptr socket)
    {
        using namespace std::placeholders;
        auto cb = std::bind(&reactor_type::socket_state_proc__, this, _1, _2);
        auto event = socket_event_type::make(socket);
        event->set_state_callback(cb);
        event->set_readable();
        socket_events_[socket.get()] = event;
        impl_.insert(event.get());
    }
    void remove(socket_base::ptr socket)
    {
        auto it = socket_events_.find(socket.get());
        if (it == socket_events_.end()) return;
        if (impl_.cancel(it->second.get()))
        socket_events_.erase(it);
    }
    void run()
    {
        while (1) { run_one__(); }
    }
private:
    inline void run_one__()
    {
        thread_id_ = std::this_thread::get_id();
        auto timeout = timer_queue_.get_latest_timer();
        impl_.run(timeout <= 0 ? 5000 : timeout);
        timer_queue_.check_expired_timers();
    }
    void socket_state_proc__(socket_event_ptr event, int type)
    {
        switch (type) {
        case socket_base::state_reading_type:
            // event->set_readable();
            // impl_.update(event.get());
            break;
        case socket_base::state_writing_type:
            event->set_writable();
            impl_.update(event.get());
            break;
        }
    }
private:
    socket_event_pool_type socket_events_;
    timer_queue_type timer_queue_;
    impl_type impl_;
    std::thread::id thread_id_;
};

}}

#endif 
