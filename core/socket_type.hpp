#ifndef __SOCKET_TYPE_HPP__
#define __SOCKET_TYPE_HPP__

#include "socket.hpp"
#include "socket_addr.hpp"
#include "socket_io_type.hpp"

namespace mininet { namespace core {

enum socket_protocol {
    tcp = socket_io_type::tcp,
    udp = socket_io_type::udp,
};

enum socket_identifier {
    server = 0x8000,
};

class socket_base 
    : public std::enable_shared_from_this<socket_base> {
public:
    static const int state_reading_type = 1;
    static const int state_writing_type = 2;
    static const int state_error_type   = 3;
public:
    typedef socket_base type;
    typedef std::shared_ptr<type> ptr;
    typedef std::function<void(ptr, int)> state_callback_type;
public:
    socket_base() = default;
    virtual ~socket_base() { socket_io_.close(); }
    static ptr make() 
    { 
        return std::make_shared<type>();
    }
    template <typename T>
    static ptr cast(std::shared_ptr<T> t)
    {
        return std::dynamic_pointer_cast<type>(t);
    }
public:
    fd_type& fd() { return socket_io_.fd(); }
    const fd_type& fd() const { return socket_io_.fd(); }
    socket_io_type& socket_io() { return socket_io_; }
    const socket_io_type& socket_io() const { return socket_io_; }
protected:
    friend class socket_event_type;
    virtual int  event_in_callback__() { return 0; }
    virtual int  event_out_callback__() { return 0; }
    virtual void event_error_callback__() { }
protected:
    void set_state_callback__(state_callback_type cb)
    { 
        state_callback_ = std::move(cb);
    }
    void state_callback__(int type) 
    { 
        if (state_callback_) state_callback_(shared_from_this(), type);
    }
protected:
    socket_io_type socket_io_;
    state_callback_type state_callback_;
};

// Buffer must have begin(), end(), cbegin(), cend(), size(), resize()
// For STL: std::array, std::vector<char>, std::string are perfect
// Or: your custom buffer type
template <typename Reactor,
    typename Buffer, 
    int Protocol = tcp>
class socket_type 
    : public socket_base {
public:
    typedef Reactor reactor_type;
    typedef Buffer buffer_type;
    typedef socket_type<reactor_type, buffer_type, Protocol> type;
    typedef std::shared_ptr<type> ptr;
    typedef std::shared_ptr<buffer_type> buffer_ptr;
    typedef std::function<void(ptr, buffer_ptr, const std::string&)> read_callback_type;
    typedef std::function<void(ptr, buffer_ptr, const std::string&)> write_callback_type;
    typedef std::tuple<buffer_ptr, read_callback_type> async_read_type;
    typedef std::tuple<buffer_ptr, write_callback_type> async_write_type;
public:
    explicit socket_type(reactor_type& reactor) : reactor_(reactor) { }
    static ptr make(reactor_type& reactor, const std::string& host = std::string())
    {
        auto connector = std::make_shared<type>(reactor);
        if (!host.empty() && !connector->initialize__(host)) return nullptr;
        if (!host.empty()) reactor.add(connector);
        return connector;
    }
    static socket_base::ptr parent(ptr p) 
    { 
        return socket_base::cast(p); 
    }
    template <typename T>
    static ptr cast(std::shared_ptr<T> t) 
    { 
        return std::dynamic_pointer_cast<type>(t);
    }
    ptr shared_from_this() 
    { 
        return std::dynamic_pointer_cast<type>(socket_base::shared_from_this()); 
    }
public:
    void async_read(buffer_ptr buffer, read_callback_type cb)
    {
        state_callback__(state_reading_type);
        async_read_list_.push_back(std::make_tuple(buffer, std::move(cb)));
    }
    void async_write(buffer_ptr buffer, write_callback_type cb)
    {
        state_callback__(state_writing_type);
        async_write_list_.push_back(std::make_tuple(buffer, std::move(cb)));
    }
    template <typename T>
    void async_write(const T& buffer, write_callback_type cb)
    {
        auto data = std::make_shared<buffer_type>(buffer.begin(), buffer.end());
        async_write(data, std::move(cb));
    }
    void async_write(const char* buffer, write_callback_type cb)
    {
        auto data = std::make_shared<buffer_type>(buffer, buffer + strlen(buffer));
        async_write(data, std::move(cb));
    }
protected:
    int event_in_callback__()
    {
        if (async_read_list_.empty()) return 0;
        auto& tuple = async_read_list_.front();
        auto& buffer = std::get<0>(tuple);
        auto& callback = std::get<1>(tuple);
        auto ret = socket_io_.recv(&*buffer->begin(), buffer->size());
        if (ret > 0) buffer->resize(ret);
        callback(shared_from_this(), buffer, ret >= 0 ? "" : "recv error");
        async_read_list_.pop_front();
        return ret;
    }
    int event_out_callback__()
    {
        if (async_write_list_.empty()) return 0;
        auto& tuple = async_write_list_.front();
        auto& buffer = std::get<0>(tuple);
        auto& callback = std::get<1>(tuple);
        auto ret = socket_io_.send(&*buffer->cbegin(), buffer->size());
        callback(shared_from_this(), buffer, ret >= 0 ? "" : "send error");
        async_write_list_.pop_front();
        return ret;
    }
    void event_error_callback__()
    {
        reactor_.remove(shared_from_this());
    }
protected:
    bool initialize__(const std::string& host)
    {
        if (!socket_io_.set_protocol(core::tcp)) return false;
        if (!socket_io_.connect(host)) return false;
        return true;
    }
protected:
    reactor_type& reactor_;
    std::list<async_read_type> async_read_list_;
    std::list<async_write_type> async_write_list_;
};

template <typename Reactor,
    typename Buffer>
class socket_type<Reactor, Buffer, tcp | server>
    : public socket_base {
public:
    typedef Reactor reactor_type;
    typedef Buffer buffer_type;
    typedef socket_type<reactor_type, buffer_type, tcp | server> type;
    typedef std::shared_ptr<type> ptr;
    typedef typename socket_type<reactor_type, buffer_type, tcp>::ptr socket_ptr;
    typedef std::function<void(socket_ptr, const std::string&)> accepted_callback_type;
public:
    explicit socket_type(reactor_type& reactor) : reactor_(reactor) { }
    static ptr make(reactor_type& reactor, const std::string& host, accepted_callback_type cb)
    {
        auto acceptor = std::make_shared<type>(reactor);
        if (!acceptor->initialize__(host, std::move(cb))) return nullptr;
        reactor.add(acceptor);
        return acceptor;
    }
    static socket_base::ptr parent(ptr p)
    {
        return socket_base::cast(p);
    }
    ptr shared_from_this()
    {
        return std::dynamic_pointer_cast<type>(socket_base::shared_from_this());
    }
protected:
    bool initialize__(const std::string& host, accepted_callback_type cb)
    {
        callback_ = std::move(cb);
        if (!socket_io_.set_protocol(tcp)) return false; 
        if (!socket_io_.bind(host)) return false; 
        if (!socket_io_.listen()) return false; 
        return true;
    }
protected:
    int event_in_callback__()
    {
        auto socket = socket_type<reactor_type, buffer_type, tcp>::make(reactor_);
        bool ret = socket_io_.accepted(socket->socket_io());
        if (callback_) callback_(socket, ret ? "" : "accepted error");
        reactor_.add(socket);
        return ret ? 1 : 0;
    }
    void event_error_callback__()
    {
        reactor_.remove(shared_from_this());
    }
protected:
    reactor_type& reactor_;
    accepted_callback_type callback_;
};

// Buffer must have begin(), end(), cbegin(), cend(), size(), resize()
// For STL: std::array, std::vector<char>, std::string are perfect
// Or: your custom buffer type
template <typename Reactor,
    typename Buffer>
class socket_type<Reactor, Buffer, udp> 
    : public socket_base {
public:
    typedef Reactor reactor_type;
    typedef Buffer buffer_type; 
    typedef socket_type<reactor_type, buffer_type, udp> type;
    typedef std::shared_ptr<type> ptr;
    typedef std::shared_ptr<buffer_type> buffer_ptr;
    typedef std::shared_ptr<socket_addr_type> socket_addr_ptr;
    typedef std::function<void(ptr, socket_addr_ptr, buffer_ptr, const std::string&)> read_callback_type;
    typedef std::function<void(ptr, socket_addr_ptr, buffer_ptr, const std::string&)> write_callback_type;
    typedef std::tuple<buffer_ptr, socket_addr_ptr, read_callback_type> async_read_type;
    typedef std::tuple<buffer_ptr, socket_addr_ptr, write_callback_type> async_write_type;
public:
    explicit socket_type(reactor_type& reactor) : reactor_(reactor) { }
    static ptr make(reactor_type& reactor, const std::string& host = std::string())
    {
        auto socket = std::make_shared<type>(reactor);
        if (!socket->initialize__(host)) return nullptr;
        reactor.add(socket);
        return socket;
    }
    static socket_base::ptr parent(ptr p) 
    { 
        return socket_base::cast(p); 
    }
    template <typename T>
    static ptr cast(std::shared_ptr<T> t) 
    { 
        return std::dynamic_pointer_cast<type>(t); 
    }
    ptr shared_from_this() 
    { 
        return std::dynamic_pointer_cast<type>(socket_base::shared_from_this()); 
    }
public:
    void async_read(buffer_ptr buffer, socket_addr_ptr addr, read_callback_type cb)
    {
        state_callback__(state_reading_type);
        async_read_list_.push_back(std::make_tuple(buffer, addr, std::move(cb)));
    }
    void async_write(buffer_ptr buffer, socket_addr_ptr& addr, write_callback_type cb)
    {
        state_callback__(state_writing_type);
        async_write_list_.push_back(std::make_tuple(buffer, addr, std::move(cb)));
    }
    template <typename T>
    void async_write(const T& buffer, socket_addr_ptr& addr, write_callback_type cb)
    {
        auto data = std::make_shared<buffer_type>(buffer.begin(), buffer.end());
        async_write(data, addr, std::move(cb));
    }
    void async_write(const char* buffer, socket_addr_ptr& addr, write_callback_type cb)
    {
        auto data = std::make_shared<buffer_type>(buffer, buffer + strlen(buffer));
        async_write(data, addr, std::move(cb));
    }
protected:
    int event_in_callback__()
    {
        if (async_read_list_.empty()) return 0;
        auto& tuple = async_read_list_.front();
        auto& buffer = std::get<0>(tuple);
        auto& addr = std::get<1>(tuple);
        auto& callback = std::get<2>(tuple);
        auto ret = socket_io_.recvfrom(&*buffer->begin(), buffer->size(), *addr);
        if (ret > 0) buffer->resize(ret);
        callback(shared_from_this(), addr, buffer, ret >= 0 ? "" : "recvfrom error");
        async_read_list_.pop_front();
        return ret;
    }
    int event_out_callback__()
    {
        if (async_write_list_.empty()) return 0;
        auto& tuple = async_write_list_.front();
        auto& buffer = std::get<0>(tuple);
        auto& addr = std::get<1>(tuple);
        auto& callback = std::get<2>(tuple);
        auto ret = socket_io_.sendto(&*buffer->cbegin(), buffer->size(), *addr);
        callback(shared_from_this(), addr, buffer, ret >= 0 ? "" : "sendto error");
        async_write_list_.pop_front();
        return ret;
    }
    void event_error_callback__()
    {
        reactor_.remove(shared_from_this());
    }
protected:
    bool initialize__(const std::string& host)
    {
        if (!socket_io_.set_protocol(core::udp)) return false; 
        if (!host.empty() && !socket_io_.bind(host)) return false; 
        return true;
    }
protected:
    reactor_type& reactor_;
    std::list<async_read_type> async_read_list_;
    std::list<async_write_type> async_write_list_;
};

}}

#endif 
