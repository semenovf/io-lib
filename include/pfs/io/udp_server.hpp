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

namespace pfs {
namespace io {

namespace platform {
namespace udp {

#if defined(PFS_OS_UNIX)
    using unix_ns::udp::open_server;
    using unix_ns::swap;
#endif

}} // platform::udp

class udp_server : public udp_socket
{
public:
    using device_handle = platform::udp::device_handle;
    using host_address = platform::udp::host_address;

protected:
    udp_server (device_handle && h)
        : udp_socket(std::forward<device_handle>(h), host_address{})
    {}

public:
    udp_server () : udp_socket() {}
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

    virtual ~udp_server () {}

    friend udp_server make_udp_server (std::string const & servername
            , uint16_t port
            , bool nonblocking
            , error_code & ec);
};

inline udp_server make_udp_server (std::string const & servername
        , uint16_t port
        , bool nonblocking
        , error_code & ec)
{
    udp_server::device_handle h = platform::udp::open_server(servername
            , port
            , nonblocking
            , ec);
    return ec ? udp_server{} : udp_server{std::move(h)};
}

inline udp_server make_udp_server (std::string const & servername
        , uint16_t port
        , bool nonblocking)
{
    error_code ec;
    auto s = make_udp_server(servername
            , port
            , nonblocking
            , ec);
    if (ec) throw exception(ec);
    return s;
}

}} // pfs::io
