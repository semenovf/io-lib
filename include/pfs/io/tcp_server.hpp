////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.09 Initial version
//      2019.10.18 Refactored supporting platform-agnostic implementation
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "tcp_socket.hpp"

namespace pfs {
namespace io {

namespace platform {

#if defined(PFS_OS_UNIX)
    using device_handle = unix_ns::device_handle;
    using unix_ns::open_tcp_server;
    using unix_ns::close_socket;
    using unix_ns::accept_tcp_socket;
#endif

} // platform

class tcp_server;

class tcp_peer : public tcp_socket
{
    friend class tcp_server;

protected:
    tcp_peer (platform::device_handle && h)
        : tcp_socket(std::forward<platform::device_handle>(h))
    {}

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
    platform::device_handle _h;

protected:
    tcp_server (platform::device_handle && h)
    {
        using platform::swap;
        swap(h, _h);
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

    virtual ~tcp_server ()
    {
        close();
    }

    error_code close ()
    {
        return platform::close_socket(& _h, false);
    }

    device accept (error_code & ec)
    {
        platform::device_handle h = platform::accept_tcp_socket(& _h, ec);
        return ec ? device{} : device{new tcp_peer{std::move(h)}};
    }

    void swap (tcp_server & rhs)
    {
        using platform::swap;
        swap(_h, rhs._h);
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
    platform::device_handle h = platform::open_tcp_server(servername
            , port
            , nonblocking
            , max_pending_connections
            , ec);
    return ec ? tcp_server{} : tcp_server{std::move(h)};
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




