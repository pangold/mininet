#ifndef __SOCKET_INITIALIZER_HPP__
#define __SOCKET_INITIALIZER_HPP__

#include "socket.hpp"

namespace mininet { namespace core {

#if defined (WINDOWS)
struct socket_initializer {
    socket_initializer();
    WSADATA wsa;
};
socket_initializer::socket_initializer()
{
    WORD version = MAKEWORD(2, 2);
    if (WSAStartup(version, &wsa) != 0) {
        throw std::exception("failed to start wsa");
    }
}
#else
struct socket_initializer { };
#endif

}}

#endif