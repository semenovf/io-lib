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
#include <vector>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <cassert>

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
// open_inet_socket
////////////////////////////////////////////////////////////////////////////////
inline std::pair<native_handle,std::vector<uint8_t>> open_inet_socket (
          std::string const & servername
        , uint16_t port
        , int base_socktype
        , bool nonblocking
        , error_code & ec)
{
    native_handle fd = -1;
    std::vector<uint8_t> serveraddr;

    sockaddr_in  serveraddr4;
    sockaddr_in6 serveraddr6;

    int socktype = base_socktype;

    if (nonblocking)
        socktype |= SOCK_NONBLOCK;

    addrinfo host_addrinfo;
    host_addrinfo.ai_family    = AF_INET;
    host_addrinfo.ai_socktype  = socktype;
    host_addrinfo.ai_protocol  = 0;
    host_addrinfo.ai_addrlen   = 0;
    host_addrinfo.ai_addr      = nullptr;
    host_addrinfo.ai_canonname = nullptr;
    host_addrinfo.ai_next      = nullptr;

    addrinfo * result_addr = nullptr;

    do {
        int rc = 0;

        memset(& serveraddr4, 0, sizeof(serveraddr4));
        serveraddr4.sin_family = AF_INET;
        serveraddr4.sin_port   = htons(port);
        rc = inet_pton(AF_INET, servername.c_str(), & serveraddr4.sin_addr.s_addr);

        // Success
        if (rc > 0) {
            host_addrinfo.ai_family = AF_INET;
            host_addrinfo.ai_addrlen = sizeof(serveraddr4);
            host_addrinfo.ai_addr = reinterpret_cast<sockaddr *>(& serveraddr4);
            result_addr = & host_addrinfo;
        } else {
            memset(& serveraddr6, 0, sizeof(serveraddr6));
            serveraddr6.sin6_family = AF_INET6;
            serveraddr6.sin6_port   = htons(port);
            rc = inet_pton(AF_INET6, servername.c_str(), & serveraddr6.sin6_addr.s6_addr);

            // Success
            if (rc > 0) {
                host_addrinfo.ai_family = AF_INET6;
                host_addrinfo.ai_addrlen = sizeof(serveraddr6);
                host_addrinfo.ai_addr = reinterpret_cast<sockaddr *>(& serveraddr6);
                result_addr = & host_addrinfo;
            }
        }

        // servername does not contain a character string representing a
        // valid network address in the specified address family.
        if (!result_addr) {
            addrinfo hints;

            memset(& hints, 0, sizeof(hints));
            hints.ai_family   = AF_UNSPEC;
            hints.ai_flags    = AI_V4MAPPED;
            hints.ai_socktype = socktype;

            rc = getaddrinfo(servername.c_str(), nullptr, & hints, & result_addr);

            if (rc != 0) {
                ec = make_error_code(errc::host_not_found);
                break;
            }
        }

        if (result_addr) {
            for (addrinfo * p = result_addr; p != nullptr; p = result_addr->ai_next) {
                reinterpret_cast<sockaddr_in*>(p->ai_addr)->sin_port = htons(port);

                fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if (fd < 0) {
                    ec = get_last_system_error();
                    continue;
                }

                serveraddr = decltype(serveraddr)(reinterpret_cast<uint8_t *>(p->ai_addr)
                        , reinterpret_cast<uint8_t *>(p->ai_addr) + p->ai_addrlen);

                break;
            }
        }

        if (fd < 0)
            break;

        if (ec) {
            ::close(fd);
            break;
        }
    } while (false);

    if (result_addr && result_addr != & host_addrinfo) {
        freeaddrinfo(result_addr);
    }

    return std::make_pair(fd, serveraddr);
}

////////////////////////////////////////////////////////////////////////////////
// open_tcp_socket
////////////////////////////////////////////////////////////////////////////////
inline device_handle open_tcp_socket (std::string const & servername
        , uint16_t port
        , bool nonblocking
        , error_code & ec)
{
    auto credentials = open_inet_socket(servername
            , port
            , SOCK_STREAM
            , nonblocking
            , ec);

    native_handle & fd = credentials.first;
    auto addr = reinterpret_cast<sockaddr const *>(credentials.second.data());
    auto addrlen = static_cast<socklen_t>(credentials.second.size());

    if (fd >= 0) {
        int rc = ::connect(fd, addr, addrlen);

        if (rc < 0) {
            ec = get_last_system_error();
            ::close(fd);
            fd = -1;
        }
    }

    return fd < 0 ? device_handle{} : device_handle{fd};
}

////////////////////////////////////////////////////////////////////////////////
// open_tcp_server
////////////////////////////////////////////////////////////////////////////////
inline device_handle open_tcp_server (std::string const & servername
        , uint16_t port
        , bool nonblocking
        , int max_pending_connections
        , error_code & ec)
{
    auto credentials = open_inet_socket(servername
            , port
            , SOCK_STREAM
            , nonblocking
            , ec);

    native_handle & fd = credentials.first;
    auto addr = reinterpret_cast<sockaddr const *>(credentials.second.data());
    auto addrlen = static_cast<socklen_t>(credentials.second.size());

    if (fd >= 0) {
        // The setsockopt() function is used to allow the local
        // address to be reused when the server is restarted
        // before the required wait time expires.
        int on = 1;

        int rc = setsockopt(fd
                , SOL_SOCKET
                , SO_REUSEADDR
                , reinterpret_cast<char *>(& on)
                , sizeof(on));

        if (rc == 0) {
            rc = ::bind(fd, addr, addrlen);

            if (rc == 0) {
                rc = ::listen(fd, max_pending_connections);
            }

            if (rc < 0) {
                ec = get_last_system_error();
                ::close(fd);
                fd = -1;
            }
        }
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
// accept_tcp_socket
////////////////////////////////////////////////////////////////////////////////
inline device_handle accept_tcp_socket (device_handle * h, error_code & ec)
{
    sockaddr_in  peer_addr4;
    sockaddr_in6 peer_addr6;

    socklen_t peer_addr_len = std::max(sizeof(peer_addr4), sizeof(peer_addr6));
    sockaddr * peer_addr = sizeof(peer_addr4) > sizeof(peer_addr6)
            ? reinterpret_cast<sockaddr *> (& peer_addr4)
            : reinterpret_cast<sockaddr *> (& peer_addr6);

    auto peer_fd = ::accept(h->fd, peer_addr, & peer_addr_len);

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

////////////////////////////////////////////////////////////////////////////////
// enable_keep_alive
////////////////////////////////////////////////////////////////////////////////
error_code enable_keep_alive (device_handle * h, bool enable)
{
    int optval = enable ? 1 : 0;
    int rc = setsockopt(h->fd, SOL_SOCKET, SO_KEEPALIVE, & optval, sizeof(optval));
    return rc < 0 ? get_last_system_error() : error_code{};
}

}}} // pfs::io::unix_ns
