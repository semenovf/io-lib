////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.09 Initial version
//      2019.10.16 Refactored supporting platform-agnostic implementation
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "operationsystem.h"
#include "device.hpp"

#if defined(PFS_OS_UNIX)
#   include "unix_socket.hpp"
#else
#   error "Unsupported platform"
#endif

namespace pfs {
namespace io {

namespace platform {

#if defined(PFS_OS_UNIX)
    using device_handle = unix_ns::device_handle;
    using unix_ns::open_mode;
    using unix_ns::open_tcp_socket;
    using unix_ns::close_socket;
    using unix_ns::opened;
    using unix_ns::read_socket;
    using unix_ns::write_socket;
    using unix_ns::enable_keep_alive;
    using unix_ns::swap;
#endif

} // platform

class tcp_socket : public basic_device
{
    platform::device_handle _h;

protected:
    tcp_socket (platform::device_handle && h)
    {
        using platform::swap;
        swap(h, _h);
    }
public:
    tcp_socket () : basic_device() {}
    tcp_socket (tcp_socket const &) = delete;
    tcp_socket & operator = (tcp_socket const &) = delete;

    tcp_socket (tcp_socket && rhs) : basic_device()
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

    virtual ~tcp_socket ()
    {
        close();
    }

    virtual device_type type () const noexcept override
    {
        return device_type::tcp_socket;
    }

    virtual open_mode_flags open_mode () const noexcept override
    {
        return platform::open_mode(& _h);
    }

    virtual error_code close () override
    {
        return platform::close_socket(& _h, true);
    }

    virtual bool opened () const noexcept override
    {
        return platform::opened(& _h);
    }

    ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
    {
        return platform::read_socket(& _h, bytes, n, ec);
    }

    ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
    {
        return platform::write_socket(& _h, bytes, n, ec);
    }

    void swap (tcp_socket & rhs)
    {
        using platform::swap;
        swap(_h, rhs._h);
    }

    error_code enable_keep_alive (bool enable)
    {
        return platform::enable_keep_alive(& _h, enable);
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
    platform::device_handle h = platform::open_tcp_socket(servername
            , port
            , nonblocking
            , ec);
    return ec ? device{} : device{new tcp_socket(std::move(h))};
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
