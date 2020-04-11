#ifndef __NET_CORE_TIME_IMPL_HPP__
#define __NET_CORE_TIME_IMPL_HPP__

#include <ctime>
#include <chrono>

namespace mininet { namespace core {

class time_impl_type {
    typedef std::chrono::steady_clock clock_type;
    typedef std::chrono::milliseconds clock_unit_type;
public:
    typedef clock_type::time_point type;
public:
    time_impl_type() = default;
    template <typename T>
    time_impl_type(T ms) : time_(clock_unit_type(ms)) { }
public:
    inline bool operator > (const time_impl_type& rhs) const
    {
        return (*this - rhs) > 0;
    }
    inline bool operator < (const time_impl_type& rhs) const
    {
        return (*this - rhs) < 0;
    }
    inline bool operator == (const time_impl_type& rhs) const
    {
        return (*this - rhs) == 0;
    }
    inline bool operator != (const time_impl_type& rhs) const
    {
        return (*this - rhs) != 0;
    }
    inline int64_t operator - (const time_impl_type& rhs) const
    {
        return std::chrono::duration_cast<clock_unit_type>(time_ - rhs.time_).count();
    }
    inline int64_t operator -= (const time_impl_type& rhs)
    {
        return *this - rhs;
    }
    template <typename T>
    inline time_impl_type& operator + (T rhs)
    {
        time_ += clock_unit_type(rhs);
        return *this;
    }
    template <typename T>
    inline time_impl_type& operator += (T rhs)
    {
        time_ += clock_unit_type(rhs);
        return *this;
    }
    inline operator int64_t ()
    {
        return std::chrono::duration_cast<clock_unit_type>(time_.time_since_epoch()).count();
    }
public:
    inline static time_impl_type make()
    {
        time_impl_type t;
        t.time_ = clock_type::now();
        return t;
    }
    inline static int64_t now()
    {
        type now = clock_type::now();
        return std::chrono::duration_cast<clock_unit_type>(now.time_since_epoch()).count();
    }
private:
    type time_;
};

}}

#endif