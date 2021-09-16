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
namespace tcp {

#if defined(PFS_OS_LINUX)
    using device_handle = unix_ns::device_handle;
    using unix_ns::tcp::close;
    using unix_ns::tcp::open_server;
    using unix_ns::tcp::accept;
    using unix_ns::swap;
#endif

}} // platform::tcp

class tcp_server;

class tcp_peer : public tcp_socket
{
    friend class tcp_server;

protected:
    tcp_peer (platform::tcp::device_handle && h)
        : tcp_socket(std::move(h))
    {}

public:
    tcp_peer (tcp_peer const & rhs) = delete;
    tcp_peer & operator = (tcp_peer const & rhs) = delete;

    tcp_peer (tcp_peer && rhs)
        : tcp_socket(std::move(rhs))
    {}

    tcp_peer & operator = (tcp_peer && rhs)
    {
        tcp_socket::operator = (std::move(rhs));
        return *this;
    }

    device_type type () const noexcept override
    {
        return device_type::tcp_peer;
    }
};

class tcp_server
{
public:
    using device_handle = platform::tcp::device_handle;

protected:
    device_handle _h;

protected:
    tcp_server (device_handle && h)
    {
        using platform::tcp::swap;
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
        return platform::tcp::close(& _h, false);
    }

    device accept (error_code & ec)
    {
        device_handle h = platform::tcp::accept(& _h, ec);
        return ec ? device{} : device{new tcp_peer{std::move(h)}};
    }

    void swap (tcp_server & rhs)
    {
        using platform::tcp::swap;
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
    tcp_server::device_handle h = platform::tcp::open_server(servername
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




