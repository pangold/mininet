#ifndef __SOCKET_ADDR_HPP__
#define __SOCKET_ADDR_HPP__

#include "socket.hpp"

namespace mininet { namespace core {

class socket_addr_type {
public:
    socket_addr_type() = default;
    socket_addr_type(const sockaddr_in& addr) : addr_(addr) { }
    socket_addr_type(const std::string& host) : addr_(from_string__(host)) { }
    socket_addr_type(const std::string& ip, int port) : addr_(from_string__(ip, port)) { }
public:
    std::string ip() const { return std::string(); }
    sockaddr& to_sockaddr() { return *(sockaddr*)&addr_; }
    const sockaddr& to_sockaddr() const { return *(sockaddr*)&addr_; }
    std::string to_string() const { return std::string(inet_ntoa(addr_.sin_addr)) + ":" + std::to_string(ntohs(addr_.sin_port)); }
private:
    sockaddr_in from_string__(const std::string& ip, int port);
    sockaddr_in from_string__(const std::string& host);
private:
    sockaddr_in addr_;
};

inline sockaddr_in socket_addr_type::from_string__(const std::string& ip, int port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    return addr;
}

inline sockaddr_in socket_addr_type::from_string__(const std::string& host)
{
    auto tmp(host);
    auto pos = tmp.find_last_of(':');
    auto ip = tmp.substr(0, pos);
    auto port = tmp.substr(pos + 1);
    assert(!port.empty());
    return from_string__(ip, std::atoi(port.c_str()));
}

}}

#endif
