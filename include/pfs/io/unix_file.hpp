#pragma once
#include "device.hpp"
#include "permissions.hpp"
#include <utility>
#include <fcntl.h>
#include <unistd.h>

namespace pfs {
namespace io {
namespace unix_ns {

using native_handle = int;

struct device_handle
{
    native_handle fd = -1;
    device_handle () : fd{-1} {}
    device_handle (native_handle h) : fd{h} {}
};

inline bool permission_enabled (permissions perms, permission perm)
{
    return (perms & perm) != permission_none;
}

inline int to_native_perms (permissions perms)
{
    int result = 0;

    if (permission_enabled(perms, permission::owner_read))   result |= S_IRUSR;
    if (permission_enabled(perms, permission::owner_write))  result |= S_IWUSR;
    if (permission_enabled(perms, permission::owner_exec))   result |= S_IXUSR;
    if (permission_enabled(perms, permission::group_read))   result |= S_IRGRP;
    if (permission_enabled(perms, permission::group_write))  result |= S_IWGRP;
    if (permission_enabled(perms, permission::group_exec))   result |= S_IXGRP;
    if (permission_enabled(perms, permission::others_read))  result |= S_IROTH;
    if (permission_enabled(perms, permission::others_write)) result |= S_IWOTH;
    if (permission_enabled(perms, permission::others_exec))  result |= S_IXOTH;

    return result;
}

////////////////////////////////////////////////////////////////////////////////
inline void swap (device_handle & a, device_handle & b)
{
    using std::swap;
    swap(a.fd, b.fd);
}

////////////////////////////////////////////////////////////////////////////////
inline open_mode_flags open_mode (device_handle const * h) noexcept
{
    if (h->fd < 0)
        return not_open;

    open_mode_flags result = not_open;
    int status = ::fcntl(h->fd, F_GETFL);

    // NOTE Never pass (O_RDONLY is zero)
    //
    //if (status & O_RDONLY)
    //    result |= read_only;
    //
    // So checking for readable device make through attempt to direct read
    // from device
    {
        char buf[1] = {0};
        errno = 0;
        if (::read(h->fd, buf, 0) >= 0 && errno != EBADF)
            result |= read_only;
    }

    if (status & O_RDWR) {
        result |= read_only;
        result |= write_only;
    }

    if (status & O_WRONLY)
        result |= write_only;

    if (status & O_NONBLOCK || status & O_NDELAY)
        result |= non_blocking;

    return result;
}

////////////////////////////////////////////////////////////////////////////////
inline device_handle open (std::string const & path
        , open_mode_flags oflags
        , permissions perms// = owner_read | owner_write)
        , error_code & ec)
{
    int native_oflags = 0;
    mode_t native_mode = 0;

    if ((oflags & write_only) && (oflags & read_only)) {
        native_oflags |= O_RDWR;
        native_oflags |= O_CREAT;
        native_mode |= to_native_perms(perms);
    } else if (oflags & write_only) {
        native_oflags |= O_WRONLY;
        native_oflags |= O_CREAT;
        native_mode |= to_native_perms(perms);
    } else if (oflags & read_only) {
        native_oflags |= O_RDONLY;
    }

    if (oflags & non_blocking)
        native_oflags |= O_NONBLOCK;

    if (oflags & truncate)
        native_oflags |= O_TRUNC;

    int fd = -1;

    if (native_mode)
        fd = ::open(path.c_str(), native_oflags, native_mode);
    else
        fd = ::open(path.c_str(), native_oflags);

    if (fd >= 0)
        return device_handle{fd};

    ec = get_last_system_error();
    return device_handle{};
}

inline error_code close (device_handle * h)
{
    error_code ec;

    if (h->fd > 0) {
        if (fsync(h->fd) != 0)
            ec = get_last_system_error();

        if (::close(h->fd) < 0)
            ec = get_last_system_error();
    }

    h->fd = -1;
    return ec;
}

inline bool opened (device_handle const * h) noexcept
{
    return h->fd >= 0;
}

inline ssize_t read (device_handle * h
        , char * bytes
        , size_t n
        , error_code & ec) noexcept
{
    ssize_t sz = ::read(h->fd, bytes, n);

    if (sz < 0)
        ec = get_last_system_error();

    return sz;
}

inline ssize_t write (device_handle * h
        , char const * bytes
        , size_t n
        , error_code & ec) noexcept
{
    ssize_t sz = ::write(h->fd, bytes, n);

    if (sz < 0)
        ec = get_last_system_error();

    return sz;
}

}}} // pfs::io::unix_ns
