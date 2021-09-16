////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.14 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "operationsystem.h"
#include "device.hpp"

#if defined(PFS_OS_LINUX)
#   include "unix_socket.hpp"
#else
#   error "Unsupported platform"
#endif

namespace pfs {
namespace io {

namespace platform {
namespace udp {

#if defined(PFS_OS_LINUX)
    using device_handle = unix_ns::device_handle;
    using host_address = unix_ns::host_address;
    using unix_ns::udp::open_mode;
    using unix_ns::udp::opened;
    using unix_ns::udp::open;
    using unix_ns::udp::close;
    using unix_ns::udp::read;
    using unix_ns::udp::write;
    using unix_ns::udp::has_pending_data;
    using unix_ns::swap;
#endif

}} // platform::udp

class udp_socket : public basic_device
{
public:
    using device_handle = platform::udp::device_handle;
    using host_address = platform::udp::host_address;

protected:
    device_handle _h;
    host_address  _addr;

protected:
    udp_socket (device_handle && h, host_address && addr)
    {
        using platform::udp::swap;
        swap(h, _h);
        swap(addr, _addr);
    }

public:
    udp_socket () : basic_device() {}
    udp_socket (udp_socket const &) = delete;
    udp_socket & operator = (udp_socket const &) = delete;

    udp_socket (udp_socket && rhs) : basic_device()
    {
        swap(rhs);
    }

    udp_socket & operator = (udp_socket && rhs)
    {
        udp_socket tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    virtual ~udp_socket ()
    {
        close();
    }

    virtual device_type type () const noexcept override
    {
        return device_type::udp_socket;
    }

    virtual open_mode_flags open_mode () const noexcept override
    {
        return platform::udp::open_mode(& _h);
    }

    virtual bool has_pending_data () noexcept override
    {
        return platform::udp::has_pending_data(& _h);
    }

    virtual error_code close () override
    {
        return platform::udp::close(& _h, true);
    }

    virtual bool opened () const noexcept override
    {
        return platform::udp::opened(& _h);
    }

    virtual ssize_t read (char * bytes
            , size_t n
            , error_code & ec) noexcept override
    {
        return platform::udp::read(& _h, & _addr, bytes, n, ec);
    }

    virtual ssize_t write (char const * bytes
            , size_t n
            , error_code & ec) noexcept override
    {
        return platform::udp::write(& _h, & _addr, bytes, n, ec);
    }

    ssize_t read_from (char * bytes
            , size_t n
            , host_address * paddr
            , error_code & ec) noexcept
    {
        return platform::udp::read(& _h, paddr, bytes, n, ec);
    }

    ssize_t write_to (char const * bytes
            , size_t n
            , host_address const * paddr
            , error_code & ec) noexcept
    {
        return platform::udp::write(& _h, paddr, bytes, n, ec);
    }

    void swap (udp_socket & rhs)
    {
        using platform::udp::swap;
        swap(_h, rhs._h);
        swap(_addr, rhs._addr);
    }

    friend device make_udp_socket (std::string const & servername
            , uint16_t port
            , bool nonblocking
            , error_code & ec);
};

/**
 * Makes UDP socket.
 */
inline device make_udp_socket (std::string const & servername
            , uint16_t port
            , bool nonblocking
            , error_code & ec)
{
    udp_socket::host_address addr;
    udp_socket::device_handle h = platform::udp::open(servername
            , port
            , nonblocking
            , & addr
            , ec);
    return ec ? device{} : device{new udp_socket(std::move(h), std::move(addr))};
}

/**
 * Makes UDP socket.
 */
inline device make_udp_socket (std::string const & servername
            , uint16_t port
            , bool nonblocking)
{
    error_code ec;
    auto d = make_udp_socket(servername, port, nonblocking, ec);
    if (ec)
        throw exception(ec);
    return d;
}

}} // pfs::io

