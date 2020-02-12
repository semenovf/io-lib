////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Vladislav Trifochkin
//
// This file is part of [pfs-io](https://github.com/semenovf/pfs-io) library.
//
// Changelog:
//      2020.02.12 Initial version
////////////////////////////////////////////////////////////////////////////////
#include "pfs/io/netdevice.hpp"
#include <iostream>
#include <cstdlib>

int main (int argc, char * argv[])
{
    if (argc < 2) {
        std::cerr << "Too few arguments. Specify network device name.\n"
                << "List of network devices names:\n"
                << "\tfor Linux: `cat /sys/class/net/`\n";
        return EXIT_FAILURE;
    }

    std::string devname(argv[1]);
    pfs::io::netdevice dev(devname);

    auto mtu = dev.mtu();

    if (mtu < 0) {
        std::cerr << "Getting MTU for `" << devname << "` failure\n";
    } else {
        std::cout << "Device " << devname << "\n"
                << "    mtu: " << mtu << "\n";
    }

    return EXIT_SUCCESS;
}
