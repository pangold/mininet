#include <iostream>
#include "../core/socket_type.hpp"
#include "../core/reactor.hpp"

typedef std::vector<char> buffer_type;
typedef std::shared_ptr<buffer_type> buffer_ptr;
typedef mininet::core::socket_addr_type socket_addr_type;
typedef std::shared_ptr<socket_addr_type> socket_addr_ptr;
typedef mininet::core::reactor_type reactor_type;
typedef mininet::core::socket_type<reactor_type, buffer_type, mininet::core::udp> socket_type;

static void written_proc__(socket_type::ptr socket, socket_addr_ptr addr, buffer_ptr buffer, const std::string& error)
{
    if (!error.empty()) {
        std::cout << error << std::endl;
        return;
    }
};

static void received_proc__(socket_type::ptr socket, socket_addr_ptr addr, buffer_ptr buffer, const std::string& error)
{
    static size_t count = 0;
    if (!error.empty()) {
        std::cout << error << std::endl;
        return;
    }
    if (++count % 10000 == 0)
    std::cout << "from: " << addr->to_string() << ", received: " <<
        std::string(buffer->begin(), buffer->end()) <<
        std::endl;
    socket->async_write(*buffer, addr, written_proc__);
    buffer->assign(1024, 0);
    socket->async_read(buffer, addr, received_proc__);
}

int main()
{
    reactor_type reactor;
    socket_type::ptr socket = socket_type::make(reactor, "127.0.0.1:10086");
    if (!socket) { std::cout << "bind error" << std::endl; return 1; }
    reactor.add(socket);
    auto received_addr = std::make_shared<socket_addr_type>();
    auto received_buffer = std::make_shared<buffer_type>(1024, 0);
    socket->async_read(received_buffer, received_addr, received_proc__);
    reactor.run();

    return 0;
}
