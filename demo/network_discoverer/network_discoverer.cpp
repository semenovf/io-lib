////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2021 Vladislav Trifochkin
//
// This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
//
// Changelog:
//      2021.09.13 Initial version.
////////////////////////////////////////////////////////////////////////////////
#include "network_discoverer.hpp"
#include "pfs/memory.hpp"

namespace pfs {
namespace net {
namespace discovery {

// network_discoverer::network_discoverer (radiocast_timer & timer)
//     : _rcast_timer(& timer)
// {
//     timer.timed_out.connect(*this, & network_discoverer::radiocast);
// }
//
// network_discoverer::~network_discoverer ()
// {
//     if (_started)
//         stop();
// }
//
// auto network_discoverer::make_result (result_status::code_enum code
//     , std::string && s) -> network_discoverer::result_status
// {
//     result_status r;
//     r.code = code;
//     r.error = std::move(s);
//     return r;
// }
//
// void network_discoverer::set_options (options && opts)
// {
//     bool need_restart {false};
//
//     if (_started) {
//         stop();
//         _started = false;
//         need_restart = true;
//     }
//
//     _opts = std::move(opts);
//
//     if (need_restart)
//         start();
// }

}}} // namespace pfs::net::discovery
