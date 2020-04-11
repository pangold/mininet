#ifndef __SOCKET_IO_TYPE_HPP__
#define __SOCKET_IO_TYPE_HPP__

#include "socket.hpp"
#include "socket_addr.hpp"

namespace mininet { namespace core {

#if defined (WINDOWS)
    typedef SOCKET fd_type;
    typedef int socklen_t;
#else
    typedef int fd_type;
#endif

class socket_io_type {
public:
    static const int tcp = 1;
    static const int udp = 2;
public:
    socket_io_type() = default;
public:
    bool set_block(bool block);
    bool set_protocol(int protocol);
    bool bind(const std::string& host);
    bool listen();
    bool accepted(socket_io_type& socket);
    bool connect(const std::string& host);
    void close();
public:
    int recv(char* data, size_t size);
    int recvfrom(char* data, size_t size, socket_addr_type& addr);
    int send(const char* data, size_t size);
    int sendto(const char* data, size_t size, const socket_addr_type& addr);
public:
    fd_type& fd() { return fd_; }
    const fd_type& fd() const { return fd_; }
    operator fd_type& () { return fd_; }
    operator const fd_type& () const { return fd_; }
private:
    fd_type fd_;
    socket_addr_type addr_;
    int protocol_;
};

bool socket_io_type::set_block(bool block)
{
#if defined (WINDOWS)
    unsigned long argp = block ? 0 : 1;
    if (ioctlsocket(fd_, FIONBIO, &argp) == SOCKET_ERROR) {
        return false;
    }
#else
    int flags = fcntl(fd_, F_GETFL, 0);
    flags = block ? flags & ~O_NONBLOCK : flags | O_NONBLOCK;
    if (fcntl(fd_, F_SETFL, (long)flags) == -1) {
        return false;
    }
#endif
    return true;
}

bool socket_io_type::set_protocol(int protocol)
{
    protocol_ = protocol;
    if (protocol_ == tcp) {
        fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else if (protocol_ == udp) {
        fd_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    }
    return fd_ != 0;
}

bool socket_io_type::bind(const std::string& host)
{
    addr_ = socket_addr_type(host);
    if (::bind(fd_, &addr_.to_sockaddr(), sizeof(addr_.to_sockaddr())) != 0) {
        return false;
    }
    return true;
}

bool socket_io_type::listen()
{
    if (::listen(fd_, 64) != 0) {
        return false;
    }
    return true;
}

bool socket_io_type::accepted(socket_io_type& socket)
{
    socklen_t len = sizeof(socket.addr_.to_sockaddr());
    socket.protocol_ = tcp;
    socket.fd_ = ::accept(fd_, &socket.addr_.to_sockaddr(), &len);
    if (socket.fd_ == 0) {
        return false;
    }
    return true;
}

bool socket_io_type::connect(const std::string& host)
{
    addr_ = socket_addr_type(host);
    if (::connect(fd_, &addr_.to_sockaddr(), sizeof(addr_.to_sockaddr())) != 0) {
        return false;
    }
    return true;
}

void socket_io_type::close()
{
#if defined(WINDOWS)
    ::closesocket(fd_);
#else
    ::close(fd_);
#endif
}

int socket_io_type::recv(char* data, size_t size)
{
    auto ret = ::recv(fd_, (char*)data, size, 0);
#if !defined (WINDOWS)
    if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
        return 0;
#endif
    return ret;
}

int socket_io_type::recvfrom(char* data, size_t size, socket_addr_type& addr)
{
    socklen_t len = sizeof(addr.to_sockaddr());
    auto ret = ::recvfrom(fd_, (char*)data, size, 0, &addr.to_sockaddr(), &len);
#if !defined (WINDOWS)
    if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
        return 0;
#endif
    return ret;
}

int socket_io_type::send(const char* data, size_t size)
{
    auto ret = ::send(fd_, (const char*)data, size, 0);
    return ret;
}

int socket_io_type::sendto(const char* data, size_t size, const socket_addr_type& addr)
{
    auto ret = ::sendto(fd_, (const char*)data, size, 0, &addr.to_sockaddr(), sizeof(addr.to_sockaddr()));
    return ret;
}

}}

#endif
