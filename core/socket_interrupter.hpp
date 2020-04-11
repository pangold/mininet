#ifndef __SOCKET_INTERRUPTER_HPP__
#define __SOCKET_INTERRUPTER_HPP__

#include "socket_type.hpp"

namespace mininet { namespace core {

class socket_interrupter {
public:
    socket_interrupter() = default;
public:
    void interrupt()
    {
        // socket_send()
    }
public:
    operator fd_type () { return receiver_fd_; }
private:
    fd_type sender_fd_;
    fd_type receiver_fd_;
};

}}

#endif