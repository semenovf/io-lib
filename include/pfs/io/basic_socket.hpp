#pragma once
#include "basic_file.hpp"
#include <sys/socket.h>

namespace pfs {
namespace io {

class basic_socket : public basic_file
{
protected:
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
};

}} // pfs::io



