////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.14 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "udp_socket.hpp"
#include <unistd.h>

namespace pfs {
namespace io {

// class tcp_server;

// class tcp_peer : public tcp_socket
// {
//     friend class tcp_server;
//
// protected:
//     tcp_peer (int fd) : tcp_socket(fd) {}
//
// public:
//     tcp_peer (tcp_peer const & rhs) = delete;
//     tcp_peer & operator = (tcp_peer const & rhs) = delete;
//
//     tcp_peer (tcp_peer && rhs)
//         : tcp_socket(std::forward<tcp_socket>(rhs))
//     {}
//
//     tcp_peer & operator = (tcp_peer && rhs)
//     {
//         tcp_socket::operator = (std::forward<tcp_socket>(rhs));
//         return *this;
//     }
//
//     device_type type () const noexcept override
//     {
//         return device_type::tcp_peer;
//     }
// };

class udp_server
{
    int _fd = -1;

protected:
    error_code open (std::string const & servername
            , uint16_t port
            , bool nonblocking)
    {
        inet_socket_initializer socket_initializer(& _fd
                , SOCK_DGRAM
                , servername
                , port
                , nonblocking);
        auto ec = socket_initializer.open();
        return ec;
    }

    void swap (udp_server & rhs)
    {
        using std::swap;
        swap(_fd, rhs._fd);
    }

public:
    udp_server () {}

    udp_server (udp_server const &) = delete;
    udp_server & operator = (udp_server const &) = delete;

    udp_server (udp_server && rhs)
    {
       swap(rhs);
    }

    udp_server & operator = (udp_server && rhs)
    {
        udp_server tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    ~udp_server () {}

    error_code close ()
    {
        return socket_finalizer{& _fd, false}();
    }

//     friend udp_server make_udp_server (std::string const & servername
//             , uint16_t port
//             , bool nonblocking
//             , int max_pending_connections
//             , error_code & ec);
};

// inline tcp_server make_tcp_server (std::string const & servername
//         , uint16_t port
//         , bool nonblocking
//         , int max_pending_connections
//         , error_code & ec)
// {
//     tcp_server server;
//     ec = server.open(servername, port, nonblocking, max_pending_connections);
//     return server;
// }
//
// inline tcp_server make_tcp_server (std::string const & servername
//         , uint16_t port
//         , bool nonblocking
//         , error_code & ec)
// {
//     return make_tcp_server(servername, port, nonblocking, 30, ec);
// }
//
// inline tcp_server make_tcp_server (std::string const & servername
//         , uint16_t port
//         , bool nonblocking
//         , int max_pending_connections)
// {
//     error_code ec;
//     auto s = make_tcp_server(servername
//             , port
//             , nonblocking
//             , max_pending_connections
//             , ec);
//     if (ec) throw exception(ec);
//     return s;
// }
//
// inline tcp_server make_tcp_server (std::string const & servername
//         , uint16_t port
//         , bool nonblocking)
// {
//     return make_tcp_server(servername, port, nonblocking, 30);
// }

}} // pfs::io
