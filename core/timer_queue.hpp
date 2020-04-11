#ifndef __NET_CORE_TIMER_QUEUE_HPP__
#define __NET_CORE_TIMER_QUEUE_HPP__

#include <cstdint>
#include <memory>

#include "heap.hpp"
#include "timer.hpp"
#include "time_impl.hpp"

namespace mininet { namespace core {

class timer_queue {
public:
    typedef timer<time_impl_type> timer_type;
    typedef std::shared_ptr<timer_type> timer_ptr;
    typedef timer_type::timeout_callback_type timeout_callback_type;
public:
    timer_queue() = default;
public:
    void add_timer_at(int64_t ms, timeout_callback_type cb)
    {
        timer_ptr timer = std::make_shared<timer_type>();
        timer->set_expired_at(ms);
        timer->set_timeout_callback(cb);
        timers_.push(std::move(timer));
    }
    void add_timer_after(int64_t ms, timeout_callback_type cb)
    {
        timer_ptr timer = std::make_shared<timer_type>();
        timer->set_expired_after(ms);
        timer->set_timeout_callback(cb);
        timers_.push(std::move(timer));
    }
    void add_timer_every(int64_t ms, timeout_callback_type cb)
    {
        timer_ptr timer = std::make_shared<timer_type>();
        timer->set_expired_every(ms);
        timer->set_timeout_callback(cb);
        timers_.push(std::move(timer));
    }
    void check_expired_timers()
    {
        while (!timers_.empty() && timers_.top()->check_expired()) {
            auto&& timer = timers_.pop();
            if (timer->remaining() > 0) {
                timers_.push(std::move(timer));
            }
        }
    }
    inline int64_t get_latest_timer()
    {
        if (timers_.empty()) return 0;
        timer_ptr latest = timers_.top();
        int64_t remaining = latest->remaining();
        return remaining > 0 ? remaining : 0;
    }
private:
    min_heap_ptr<timer_ptr> timers_;
};

}}

#endif