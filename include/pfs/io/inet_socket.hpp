#pragma once
#include "basic_socket.hpp"
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cassert>

namespace pfs {
namespace io {

union inet46_addr
{
    sockaddr_in  serveraddr;
    sockaddr_in6 serveraddr6;
};

class inet_socket_initializer
{
    int * _fd = nullptr;
    int _socktype = SOCK_STREAM;
    std::string _servername;
    uint16_t _port = 0;
    bool _nonblocking = false;

    // Value greater or equals to zero (>= 0)
    // is a flag for listener initialization
    int _max_pending_connections = -1;

    // Stores result address
    inet46_addr _sa;
    size_t _addrlen = 0;

public:
    inet_socket_initializer (int * fd
            , int socktype
            , std::string const & servername
            , uint16_t port
            , bool nonblocking
            , int max_pending_connections = -1)
        : _fd(fd)
        , _socktype(socktype)
        , _servername(servername)
        , _port(port)
        , _nonblocking(nonblocking)
        , _max_pending_connections(max_pending_connections)
    {}

    size_t copy_addr (inet46_addr * dest)
    {
        std::memcpy(dest, & _sa, sizeof(inet46_addr));
        return _addrlen;
    }

    error_code open ()
    {
        bool is_listener = _max_pending_connections >= 0;

        int socktype = _socktype;

        if (_nonblocking)
            socktype |= SOCK_NONBLOCK;

        error_code ec;

        int fd = -1;
        addrinfo host_addrinfo;
        host_addrinfo.ai_family    = AF_INET;
        host_addrinfo.ai_socktype  = _socktype;
        host_addrinfo.ai_protocol  = 0;
        host_addrinfo.ai_addrlen   = 0;
        host_addrinfo.ai_addr      = nullptr;
        host_addrinfo.ai_canonname = nullptr;
        host_addrinfo.ai_next      = nullptr;

        addrinfo * result_addr = nullptr;

        do {
            int rc = 0;

            memset(& _sa.serveraddr, 0, sizeof(_sa.serveraddr));
            _sa.serveraddr.sin_family = AF_INET;
            _sa.serveraddr.sin_port   = htons(_port);
            rc = inet_pton(AF_INET, _servername.c_str(), & _sa.serveraddr.sin_addr.s_addr);

            // Success
            if (rc > 0) {
                host_addrinfo.ai_family = AF_INET;
                host_addrinfo.ai_addrlen = sizeof(_sa.serveraddr);
                host_addrinfo.ai_addr = reinterpret_cast<sockaddr *>(& _sa.serveraddr);
                result_addr = & host_addrinfo;
            } else {
                memset(& _sa.serveraddr6, 0, sizeof(_sa.serveraddr6));
                _sa.serveraddr6.sin6_family = AF_INET6;
                _sa.serveraddr6.sin6_port   = htons(_port);
                rc = inet_pton(AF_INET6, _servername.c_str(), & _sa.serveraddr6.sin6_addr.s6_addr);

                // Success
                if (rc > 0) {
                    host_addrinfo.ai_family = AF_INET6;
                    host_addrinfo.ai_addrlen = sizeof(_sa.serveraddr6);
                    host_addrinfo.ai_addr = reinterpret_cast<sockaddr *>(& _sa.serveraddr6);
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
                hints.ai_socktype = _socktype;

                rc = getaddrinfo(_servername.c_str(), nullptr, & hints, & result_addr);

                if (rc != 0) {
                    ec = make_error_code(errc::host_not_found);
                    break;
                }
            }

            if (result_addr) {
                for (addrinfo * p = result_addr; p != nullptr; p = result_addr->ai_next) {
                    reinterpret_cast<sockaddr_in*>(p->ai_addr)->sin_port = htons(_port);

                    fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                    if (fd < 0) {
                        ec = get_last_system_error();
                        continue;
                    }

                    if (is_listener) {
                        // The setsockopt() function is used to allow the local
                        // address to be reused when the server is restarted
                        // before the required wait time expires.
                        int on = 1;
                        rc = setsockopt(fd
                                , SOL_SOCKET
                                , SO_REUSEADDR
                                , reinterpret_cast<char *>(& on)
                                , sizeof(on));

                        if (rc == 0) {
                            rc = ::bind(fd, p->ai_addr, p->ai_addrlen);

                            if (rc == 0) {
                                rc = ::listen(fd, _max_pending_connections);
                            }
                        }
                    } else {
                        // TCP socket
                        if (_socktype == SOCK_STREAM)
                            rc = ::connect(fd, p->ai_addr, p->ai_addrlen);

                        // UDP socket
                        if (_socktype == SOCK_DGRAM) {
                            assert(sizeof(_sa.serveraddr) == p->ai_addrlen
                                    || sizeof(_sa.serveraddr6) == p->ai_addrlen);
                            std::memcpy(& _sa.serveraddr, p->ai_addr, p->ai_addrlen);
                        }
                    }

                    if (rc < 0) {
                        ec = get_last_system_error();
                        ::close(fd);
                        fd = -1;
                        continue;
                    }

                    break;
                }
            }

            if (fd < 0)
                break;

            if (ec) {
                ::close(fd);
                break;
            }

            *_fd = fd;
        } while (false);

        if (result_addr && result_addr != & host_addrinfo) {
            freeaddrinfo(result_addr);
        }

        return ec;
    }
};

}} // namespace pfs::io
