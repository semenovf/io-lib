////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.09.29 Initial version
//      2019.10.15 Refactored supporting platform-agnostic implementation
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "operationsystem.h"
#include "device.hpp"
#include <sys/types.h>
#include <sys/stat.h>

#if defined(PFS_OS_UNIX)
#   include "unix_file.hpp"
#else
#   error "Unsupported platform"
#endif

namespace pfs {
namespace io {

namespace platform {
namespace file {

#if defined(PFS_OS_UNIX)
    using device_handle = unix_ns::device_handle;
    using unix_ns::file::open_mode;
    using unix_ns::file::open;
    using unix_ns::file::close;
    using unix_ns::file::opened;
    using unix_ns::file::read;
    using unix_ns::file::write;
    using unix_ns::file::has_pending_data;
    using unix_ns::swap;
#endif

}} // platform::file

class file : public basic_device
{
    platform::file::device_handle _h;

protected:
    file (platform::file::device_handle && h)
    {
        using platform::file::swap;
        swap(h, _h);
    }

public:
    file () : basic_device() {}
    file (file const &) = delete;
    file & operator = (file const &) = delete;

    file (file && rhs) : basic_device()
    {
        swap(rhs);
    }

    file & operator = (file && rhs)
    {
        file tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    virtual ~file ()
    {
        close();
    }

    virtual device_type type () const noexcept override
    {
        return device_type::file;
    }

    virtual open_mode_flags open_mode () const noexcept override
    {
        return platform::file::open_mode(& _h);
    }

    virtual bool has_pending_data () noexcept
    {
        return platform::file::has_pending_data(& _h);
    }

    virtual error_code close () override
    {
        return platform::file::close(& _h);
    }

    virtual bool opened () const noexcept override
    {
        return platform::file::opened(& _h);
    }

    virtual ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
    {
        return platform::file::read(& _h, bytes, n, ec);
    }

    virtual ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
    {
        return platform::file::write(& _h, bytes, n, ec);
    }

    void swap (file & rhs)
    {
        using platform::file::swap;
        swap(_h, rhs._h);
    }

    friend device make_file (std::string const & path
        , open_mode_flags oflags
        , permissions perms
        , error_code & ec);
};

/**
 * Makes file device.
 */
inline device make_file (std::string const & path
    , open_mode_flags oflags
    , permissions perms
    , error_code & ec)
{
    platform::file::device_handle h = platform::file::open(path, oflags, perms, ec);
    return ec ? device{} : device{new file(std::move(h))};
}

/**
 * Makes file device.
 */
inline device make_file (std::string const & path
        , open_mode_flags oflags
        , error_code & ec)
{
    return make_file(path
            , oflags
            , owner_read | owner_write
            , ec);
}

/**
 * Makes file device.
 */
inline device make_file (std::string const & path
        , open_mode_flags oflags
        , permissions perms)
{
    error_code ec;
    auto d = make_file(path, oflags, perms, ec);
    if (ec) throw exception(ec);
    return d;
}

/**
 * Makes file device.
 */
inline device make_file (std::string const & path
        , open_mode_flags oflags)
{
    error_code ec;
    auto d = make_file(path, oflags, ec);
    if (ec) throw exception(ec);
    return d;
}

}} // pfs::io

