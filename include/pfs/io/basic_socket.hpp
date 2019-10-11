#pragma once
#include "basic_file.hpp"
#include <sys/socket.h>

namespace pfs {
namespace io {

class basic_socket : public basic_file
{
protected:
    basic_socket (int fd) : basic_file(fd) {}
    basic_socket () {}

public:
    virtual ~basic_socket () {}

    error_code close () override
    {
        error_code ec;

        if (_fd > 0) {
            shutdown(_fd, SHUT_RDWR);

            if (::close(_fd) < 0)
                ec = get_last_system_error();
        }

        _fd = -1;
        return ec;
    }

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
     * @return -1 on error.
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
};

}} // pfs::io



