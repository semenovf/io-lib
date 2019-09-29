#pragma once
#include "device.hpp"

namespace pfs { 
namespace io {

template <typename ContiguousContainer>
class buffer : public basic_device
{
    ContiguousContainer * _c = nullptr;
    open_mode_flags _oflags = not_open;
    size_t _pos = 0;

public:
    buffer () {}
    
    buffer (ContiguousContainer & container, open_mode_flags oflags) 
        : _c(& container)
        , _oflags(oflags)
    {}
    
    device_type type () const noexcept override
    {
        return device_type::buffer;
    }
    
    open_mode_flags open_mode () const noexcept override
    {
        return _oflags;
    }
    
    error_code close () override
    {
        _c = nullptr;
        _oflags = not_open;
        return std::error_code{};
    }
    
    bool opened () const noexcept override
    {
        return !(_c == nullptr);
    }
    
    /**
     * @return -1 on error.
     * @exception errc::invalid_argument if buffer is unsuitable for reading
     * (i.e. opened without read mode)
     */
    ssize_t read (char * bytes, size_t n, error_code & ec) noexcept override
    {
        if (!(_oflags & read_only)) {
            ec = make_error_code(errc::invalid_argument);
            return -1;
        }
        
        if (_pos >= _c->size())
            return 0;

        n = std::min(n, _c->size() - _pos);
        _c->copy(bytes, n, _pos);
        _pos += n;
 
        return static_cast<ssize_t>(n);
    }
    
    /**
     */
    ssize_t write (char const * bytes, size_t n, error_code & ec) noexcept override
    {
        if (!(_oflags & write_only)) {
            ec = make_error_code(errc::invalid_argument);
            return -1;
        }
        
        if (n > _c->max_size() - _pos) {
            ec = make_error_code(errc::device_too_large);
            return -1;
        }
    
        _c->append(bytes, n);
        return static_cast<ssize_t>(n);
    }
};

template <typename ContiguousContainer>
inline device make_buffer (ContiguousContainer & container, open_mode_flags oflags)
{
    return device(new buffer<ContiguousContainer>{container, oflags});
}
    
}} // pfs::io
