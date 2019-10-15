////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.08 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "local_socket.hpp"
#include <unistd.h>

namespace pfs {
namespace io {

class local_server;

class local_peer : public local_socket
{
    friend class local_server;

protected:
    local_peer (int fd) : local_socket(fd) {}

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
    int _fd = -1;

protected:
    error_code open (std::string const & name
            , bool nonblocking
            , int max_pending_connections = 30)
    {
        sockaddr_un saddr;

        if (sizeof(saddr.sun_path) < name.size() + 1)
            return make_error_code(errc::invalid_argument);

        int socktype = SOCK_STREAM;
        socktype |= SOCK_CLOEXEC;

        if (nonblocking)
            socktype |= SOCK_NONBLOCK;

        error_code ec;

        int fd = -1;

        do {
            fd = ::socket(AF_LOCAL, socktype, 0);

            if (fd < 0) {
                ec = get_last_system_error();
                break;
            }

            memset(& saddr, 0, sizeof(saddr));

            saddr.sun_family = AF_LOCAL;
            memcpy(saddr.sun_path, name.c_str(), name.size());
            saddr.sun_path[name.size()] = '\0';

            // TODO File deletion must be more reasonable
            int rc = unlink(saddr.sun_path);

            rc = ::bind(fd
                    , reinterpret_cast<sockaddr *>(& saddr)
                    , sizeof(saddr));

            if (rc < 0) {
                ec = get_last_system_error();
                break;
            }

            rc = listen(fd, max_pending_connections);

            if (rc < 0) {
                ec = get_last_system_error();
                break;
            }

            _fd = fd;
        } while (false);

        if (ec && fd < 0) {
            ::close(fd);
        }

        return ec;
    }

    void swap (local_server & rhs)
    {
        using std::swap;
        swap(_fd, rhs._fd);
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

    ~local_server () {}

    error_code close ()
    {
        return socket_finalizer{& _fd, false}();
    }

    device accept (error_code & ec)
    {
        sockaddr_un peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);

        int peer_fd = ::accept(_fd
                , reinterpret_cast<sockaddr *> (& peer_addr)
                , & peer_addr_len);

        if (peer_fd < 0) {
            ec = get_last_system_error();
            return device{};
        }

        return device{new local_peer(peer_fd)};
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
    local_server server;
    ec = server.open(name, nonblocking, max_pending_connections);
    return server;
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



