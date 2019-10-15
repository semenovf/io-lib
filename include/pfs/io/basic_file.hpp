#pragma once
#include "device.hpp"

namespace pfs {
namespace io {

// TODO DEPRECATED
class basic_file : public basic_device
{
protected:
    int _fd = -1;

protected:
    basic_file () {}
    basic_file (int fd) : _fd(fd) {}

//     void swap (basic_file & rhs)
//     {
//         using std::swap;
//         swap(_fd, rhs._fd);
//     }

public:
    virtual ~basic_file ()
    {
        if (_fd > 0)
            close();
    }

//     virtual open_mode_flags open_mode () const noexcept override
//     {
//         if (_fd < 0)
//             return not_open;
//
//         open_mode_flags result = not_open;
//         int status = ::fcntl(_fd, F_GETFL);
//
//         // NOTE Never pass (O_RDONLY is zero)
//         //
//         //if (status & O_RDONLY)
//         //    result |= read_only;
//         //
//         // So checking for readable device make through attempt to direct read
//         // from device
//         {
//             char buf[1] = {0};
//             errno = 0;
//             if (::read(_fd, buf, 0) >= 0 && errno != EBADF)
//                 result |= read_only;
//         }
//
//         if (status & O_RDWR) {
//             result |= read_only;
//             result |= write_only;
//         }
//
//         if (status & O_WRONLY)
//             result |= write_only;
//
//         if (status & O_NONBLOCK || status & O_NDELAY)
//             result |= non_blocking;
//
//         return result;
//     }

//     virtual error_code close () override
//     {
//         error_code ec;
//
//         if (_fd > 0) {
//             if (fsync(_fd) != 0)
//                 ec = get_last_system_error();
//
//             if (::close(_fd) < 0)
//                 ec = get_last_system_error();
//         }
//
//         _fd = -1;
//         return ec;
//     }

//     virtual bool opened () const noexcept override
//     {
//         return _fd >= 0;
//     }

//     /**
//      * @return -1 on error.
//      */
//     virtual ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
//     {
//         ssize_t sz = ::read(_fd, bytes, n);
//
//         if (sz < 0)
//             ec = get_last_system_error();
//
//         return sz;
//     }

    /**
     */
//     virtual ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
//     {
//         ssize_t sz = ::write(_fd, bytes, n);
//
//         if (sz < 0)
//             ec = get_last_system_error();
//
//         return sz;
//     }
};

}} // pfs::io



