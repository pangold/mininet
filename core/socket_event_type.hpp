#ifndef __SOCKET_EVENT_TYPE_HPP__
#define __SOCKET_EVENT_TYPE_HPP__

#include "socket.hpp"
#include "socket_addr.hpp"
#include "socket_io_type.hpp"
#include "socket_type.hpp"

namespace mininet { namespace core {

enum socket_event {
    socket_event_none      = 0x00000000,
    socket_event_in        = 0x00000001,
    socket_event_out       = 0x00000002,
    socket_event_has_error = 0x00000004,
};

class socket_event_type;
typedef std::shared_ptr<socket_event_type> socket_event_ptr;
typedef std::map<socket_base*, socket_event_ptr> socket_event_pool_type;

class socket_event_type
    : public std::enable_shared_from_this<socket_event_type> {
public:
    typedef socket_event_type type;
    typedef std::shared_ptr<type> ptr;
    typedef std::function<void(ptr, int)> state_callback_type;
public:
    explicit socket_event_type(socket_base::ptr socket) : socket_(socket) { }
    static ptr make(socket_base::ptr socket) { return std::make_shared<type>(socket); }
public:
    void set_readable() noexcept { events_ |= socket_event_in; }
    void set_writable() noexcept { events_ |= socket_event_out; }
    void reset_readable() noexcept { events_ &= ~socket_event_in; }
    void reset_writable() noexcept { events_ &= ~socket_event_out; }
    bool readable() const { return (events_ & socket_event_in) != 0; }
    bool writable() const { return (events_ & socket_event_out) != 0; }
public:
    fd_type& fd() { return socket_->fd(); }
    const fd_type& fd() const { return socket_->fd(); }
    socket_base::ptr& socket() { return socket_; }
    const socket_base::ptr& socket() const { return socket_; }
    uint32_t events() const { return events_; }
public:
    void set_state_callback(state_callback_type cb)
    {
        using namespace std::placeholders;
        auto state_cb = std::bind(&socket_event_type::state_changed_proc__, this, _1, _2);
        socket_->set_state_callback__(state_cb);
        state_callback_ = std::move(cb);
    }
    bool update(uint32_t events)
    {
        if (events & socket_event_in) return update_event_in__();
        if (events & socket_event_out) return update_event_out__();
        if (events & socket_event_has_error) return update_event_error__();
        return true; // idle socket
    }
private:
    bool update_event_in__()
    {
        // reset_readable();
        if (socket_->event_in_callback__() >= 0) {
            return true;
        }
        return update_event_error__();
    }
    bool update_event_out__()
    {
        reset_writable();
        if (socket_->event_out_callback__() >= 0) {
            return true;
        }
        return update_event_error__();
    }
    bool update_event_error__()
    {
        socket_->event_error_callback__();
        return false;
    }
    void state_changed_proc__(socket_base::ptr socket, int type)
    {
        if (!state_callback_) return;
        state_callback_(shared_from_this(), type);
    }
private:
    socket_base::ptr socket_;
    uint32_t events_;
    state_callback_type state_callback_;
};

}}

#endif 
