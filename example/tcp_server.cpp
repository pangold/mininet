#include <iostream>
#include "../core/socket_type.hpp"
#include "../core/reactor.hpp"

typedef std::vector<char> buffer_type;
typedef std::shared_ptr<buffer_type> buffer_ptr;
typedef mininet::core::reactor_type reactor_type;
typedef mininet::core::socket_type<reactor_type, buffer_type, mininet::core::tcp> socket_type;
typedef mininet::core::socket_type<reactor_type, buffer_type, mininet::core::tcp | mininet::core::server> acceptor_type;

static void written_proc__(socket_type::ptr socket, buffer_ptr buffer, const std::string& error) 
{
    if (!error.empty()) {
        std::cout << error << std::endl;
        return;
    }
};

static void received_proc__(socket_type::ptr socket, buffer_ptr buffer, const std::string& error) 
{
    static size_t count = 0;
    if (!error.empty()) {
        std::cout << error << std::endl;
        return;
    }
    if (++count % 10000 == 0)
    std::cout << "received: " << std::string(buffer->begin(), buffer->end()) << std::endl;
    socket->async_write(*buffer, written_proc__);
    buffer->assign(1024, 0);
    socket->async_read(buffer, received_proc__);
}

static void accepted_proc__(socket_type::ptr conn, const std::string& error) 
{
    auto received_buffer = std::make_shared<buffer_type>(1024, 0);
    conn->async_read(received_buffer, received_proc__);
};

int main()
{
    reactor_type reactor;

    acceptor_type::ptr listener = acceptor_type::make(reactor, "127.0.0.1:8888", accepted_proc__);

    //reactor.add_timer_at(reactor_type::time_type::now() + 1000, []() {
    //    std::cout << "timeout at 1000 ms" << std::endl;
    //});
    //reactor.add_timer_after(2000, []() {
    //    std::cout << "timeout after 2000 ms" << std::endl;
    //});
    //reactor.add_timer_every(500, []() {
    //    std::cout << "timeout every 500 ms" << std::endl;
    //});

    reactor.run();

    return 0;
}
