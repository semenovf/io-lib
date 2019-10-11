////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.09 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "tcp_socket.hpp"
#include <unistd.h>

namespace pfs {
namespace io {

class tcp_server;

class tcp_peer : public tcp_socket
{
    friend class tcp_server;

protected:
    tcp_peer (int fd) : tcp_socket(fd) {}

public:
    tcp_peer (tcp_peer const & rhs) = delete;
    tcp_peer & operator = (tcp_peer const & rhs) = delete;

    tcp_peer (tcp_peer && rhs)
        : tcp_socket(std::forward<tcp_socket>(rhs))
    {}

    tcp_peer & operator = (tcp_peer && rhs)
    {
        tcp_socket::operator = (std::forward<tcp_socket>(rhs));
        return *this;
    }

    device_type type () const noexcept override
    {
        return device_type::tcp_peer;
    }
};

class tcp_server
{
    int _fd = -1;

protected:
    error_code open (std::string const & servername
            , uint16_t port
            , bool nonblocking
            , int max_pending_connections = 30)
    {
        inet_socket_initializer socket_initializer(& _fd
                , SOCK_STREAM
                , servername
                , port
                , nonblocking
                , max_pending_connections);
        auto ec = socket_initializer.open();
        return ec;
    }

    void swap (tcp_server & rhs)
    {
        using std::swap;
        swap(_fd, rhs._fd);
    }

public:
    tcp_server () {}

    tcp_server (tcp_server const &) = delete;
    tcp_server & operator = (tcp_server const &) = delete;

    tcp_server (tcp_server && rhs)
    {
       swap(rhs);
    }

    tcp_server & operator = (tcp_server && rhs)
    {
        tcp_server tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    ~tcp_server () {}

    error_code close ()
    {
        error_code ec;

        if (_fd > 0) {
            if (::close(_fd) < 0)
                ec = get_last_system_error();
        }

        _fd = -1;
        return ec;
    }

    device accept (error_code & ec)
    {
        sockaddr_in peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);

        int peer_fd = ::accept(_fd
                , reinterpret_cast<sockaddr *> (& peer_addr)
                , & peer_addr_len);

        if (peer_fd < 0) {
            ec = get_last_system_error();
            return device{};
        }

        return device{new tcp_peer(peer_fd)};
    }

    friend tcp_server make_tcp_server (std::string const & servername
            , uint16_t port
            , bool nonblocking
            , int max_pending_connections
            , error_code & ec);
};

inline tcp_server make_tcp_server (std::string const & servername
        , uint16_t port
        , bool nonblocking
        , int max_pending_connections
        , error_code & ec)
{
    tcp_server server;
    ec = server.open(servername, port, nonblocking, max_pending_connections);
    return server;
}

inline tcp_server make_tcp_server (std::string const & servername
        , uint16_t port
        , bool nonblocking
        , error_code & ec)
{
    return make_tcp_server(servername, port, nonblocking, 30, ec);
}

inline tcp_server make_tcp_server (std::string const & servername
        , uint16_t port
        , bool nonblocking
        , int max_pending_connections)
{
    error_code ec;
    auto s = make_tcp_server(servername
            , port
            , nonblocking
            , max_pending_connections
            , ec);
    if (ec) throw exception(ec);
    return s;
}

inline tcp_server make_tcp_server (std::string const & servername
        , uint16_t port
        , bool nonblocking)
{
    return make_tcp_server(servername, port, nonblocking, 30);
}

}} // pfs::io




