////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.09 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "inet_socket.hpp"

namespace pfs {
namespace io {

//       reuse_addr   = 0x0001 // SO_REUSEADDR
//, so_type       = 0x00000002 // SO_TYPE
// 0x00000004 //#define SO_ERROR
// 0x00000008 //#define SO_DONTROUTE
// 0x00000010 //#define SO_BROADCAST
// 0x00000020 //#define SO_SNDBUF
// 0x00000040 //#define SO_RCVBUF
// 0x00000080 //#define SO_SNDBUFFORCE
// 0x00000100 //#define SO_RCVBUFFORCE
//     , keep_alive = 0x0200 // SO_KEEPALIVE
//#define SO_OOBINLINE	10
//#define SO_NO_CHECK	11
//#define SO_PRIORITY	12
//#define SO_LINGER	13
//#define SO_BSDCOMPAT	14
//#define SO_REUSEPORT	15
//#ifndef SO_PASSCRED /* powerpc only differs in these */
//#define SO_PASSCRED	16
//#define SO_PEERCRED	17
//#define SO_RCVLOWAT	18
//#define SO_SNDLOWAT	19
//#define SO_RCVTIMEO	20
//#define SO_SNDTIMEO	21

class tcp_socket : public basic_socket
{
protected:
    tcp_socket (int fd) : basic_socket(fd) {}

    error_code open (std::string const & servername
            , uint16_t port
            , bool nonblocking)
    {
        inet_socket_initializer socket_initializer(& _fd
                , SOCK_STREAM
                , servername
                , port
                , nonblocking);
        auto ec = socket_initializer.open();
        return ec;
    }

public:
    tcp_socket () : basic_socket{} {}
    tcp_socket (tcp_socket const &) = delete;
    tcp_socket & operator = (tcp_socket const &) = delete;

    tcp_socket (tcp_socket && rhs)
    {
       swap(rhs);
    }

    tcp_socket & operator = (tcp_socket && rhs)
    {
        tcp_socket tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    virtual ~tcp_socket () {}

    device_type type () const noexcept override
    {
        return device_type::tcp_socket;
    }

    // Inherited from basic_file.
    // open_mode_flags open_mode () const noexcept override

    // Inherited from basic_socket.
    // error_code close () override

    // Inherited from basic_file.
    // bool opened () const noexcept override

    // Inherited from basic_socket.
    // ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override

    // Inherited from basic_socket.
    // ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override

    error_code enable_keep_alive (bool enable)
    {
        int optval = enable ? 1 : 0;
        int rc = setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, & optval, sizeof(optval));
        return rc < 0 ? get_last_system_error() : error_code{};
    }

    friend device make_tcp_socket (std::string const & servername
            , uint16_t port
            , bool nonblocking
            , error_code & ec);
};

/**
 * Makes TCP socket.
 */
inline device make_tcp_socket (std::string const & servername
            , uint16_t port
            , bool nonblocking
            , error_code & ec)
{
    tcp_socket sock;
    ec = sock.open(servername, port, nonblocking);
    return ec ? device{} : device{new tcp_socket(std::move(sock))};
}

/**
 * Makes TCP socket.
 */
inline device make_tcp_socket (std::string const & servername
            , uint16_t port
            , bool nonblocking)
{
    error_code ec;
    auto d = make_tcp_socket(servername, port, nonblocking, ec);
    if (ec)
        throw exception(ec);
    return d;
}

}} // pfs::io
