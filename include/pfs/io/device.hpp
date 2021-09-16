////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.08.22 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "operationsystem.h"
#include <exception>
#include <memory>
#include <string>
#include <system_error>
#include <cstdint>

namespace pfs {
namespace io {

using error_code = std::error_code;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
enum open_mode_enum
{
      not_open     = 0                       /**< Device is not opened */
    , read_only    = 0x0001                  /**< Open device for read only */
    , write_only   = 0x0002                  /**< Open device for write only */
    , read_write   = read_only | write_only  /**< Open device for read and write */
    , write_read   = read_write              /**< Synonym for read_write */
    , non_blocking = 0x0004                  /**< Open device in non-blocking mode */
    , truncate     = 0x0010                  /**< Create device (only for regular file device) */
};

using open_mode_flags = std::underlying_type<open_mode_enum>::type;

enum class device_type
{
      unknown = 0
    , null
    , buffer
//     , stream
    , file
    , local_socket
    , local_peer
    , tcp_socket
    , tcp_peer
    , udp_socket
};

////////////////////////////////////////////////////////////////////////////////
// Error codes, category, exception class
////////////////////////////////////////////////////////////////////////////////
enum class errc
{
      success = 0
    , operation_in_progress
    , connection_refused
    , already_connected

    // The device (socket) is not connected, and no target has been given.
    , not_connected
    , try_again
    , bad_file_descriptor

    // No such file or directory
    , file_not_found

    // The host address was not found.
    // May be occured while host address resolving (while inet socket open)
    , host_not_found

    // For connection-oriented sockets:
    //      The user tried to connect to a broadcast address without
    //      having the socket broadcast flag enabled or the connection
    //      request failed because of a local firewall rule.
    , permission_denied
    , invalid_argument
    , device_too_large
    , stream
    , timedout
};

class error_category : public std::error_category
{
public:
    virtual char const * name () const noexcept override
    {
        return "io_category";
    }

    virtual std::string message (int ev) const override
    {
        switch (ev) {
            case static_cast<int>(errc::success):
                return std::string{"no error"};

            case static_cast<int>(errc::operation_in_progress):
                return std::string{"operation in progress"};

            case static_cast<int>(errc::connection_refused):
                return std::string{"connection refused"};

            case static_cast<int>(errc::already_connected):
                return std::string{"already connected"};

            case static_cast<int>(errc::not_connected):
                return std::string{"not connected"};

            case static_cast<int>(errc::try_again):
                return std::string{"try again"};

            case static_cast<int>(errc::bad_file_descriptor):
                return std::string{"bad file descriptor"};

            case static_cast<int>(errc::file_not_found):
                return std::string{"file not found"};

            case static_cast<int>(errc::host_not_found):
                return std::string{"host not found"};

            case static_cast<int>(errc::permission_denied):
                return std::string{"permission denied"};

            case static_cast<int>(errc::invalid_argument):
                return std::string{"invalid argument"};

            case static_cast<int>(errc::device_too_large):
                return std::string{"device too large"};

            case static_cast<int>(errc::stream):
                return std::string{"stream error"};

            case static_cast<int>(errc::timedout):
                return std::string{"timed out"};

            default: return std::string{"unknown I/O error"};
        }
    }
};

inline std::error_category const & get_error_category ()
{
    static error_category instance;
    return instance;
}

inline std::error_code make_error_code (errc e)
{
    return std::error_code(static_cast<int>(e), get_error_category());
}

// TODO Crossplatform
// [Construct std::error_code from errno on POSIX and GetLastError() on Windows](https://stackoverflow.com/questions/13950938/construct-stderror-code-from-errno-on-posix-and-getlasterror-on-windows)
//
#if defined(PFS_OS_LINUX)
inline std::error_code make_error_code_from_errno (int e)
{
    switch(e) {
        case EINVAL      : return make_error_code(errc::invalid_argument);
        case ECONNREFUSED: return make_error_code(errc::connection_refused);
        case EINPROGRESS : return make_error_code(errc::operation_in_progress);
        case EISCONN     : return make_error_code(errc::already_connected);
        case ENOTCONN    : return make_error_code(errc::not_connected);
        case EAGAIN      : return make_error_code(errc::try_again);
        case EBADF       : return make_error_code(errc::bad_file_descriptor);
        case ETIMEDOUT   : return make_error_code(errc::timedout);
        case ENOENT      : return make_error_code(errc::file_not_found);
        case EACCES      :
        case EPERM       : return make_error_code(errc::permission_denied);

        default: break;
    }
    return error_code(errno, std::generic_category());
}
#endif

inline std::error_code get_last_system_error ()
{
#if defined(PFS_OS_WIN)
    return error_code(::GetLastError(), std::system_category());
#else  // POSIX
    return make_error_code_from_errno(errno);
#endif // defined(PFS_OS_WIN)
}

/**
 * @class pfs::io::exception
 */
using exception = std::system_error;

////////////////////////////////////////////////////////////////////////////////
// Basic device class
////////////////////////////////////////////////////////////////////////////////

class basic_device
{
public:
    basic_device () {}
    virtual ~basic_device () {}

    virtual device_type type () const noexcept = 0;

    virtual open_mode_flags open_mode () const noexcept = 0;

    virtual bool has_pending_data () noexcept = 0;

    virtual ssize_t read (char * bytes, size_t n, error_code & ec) noexcept = 0;

    virtual ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept = 0;

    virtual error_code close () = 0;

    virtual bool opened () const noexcept = 0;
};

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
template <typename T>
using unique_ptr = std::unique_ptr<T>;

class device
{
private:
    unique_ptr<basic_device> _d;

public:
    device () {}
    device (basic_device * d) : _d(d) {}
    device (device &&) = default;
    device & operator = (device &&) = default;

    device (device const &) = delete;
    device & operator = (device const &) = delete;

    ~device () {
        if (opened())
            close();
    }

    inline bool is_null ()  const noexcept
    {
        return _d.get() == nullptr;
    }

    inline device_type type () const noexcept
    {
        return _d->type();
    }

    inline open_mode_flags open_mode () const noexcept
    {
        return _d->open_mode();
    }

    inline bool has_pending_data () noexcept
    {
        return _d->has_pending_data();
    }

    inline bool is_readable () const noexcept
    {
        return _d->open_mode() & read_only;
    }

    inline bool is_writable () const noexcept
    {
        return this->open_mode() & write_only;
    }

    inline bool is_nonblocking () const noexcept
    {
        return this->open_mode() & non_blocking;
    }

    inline error_code close ()
    {
        return _d->close();
    }

    inline bool opened () const noexcept
    {
        return _d && _d->opened();
    }

    inline ssize_t read (char * bytes, size_t n, error_code & ec) noexcept
    {
        return _d->read(bytes, n, ec);
    }

    inline ssize_t read (char * bytes, size_t n)
    {
        error_code ec;
        ssize_t r = read(bytes, n, ec);
        if (r < 0) throw exception(ec);
        return r;
    }

    inline ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept
    {
        return _d->write(bytes, n, ec);
    }

    inline void swap (device & rhs)
    {
        _d.swap(rhs._d);
    }

    inline void invalidate ()
    {
        if (_d) {
            device tmp;
            swap(tmp);
        }
    }

//     ssize_t read (byte_string & bytes, size_t n, error_code & ec) noexcept
//     {
//         if (n == 0)
//             return 0;
//
//         size_t oldsize = bytes.size();
//
//         bytes.resize(oldsize + n);
//
//         ssize_t r = read(bytes.data() + oldsize, n, ec);
//
//         if (r < 0) {
//             // Restore previous state of buffer
//             bytes.resize(oldsize);
//             return r;
//         }
//
//         if (r > 0 && n != integral_cast_check<size_t>(r))
//             bytes.resize(oldsize + r);
//
//         return r;
//     }
//
//     ssize_t read (byte_string & bytes, size_t n)
//     {
//         error_code ec;
//         ssize_t r = read(bytes, n, ec);
//         if (r < 0)
//             PFS_THROW(io_exception(ec));
//         return r;
//     }
//
//     /**
//      * @brief Read data from device and appends them
//      */
//     ssize_t read (byte_string & bytes, error_code & ec) noexcept
//     {
//         return this->read(bytes, available(), ec);
//     }
//
//     ssize_t read (byte_string & bytes)
//     {
//         return this->read(bytes, available());
//     }
//
//     /**
//      * @param millis Timout in milliseconds for wait for reading @arg n bytes ().
//      *               A negative value in timeout means an infinite timeout.
//      *               Zero value means the behaviour as @c read method.
//      */
//     virtual ssize_t read_wait (byte_t * bytes, size_t n, error_code & ec, int millis) noexcept;
//
//     ssize_t read_wait (byte_t * bytes, size_t n, int millis)
//     {
//         error_code ec;
//         ssize_t r = read_wait(bytes, n, ec, millis);
//         if (r < 0)
//             PFS_THROW(io_exception(ec));
//         return r;
//     }
//
//     inline ssize_t read_wait (char * chars, size_t n, error_code & ec, int millis) noexcept
//     {
//         return this->read_wait(reinterpret_cast<byte_t *>(chars), n, ec, millis);
//     }
//
//     inline ssize_t read_wait (char * chars, size_t n, int millis)
//     {
//         return this->read_wait(reinterpret_cast<byte_t *>(chars), n, millis);
//     }
//
//     /**
//      * @brief Read @a n bytes from device.
//      *
//      * @param millis Timout in milliseconds for wait for reading @arg n bytes.
//      *               A negative value in timeout means an infinite timeout.
//      *               Zero value means the behaviour as @c read method.
//      * @return The number of bytes received, or -1 if an error occurred.
//      */
//     ssize_t read_wait (byte_string & bytes, size_t n, error_code & ec, int millis) noexcept;
//
//     ssize_t read_wait (byte_string & bytes, size_t n, int millis)
//     {
//         error_code ec;
//         ssize_t r = read_wait(bytes, n, ec, millis);
//         if (r < 0)
//             PFS_THROW(io_exception(ec));
//         return r;
//     }
//
//     /**
//      * @brief Read data from device and appends them
//      */
//     ssize_t read_wait (byte_string & bytes, error_code & ec, int millis) noexcept
//     {
//         return this->read_wait(bytes, available(), ec, millis);
//     }
//
//     ssize_t read_wait (byte_string & bytes, int millis)
//     {
//         return this->read_wait(bytes, available(), millis);
//     }
//
//     ssize_t write (byte_t const * bytes, size_t n)
//     {
//         error_code ec;
//         ssize_t r = write(bytes, n, ec);
//         if (r < 0)
//             PFS_THROW(io_exception(ec));
//         return r;
//     }
//
//     ssize_t write (const char * chars, size_t n, error_code & ec) noexcept
//     {
//         return this->write(reinterpret_cast<const byte_t *>(chars), n, ec);
//     }
//
//     ssize_t write (const char * chars, size_t n)
//     {
//         return this->write(reinterpret_cast<const byte_t *>(chars), n);
//     }
//
//     ssize_t write (byte_string const & bytes, size_t n, error_code & ec) noexcept
//     {
//         return this->write(bytes.data(), pfs::min(n, bytes.size()), ec);
//     }
//
//     ssize_t write (byte_string const & bytes, size_t n)
//     {
//         return this->write(bytes.data(), pfs::min(n, bytes.size()));
//     }
//
//     ssize_t write (byte_string const & bytes, error_code & ec) noexcept
//     {
//         return this->write(bytes.data(), bytes.size(), ec);
//     }
//
//     ssize_t write (byte_string const & bytes)
//     {
//         return this->write(bytes.data(), bytes.size());
//     }

    template <typename UnderlyingDevice>
    friend UnderlyingDevice * underlying_device (device & dev);

    template <typename UnderlyingDevice>
    friend UnderlyingDevice const * underlying_device (device const & dev);
};

template <typename UnderlyingDevice>
inline UnderlyingDevice * underlying_device (device & dev)
{
    return dev.type() == UnderlyingDevice{}.type()
            ? reinterpret_cast<UnderlyingDevice *>(dev._d.get())
            : nullptr;
}

template <typename UnderlyingDevice>
inline UnderlyingDevice const * underlying_device (device const & dev)
{
    return dev.type() == UnderlyingDevice{}.type()
            ? reinterpret_cast<UnderlyingDevice *>(dev._d.get())
            : nullptr;
}

}} // pfs::io
