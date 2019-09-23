////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2019.08.22 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "cxx11required.hpp"
#include <exception>
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

enum device_type_enum
{
      device_unknown = 0
    , device_null
    , device_buffer
    , device_stream
    , device_file
    , device_tcp_socket
    , device_tcp_peer
    , device_udp_socket
    , device_udp_peer
    , device_local_socket
    , device_local_peer
};

////////////////////////////////////////////////////////////////////////////////
// Error codes, category, exception class
////////////////////////////////////////////////////////////////////////////////
enum class errc
{
      success = 0
    , operation_in_progress
    , connection_refused
    , bad_file_descriptor
    , stream
    , timeout
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

            case static_cast<int>(errc::bad_file_descriptor):
                return std::string{"bad file descriptor"};

            case static_cast<int>(errc::stream):
                return std::string{"stream error"};

            case static_cast<int>(errc::timeout):
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

/**
 * @class pfs::io::exception
 */
using exception = std::system_error;

////////////////////////////////////////////////////////////////////////////////
// Basic device class
////////////////////////////////////////////////////////////////////////////////

template <typename NativeHandle = int>
class basic_device
{
public:
    using native_handle_type = NativeHandle;
    using open_mode_flags    = std::uint32_t;
    using open_mode_type     = open_mode_enum;

protected:
    native_handle_type _handle;

public:
    basic_device (native_handle_type && handle)
        : _handle{handle}
    {}

    virtual ~basic_device () {}

    virtual native_handle_type native_handle () const = 0;

//     virtual error_code reopen () = 0;
//
//     virtual open_mode_flags open_mode () const = 0;
//
//     virtual ssize_t available () const = 0;
//
//     bool at_end () const
//     {
//         return this->available() == ssize_t(0);
//     }

    /**
     * @note Implementation must read no more than @a n and return immediately.
     */
    virtual ssize_t read (char * bytes, size_t n, error_code & ec) noexcept = 0;

//     ssize_t read (byte_t * bytes, size_t n)
//     {
//         error_code ec;
//         ssize_t r = read(bytes, n, ec);
//         if (r < 0)
//             PFS_THROW(io_exception(ec));
//         return r;
//     }
//
//     ssize_t read (char * chars, size_t n, error_code & ec) noexcept
//     {
//         return this->read(reinterpret_cast<byte_t *>(chars), n, ec);
//     }
//
//     ssize_t read (char * chars, size_t n)
//     {
//         return this->read(reinterpret_cast<byte_t *>(chars), n);
//     }
//
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
//     virtual ssize_t write (byte_t const * bytes, size_t n, error_code & ec) noexcept = 0;
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
//
//     virtual error_code close () = 0;
//
//     virtual bool opened () const = 0;
//
//     virtual void flush () = 0;
//
//     virtual bool set_nonblocking (bool on) = 0;
//
//     virtual bool is_nonblocking () const = 0;
//
//     bool is_readable () const
//     {
//         return this->open_mode() | read_only;
//     }
//
//     bool is_writable () const
//     {
//         return this->open_mode() | write_only;
//     }
//
//     //virtual native_handle_type native_handle () const = 0;
//
//     virtual device_type type () const = 0;
};

}}} // pfs::io::details
