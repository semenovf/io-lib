#pragma once
#include "basic_file.hpp"
#include <cstring>
#include <sys/un.h>
#include <sys/socket.h>

namespace pfs { 
namespace io {

class local_socket : public basic_file
{
    sockaddr_un  _sockaddr;
    
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
    
    // Inherited from basic_file.
    // bool opened () const noexcept override
    
    /**
     * @return -1 on error.
     */
    ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
    {
//         ssize_t sz = ::read(_fd, bytes, n);
// 
//         if (sz < 0)
//             ec = get_last_system_error();
// 
//         return sz;
    }
    
    /**
     */
    ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
    {
//         ssize_t sz = ::write(_fd, bytes, n);
// 
//         if (sz < 0)
//             ec = get_last_system_error();
// 
//         return sz;
    }
    
    void swap (local_socket & rhs)
    {
        using std::swap;
        swap(_fd, rhs._fd);
        
        sockaddr_un tmp;
        std::memcpy(& tmp, & _sockaddr, sizeof(tmp));
        std::memcpy(& _sockaddr, & rhs._sockaddr, sizeof(tmp));
        std::memcpy(& rhs._sockaddr, & tmp, sizeof(tmp)); 
    }
};

/**
 * Makes file device.
 */
device make_local_socket (std::string const & name
        , open_mode_flags oflags
        , error_code & ec)
{
    local_socket sock;
    ec = sock.connect(name, oflags);
    return ec ? device{} : device{new local_socket(std::move(sock))};
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


