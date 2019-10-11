////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.09.29 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "basic_socket.hpp"
#include <cstring>
#include <sys/un.h>
#include <sys/socket.h>

namespace pfs {
namespace io {

class local_socket : public basic_socket
{
protected:
    local_socket (int fd) : basic_socket(fd) {}

    error_code open (std::string const & name, bool nonblocking)
    {
        sockaddr_un saddr;

        if (sizeof(saddr.sun_path) < name.size() + 1)
            return make_error_code(errc::invalid_argument);

        int socktype = SOCK_STREAM;

        if (nonblocking)
            socktype |= SOCK_NONBLOCK;

        error_code ec;
        int fd = ::socket(PF_LOCAL, socktype, 0);

        if (fd >= 0) {
            memset(& saddr, 0, sizeof(saddr));

            saddr.sun_family = AF_LOCAL;
            memcpy(saddr.sun_path, name.c_str(), name.size());
            saddr.sun_path[name.size()] = '\0';

            int rc = ::connect(fd
                    , reinterpret_cast<struct sockaddr *>(& saddr)
                    , sizeof(saddr));

            if (rc < 0) {
                ec = get_last_system_error();
                ::close(fd);
            } else {
                _fd = fd;
            }
        } else {
            ec = get_last_system_error();
        }

        return ec;
    }

public:
    local_socket () {}
    local_socket (local_socket const &) = delete;
    local_socket & operator = (local_socket const &) = delete;

    local_socket (local_socket && rhs)
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

    virtual ~local_socket () {}

    device_type type () const noexcept override
    {
        return device_type::local_socket;
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
    local_socket sock;
    ec = sock.open(name, nonblocking);
    return ec ? device{} : device{new local_socket(std::move(sock))};
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


