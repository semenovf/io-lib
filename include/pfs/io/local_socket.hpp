#pragma once
#include "basic_file.hpp"
#include <cstring>
#include <sys/un.h>
#include <sys/socket.h>

namespace pfs {
namespace io {

class local_socket : public basic_file
{
protected:
    local_socket (int fd) : basic_file(fd) {}

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

    /**
     * @return -1 on error.
     */
    ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
    {
        ssize_t rc = recv(_fd, bytes, n, 0);

        if (rc < 0
                && errno == EAGAIN
                || (EAGAIN != EWOULDBLOCK && errno == EWOULDBLOCK))
            rc = 0;

        if (rc < 0)
            ec = get_last_system_error();

        return rc;
    }

    /**
     */
    ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
    {
        int total_written = 0; // total sent

        while (n) {
            // MSG_NOSIGNAL flag means:
            // requests not to send SIGPIPE on errors on stream oriented sockets
            // when the other end breaks the connection.
            // The EPIPE error is still returned.
            ssize_t written = send(_fd, bytes + total_written, n, MSG_NOSIGNAL);

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

    friend device make_local_socket (std::string const & name
            , bool nonblocking
            , error_code & ec);

    friend device make_local_socket (std::string const & name
            , bool nonblocking);
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


