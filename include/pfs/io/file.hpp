#pragma once
#include "device.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace pfs { 
namespace io {

enum class permission {
      none         = 0
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

class file : public basic_device
{
    int _fd = -1;
    std::string _path;

private:
    static bool is_permission_enabled (permissions perms, permission perm)
    {
        return (perms & static_cast<permissions>(permission::owner_read))
                != static_cast<permissions>(permission::none));
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
            , permissions perms)
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
        
        
        int fd = ::open(path.c_str(), native_oflags, native_mode);
        error_code ec;

        if (fd >= 0) {
            _fd = fd;
            _path = path;
        } else {
            ec = get_last_system_error();
        }

        return ec;
    }
    
public:
    file () {}

    device_type type () const noexcept override
    {
        return device_type::file;
    }
    
    open_mode_flags open_mode () const noexcept override
    {
        if (_fd < 0)
            return not_open;
        
        open_mode_flags result = not_open;

        char buf[1] = {0};

        if (::read(_fd, buf, 0) >= 0 && errno != EBADF)
            result |= read_only;

        if (::write(_fd, buf, 0) >= 0 && errno != EBADF)
            result |= write_only;

        return result;
    }
    
    error_code close () override
    {
        error_code ec;

        if (_fd > 0) {
            if (fsync(_fd) != 0)
                ec = get_last_system_error();
            
            if (::close(_fd) < 0)
                ec = get_last_system_error();
        }

        _fd = -1;
        return ec;
    }
    
    bool opened () const noexcept override
    {
        return _fd >= 0;
    }
    
//     /**
//      * @return -1 on error.
//      * @exception errc::invalid_argument if buffer is unsuitable for reading
//      * (i.e. opened without read mode)
//      */
//     ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
//     {
//         if (!(_oflags & read_only)) {
//             ec = make_error_code(errc::invalid_argument);
//             return -1;
//         }
//         
//         if (_pos >= _c->size())
//             return 0;
// 
//         n = std::min(n, _c->size() - _pos);
//         _c->copy(bytes, n, _pos);
//         _pos += n;
//  
//         return static_cast<ssize_t>(n);
//     }
//     
//     /**
//      */
//     ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
//     {
//         if (!(_oflags & write_only)) {
//             ec = make_error_code(errc::invalid_argument);
//             return -1;
//         }
//         
//         if (n > _c->max_size() - _pos) {
//             ec = make_error_code(errc::device_too_large);
//             return -1;
//         }
//     
//         _c->append(bytes, n);
//         return static_cast<ssize_t>(n);
//     }
};

device make_file (std::string const & path, open_mode_flags oflags)
{
    return device{new file{path, oflags}};
}

device make_file (std::string const & path
        , open_mode_flags oflags
        , permissions perms)
{
    return device{new file{path, oflags, perms}};
}

}} // pfs::io

