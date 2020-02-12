////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2020.02.12 Initial version
//
// References:
//      1. man netdevice
//      2. [Getting interface MTU under Linux with PCAP](https://serverfault.com/questions/361503/getting-interface-mtu-under-linux-with-pcap)
//      3. [ifconfig.c](https://github.com/openbsd/src/blob/master/sbin/ifconfig/ifconfig.c)
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <memory>
#include <fstream>

#if defined(__linux) || defined(__linux__)
#   define PFS_IO_NETDEVICE_IMPL_LINUX 1
#   include <unistd.h>
#   include <sys/ioctl.h>
#   include <net/if.h>
    using impl_type = struct ifreq;
#endif

namespace pfs {
namespace io {

class netdevice
{
    std::string _name; // device name
    std::unique_ptr<impl_type> _d;

public:
    netdevice (std::string const & name);
    ~netdevice () = default;

    netdevice (netdevice const &) = delete;
    netdevice (netdevice &&) = delete;
    netdevice & operator = (netdevice const &) = delete;
    netdevice & operator = (netdevice &&) = delete;

    /**
     * Return MTU value for net device, or -1 if error occurred
     */
    int mtu () const;

private:
#if PFS_IO_NETDEVICE_IMPL_LINUX
    int mtu_alternative0 () const;
    int mtu_alternative1 () const;
#endif // PFS_IO_NETDEVICE_IMPL_LINUX
};

inline netdevice::netdevice (std::string const & name)
    : _name(name)
{
#if PFS_IO_NETDEVICE_IMPL_LINUX
#endif // PFS_IO_NETDEVICE_IMPL_LINUX
}

#if PFS_IO_NETDEVICE_IMPL_LINUX
int netdevice::mtu_alternative0 () const
{
    // TODO Implement using ioctl (man netdevice)
    return -1;
}

int netdevice::mtu_alternative1 () const
{
    std::string path {"/sys/class/net/"};

    // Check if specified directory exists
    if (::access(path.c_str(), F_OK) != 0)
        return -1;

    path += _name;
    path += "/mtu";

    // Unable read access for file
    if (::access(path.c_str(), R_OK) != 0)
        return -1;

    std::ifstream ifs(path);

    if (!ifs.is_open())
        return -1;

    int result = -1;
    ifs >> result;

    return result;
}
#endif // PFS_IO_NETDEVICE_IMPL_LINUX

inline int netdevice::mtu () const
{
    int result = -1;

#if PFS_IO_NETDEVICE_IMPL_LINUX
    result = mtu_alternative0();

    if (result < 0)
        result = mtu_alternative1();

#endif // PFS_IO_NETDEVICE_IMPL_LINUX

    return result;
}

}} // namespace pfs::io
