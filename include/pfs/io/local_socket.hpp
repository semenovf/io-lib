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

#if defined(PFS_OS_UNIX)
    using device_handle = unix_ns::device_handle;
    using unix_ns::open_mode;
    using unix_ns::open_local_socket;
    using unix_ns::close_socket;
    using unix_ns::opened;
    using unix_ns::read_socket;
    using unix_ns::write_socket;
    using unix_ns::swap;
#endif

} // platform

class local_socket : public basic_device
{
    platform::device_handle _h;

protected:
    local_socket (platform::device_handle && h)
    {
        using platform::swap;
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

    void swap (local_socket & rhs)
    {
        using platform::swap;
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
    platform::device_handle h = platform::open_local_socket(name, nonblocking, ec);
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


