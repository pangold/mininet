#include <iostream>
#include "../core/socket_type.hpp"
#include "../core/reactor.hpp"

typedef std::vector<char> buffer_type;
typedef std::shared_ptr<buffer_type> buffer_ptr;
typedef mininet::core::reactor_type reactor_type;
typedef mininet::core::socket_type<reactor_type, buffer_type, mininet::core::tcp> connector_type;

static void written_proc__(connector_type::ptr socket, buffer_ptr buffer, const std::string& error)
{
    static size_t count = 0;
    if (!error.empty()) {
        std::cout << "write error" << std::endl;
        return;
    }
    socket->async_write(std::to_string(++count), written_proc__);
}

static void received_proc__(connector_type::ptr socket, buffer_ptr buffer, const std::string& error)
{
    static size_t count = 0;
    if (!error.empty()) {
        std::cout << error << std::endl;
        return;
    }
    if (++count % 1000 == 0)
    std::cout << "received: " << std::string(buffer->begin(), buffer->end()) << std::endl;
    buffer->assign(1024, 0);
    socket->async_read(buffer, received_proc__);
}

int main()
{
    reactor_type reactor;
    connector_type::ptr connector = connector_type::make(reactor, "127.0.0.1:8888");
    if (!connector) { std::cout << "failed to connect" << std::endl; return 1; }
    auto received_buffer = std::make_shared<buffer_type>(1024, 0);
    connector->async_write("hello", written_proc__);
    connector->async_read(received_buffer, received_proc__);
    reactor.run();

    return 0;
}
