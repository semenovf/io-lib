////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.08 Initial version
//      2019.10.16 Refactored supporting platform-agnostic implementation
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "local_socket.hpp"
#include <unistd.h>

namespace pfs {
namespace io {

namespace platform {

#if defined(PFS_OS_UNIX)
    using unix_ns::open_local_server;
    using unix_ns::close_socket;
    using unix_ns::accept_local_socket;
#endif

} // platform

class local_server;

class local_peer : public local_socket
{
    friend class local_server;

protected:
    local_peer (platform::device_handle && h)
        : local_socket{std::forward<platform::device_handle>(h)}
    {}

public:
    local_peer (local_peer const & rhs) = delete;
    local_peer & operator = (local_peer const & rhs) = delete;

    local_peer (local_peer && rhs)
        : local_socket(std::forward<local_socket>(rhs))
    {}

    local_peer & operator = (local_peer && rhs)
    {
        local_socket::operator = (std::forward<local_socket>(rhs));
        return *this;
    }

    device_type type () const noexcept override
    {
        return device_type::local_peer;
    }
};

class local_server
{
    platform::device_handle _h;

protected:
    local_server (platform::device_handle && h)
    {
        using platform::swap;
        swap(h, _h);
    }

public:
    local_server () {}
    local_server (local_server const &) = delete;
    local_server & operator = (local_server const &) = delete;

    local_server (local_server && rhs)
    {
        swap(rhs);
    }

    local_server & operator = (local_server && rhs)
    {
        local_server tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    virtual ~local_server ()
    {
        close();
    }

    error_code close ()
    {
        return platform::close_socket(& _h, false);
    }

    device accept (error_code & ec)
    {
        platform::device_handle h = platform::accept_local_socket(& _h, ec);
        return ec ? device{} : device{new local_peer{std::move(h)}};
    }

    void swap (local_server & rhs)
    {
        using platform::swap;
        swap(_h, rhs._h);
    }

    friend local_server make_local_server (std::string const & name
            , bool nonblocking
            , int max_pending_connections
            , error_code & ec);
};

inline local_server make_local_server (std::string const & name
        , bool nonblocking
        , int max_pending_connections
        , error_code & ec)
{
    platform::device_handle h = platform::open_local_server(name
            , nonblocking
            , max_pending_connections
            , ec);
    return ec ? local_server{} : local_server{std::move(h)};
}

inline local_server make_local_server (std::string const & name
        , bool nonblocking
        , error_code & ec)
{
    return make_local_server(name, nonblocking, 30, ec);
}

inline local_server make_local_server (std::string const & name
        , bool nonblocking
        , int max_pending_connections)
{
    error_code ec;
    auto s = make_local_server(name, nonblocking, max_pending_connections, ec);
    if (ec) throw exception(ec);
    return s;
}

inline local_server make_local_server (std::string const & name
            , bool nonblocking)
{
    return make_local_server(name, nonblocking, 30);
}

}} // pfs::io
