#pragma once
#include "basic_file.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace pfs {
namespace io {

enum permission {
      permission_none = 0
    , owner_read   = 1 << 0
    , owner_write  = 1 << 1
    , owner_exec   = 1 << 2
    , group_read   = 1 << 3
    , group_write  = 1 << 4
    , group_exec   = 1 << 5
    , others_read  = 1 << 6
    , others_write = 1 << 7
    , others_exec  = 1 << 8
};

using permissions = std::underlying_type<permission>::type;

class file : public basic_file
{
private:
    static bool is_permission_enabled (permissions perms, permission perm)
    {
        return (perms & perm) != permission_none;
    }

    static int to_native_perms (permissions perms)
    {
        int result = 0;

        if (is_permission_enabled(perms, permission::owner_read))   result |= S_IRUSR;
        if (is_permission_enabled(perms, permission::owner_write))  result |= S_IWUSR;
        if (is_permission_enabled(perms, permission::owner_exec))   result |= S_IXUSR;
        if (is_permission_enabled(perms, permission::group_read))   result |= S_IRGRP;
        if (is_permission_enabled(perms, permission::group_write))  result |= S_IWGRP;
        if (is_permission_enabled(perms, permission::group_exec))   result |= S_IXGRP;
        if (is_permission_enabled(perms, permission::others_read))  result |= S_IROTH;
        if (is_permission_enabled(perms, permission::others_write)) result |= S_IWOTH;
        if (is_permission_enabled(perms, permission::others_exec))  result |= S_IXOTH;

        return result;
    }

public:
    error_code open (std::string const & path
            , open_mode_flags oflags
            , permissions perms = owner_read | owner_write)
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

        error_code ec;

        if (fd >= 0) {
            _fd = fd;
        } else {
            ec = get_last_system_error();
        }

        return ec;
    }

public:
    file () : basic_file() {}
    file (file const &) = delete;
    file & operator = (file const &) = delete;

    file (file && rhs) : basic_file()
    {
        swap(rhs);
    }

    file & operator = (file && rhs)
    {
        file tmp;
        rhs.swap(tmp);
        swap(tmp);
        return *this;
    }

    virtual ~file () {}

    device_type type () const noexcept override
    {
        return device_type::file;
    }

    // Inherited from basic_file.
    // open_mode_flags open_mode () const noexcept override

    // Inherited from basic_file.
    // error_code close () override

    // Inherited from basic_file.
    // bool opened () const noexcept override

    // Inherited from basic_file.
    // ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override

    // Inherited from basic_file.
    // ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
};

/**
 * Makes file device.
 */
device make_file (std::string const & path
        , open_mode_flags oflags
        , permissions perms
        , error_code & ec)
{
    file f;
    ec = f.open(path, oflags, perms);
    return ec ? device{} : device{new file(std::move(f))};
}

/**
 * Makes file device.
 */
device make_file (std::string const & path
        , open_mode_flags oflags
        , error_code & ec)
{
    return make_file(path
            , oflags
            , owner_read | owner_write
            , ec);
}

/**
 * Makes file device.
 */
device make_file (std::string const & path
        , open_mode_flags oflags
        , permissions perms)
{
    error_code ec;
    auto d = make_file(path, oflags, perms, ec);
    if (ec) throw exception(ec);
    return d;
}

/**
 * Makes file device.
 */
device make_file (std::string const & path
        , open_mode_flags oflags)
{
    error_code ec;
    auto d = make_file(path, oflags, ec);
    if (ec) throw exception(ec);
    return d;
}

}} // pfs::io

