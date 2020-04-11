#ifndef __NET_CORE_TIMER_HPP__
#define __NET_CORE_TIMER_HPP__

#include <functional>
#include <cstdint>
#include <algorithm>
#include <utility>

namespace mininet { namespace core {

template <typename Impl>
class timer {
public:
    typedef Impl impl_type;
    typedef std::function<void()> timeout_callback_type;
public:
    timer() = default;
    timer(int64_t expired_at, timeout_callback_type cb, int64_t interval = 0)
        : expired_at_(expired_at)
        , callback_(cb)
        , interval_(interval)
    { }
    timer(const timer& rhs) 
        : expired_at_(rhs.expired_at_)
        , callback_(rhs.callback_)
        , interval_(rhs.interval_)
    { }
    timer(timer&& rhs) noexcept 
        : expired_at_(std::exchange(rhs.expired_at_, 0))
        , callback_(std::move(rhs.callback_))
        , interval_(std::exchange(rhs.interval_, 0))
    { }
public:
    void set_timeout_callback(timeout_callback_type cb) noexcept
    {
        callback_ = cb;
    }
    void set_expired_at(int64_t expired_at) noexcept
    {
        expired_at_ = expired_at;
        interval_ = 0;
    }
    void set_expired_after(int64_t expired_after) noexcept
    {
        expired_at_ = impl_type::now() + expired_after;
        interval_ = 0;
    }
    void set_expired_every(int64_t interval) noexcept
    {
        expired_at_ = impl_type::now() + interval;
        interval_ = interval;
    }
    inline bool check_expired()
    {
        auto remaining = expired_at_ - impl_type::now();
        if (remaining > 0) return false;  // not expired yet
        if (callback_) callback_();
        expired_at_ += interval_;
        return true;
    }
    inline int64_t remaining() const 
    { 
        auto remain = expired_at_ - impl_type::now(); 
        while (interval_ > 0 && remain <= 0) remain += interval_;
        return remain;
    }
    inline int64_t expired_at() const 
    { 
        return expired_at_;
    }
    inline int64_t interval() const 
    { 
        return interval_;
    }
    inline bool operator < (const timer& rhs) const 
    { 
        return expired_at_ < rhs.expired_at_; 
    }
    inline bool operator > (const timer& rhs) const 
    { 
        return expired_at_ > rhs.expired_at_; 
    }
    inline timer& operator = (timer& rhs)
    {
        expired_at_ = rhs.expired_at_;
        interval_ = rhs.interval_;
        callback_ = rhs.callback_;
        return *this;
    }
    inline timer& operator = (timer&& rhs)
    {
        assert(this != &rhs);
        std::exchange(expired_at_, rhs.expired_at_);
        std::exchange(interval_, rhs.interval_);
        std::swap(callback_, rhs.callback_);
        return *this;
    }
    inline operator impl_type () 
    { 
        return expired_at_;
    }
private:
    int64_t expired_at_;
    timeout_callback_type callback_;
    int64_t interval_; // milliseconds
}; 

}}

#endif
