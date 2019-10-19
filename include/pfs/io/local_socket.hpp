////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.09.29 Initial version
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
namespace local {

#if defined(PFS_OS_UNIX)
    using device_handle = unix_ns::device_handle;
    using unix_ns::local::open_mode;
    using unix_ns::local::opened;
    using unix_ns::local::open;
    using unix_ns::local::close;
    using unix_ns::local::read;
    using unix_ns::local::write;
    using unix_ns::local::has_pending_data;
    using unix_ns::swap;
#endif

}} // platform::local

class local_socket : public basic_device
{
public:
    using device_handle = platform::local::device_handle;

protected:
    device_handle _h;

protected:
    local_socket (device_handle && h)
    {
        using platform::local::swap;
        swap(h, _h);
    }

public:
    local_socket () : basic_device() {}
    local_socket (local_socket const &) = delete;
    local_socket & operator = (local_socket const &) = delete;

    local_socket (local_socket && rhs) : basic_device()
    {
        swap(rhs);
    }

    local_socket & operator = (local_socket && rhs)
    {
        local_socket tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    virtual ~local_socket ()
    {
        close();
    }

    virtual device_type type () const noexcept override
    {
        return device_type::local_socket;
    }

    virtual open_mode_flags open_mode () const noexcept override
    {
        return platform::local::open_mode(& _h);
    }

    virtual error_code close () override
    {
        return platform::local::close(& _h, true);
    }

    virtual bool opened () const noexcept override
    {
        return platform::local::opened(& _h);
    }

    virtual bool has_pending_data () noexcept override
    {
        return platform::local::has_pending_data(& _h);
    }

    virtual ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
    {
        return platform::local::read(& _h, bytes, n, ec);
    }

    virtual ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
    {
        return platform::local::write(& _h, bytes, n, ec);
    }

    void swap (local_socket & rhs)
    {
        using platform::local::swap;
        swap(_h, rhs._h);
    }

    friend device make_local_socket (std::string const & name
            , bool nonblocking
            , error_code & ec);
};

/**
 * Makes local socket.
 */
inline device make_local_socket (std::string const & name
        , bool nonblocking
        , error_code & ec)
{
    local_socket::device_handle h = platform::local::open(name, nonblocking, ec);
    return ec ? device{} : device{new local_socket(std::move(h))};
}

/**
 * Makes local socket.
 */
inline device make_local_socket (std::string const & name, bool nonblocking)
{
    error_code ec;
    auto d = make_local_socket(name, nonblocking, ec);
    if (ec) throw exception(ec);
    return d;
}

}} // pfs::io


