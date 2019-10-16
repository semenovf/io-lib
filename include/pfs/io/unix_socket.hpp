////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.16 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "unix_file.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>

namespace pfs {
namespace io {
namespace unix_ns {

////////////////////////////////////////////////////////////////////////////////
// open_local_socket
////////////////////////////////////////////////////////////////////////////////
inline device_handle open_local_socket (std::string const & name
        , bool nonblocking
        , error_code & ec)
{
    sockaddr_un saddr;
    device_handle result;

    do {
        if (sizeof(saddr.sun_path) < name.size() + 1) {
            ec = make_error_code(errc::invalid_argument);
            break;
        }

        int socktype = SOCK_STREAM;

        if (nonblocking)
            socktype |= SOCK_NONBLOCK;

        int fd = ::socket(AF_LOCAL, socktype, 0);

        if (fd < 0) {
            ec = get_last_system_error();
            break;
        }

        memset(& saddr, 0, sizeof(saddr));

        saddr.sun_family = AF_LOCAL;
        memcpy(saddr.sun_path, name.c_str(), name.size());
        saddr.sun_path[name.size()] = '\0';

        int rc = ::connect(fd
                , reinterpret_cast<sockaddr *>(& saddr)
                , sizeof(saddr));

        if (rc < 0) {
            ec = get_last_system_error();
            break;
        }

        result.fd = fd;
    } while (false);

    if (ec && result.fd > 0) {
        ::close(result.fd);
        return device_handle{};
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// open_local_server
////////////////////////////////////////////////////////////////////////////////
inline device_handle open_local_server (std::string const & name
        , bool nonblocking
        , int max_pending_connections
        , error_code & ec)
{
    sockaddr_un saddr;
    int fd = -1;

    do {
        if (sizeof(saddr.sun_path) < name.size() + 1) {
            ec = make_error_code(errc::invalid_argument);
            break;
        }

        int socktype = SOCK_STREAM;

        if (nonblocking)
            socktype |= SOCK_NONBLOCK;

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
    } while (false);

    if (ec && fd > 0) {
        ::close(fd);
        fd = -1;
    }

    return fd < 0 ? device_handle{} : device_handle{fd};
}

////////////////////////////////////////////////////////////////////////////////
// accept_local_socket
////////////////////////////////////////////////////////////////////////////////
inline device_handle accept_local_socket (device_handle * h, error_code & ec)
{
    sockaddr_un peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);

    int peer_fd = ::accept(h->fd
            , reinterpret_cast<sockaddr *> (& peer_addr)
            , & peer_addr_len);

    if (peer_fd < 0)
        ec = get_last_system_error();

    return peer_fd < 0 ? device_handle{} : device_handle{peer_fd};
}

////////////////////////////////////////////////////////////////////////////////
// close_socket
////////////////////////////////////////////////////////////////////////////////
inline error_code close_socket (device_handle * h, bool force_shutdown)
{
    error_code ec;

    if (h->fd > 0) {
        if (force_shutdown)
            shutdown(h->fd, SHUT_RDWR);

        if (::close(h->fd) < 0)
            ec = get_last_system_error();
    }

    h->fd = -1;
    return ec;
}

//
// return -1 on error.
//
inline ssize_t read_socket (device_handle * h
        , char * bytes
        , size_t n
        , error_code & ec) noexcept
{
    ssize_t rc = recv(h->fd, bytes, n, 0);

    if (rc < 0
            && errno == EAGAIN
            || (EAGAIN != EWOULDBLOCK && errno == EWOULDBLOCK))
        rc = 0;

    if (rc < 0)
        ec = get_last_system_error();

    return rc;
}

//
// return -1 on error.
//
ssize_t write_socket (device_handle * h
        , char const * bytes
        , size_t n
        , error_code & ec) noexcept
{
    int total_written = 0; // total sent

    while (n) {
        // MSG_NOSIGNAL flag means:
        // requests not to send SIGPIPE on errors on stream oriented sockets
        // when the other end breaks the connection.
        // The EPIPE error is still returned.
        ssize_t written = send(h->fd, bytes + total_written, n, MSG_NOSIGNAL);

        if (written < 0) {
            if (errno == EAGAIN
                    || (EAGAIN != EWOULDBLOCK && errno == EWOULDBLOCK))
                continue;

            total_written = -1;
            break;
        }

        total_written += written;
        n -= written;
    }

    if (total_written < 0)
        ec = get_last_system_error();

    return total_written;
}

}}} // pfs::io::unix_ns
