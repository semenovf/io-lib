////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.10.14 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "inet_socket.hpp"

namespace pfs {
namespace io {

class udp_socket : public basic_socket
{
    inet46_addr _sa;
    size_t _addrlen = 0;

protected:
    udp_socket (int fd) : basic_socket(fd) {}

    error_code open (std::string const & servername
            , uint16_t port
            , bool nonblocking)
    {
        inet_socket_initializer socket_initializer(& _fd
                , SOCK_DGRAM
                , servername
                , port
                , nonblocking);
        auto ec = socket_initializer.open();

        if (!ec) {
            _addrlen = socket_initializer.copy_addr(& _sa);
        }

        return ec;
    }

public:
    udp_socket () : basic_socket{} {}
    udp_socket (udp_socket const &) = delete;
    udp_socket & operator = (udp_socket const &) = delete;

    udp_socket (udp_socket && rhs)
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

    virtual ~udp_socket () {}

    device_type type () const noexcept override
    {
        return device_type::udp_socket;
    }

    // Inherited from basic_file.
    // open_mode_flags open_mode () const noexcept override

    error_code close () override
    {
        return socket_finalizer{& _fd, false}();
    }

    // Inherited from basic_file.
    // bool opened () const noexcept override

    // Inherited from basic_socket.
    // ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override

    ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
    {
        int total_written = 0; // total sent

        while (n) {
            // MSG_NOSIGNAL flag means:
            // requests not to send SIGPIPE on errors on stream oriented sockets
            // when the other end breaks the connection.
            // The EPIPE error is still returned.
            ssize_t written = sendto(_fd, bytes + total_written, n
                    , MSG_NOSIGNAL
                    , reinterpret_cast<sockaddr *>(& _sa.serveraddr)
                    , _addrlen);

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
    udp_socket sock;
    ec = sock.open(servername, port, nonblocking);
    return ec ? device{} : device{new udp_socket(std::move(sock))};
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

