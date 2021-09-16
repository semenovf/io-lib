////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2021 Vladislav Trifochkin
//
// This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
//
// Changelog:
//      2021.09.13 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pfs/emitter.hpp"
#include <chrono>
#include <memory>
#include <string>

// #if QT5_NETWORK_ENABLED
// #   include <QUdpSocket>
// #else
// #   error "No network backend specified"
// #endif

namespace pfs {
namespace net {
namespace discovery {

class radiocast_timer
{
public:
    pfs::emitter<> timed_out;

public:
    virtual void start (std::chrono::milliseconds interval) = 0;
    virtual void stop () = 0;
};

enum class backend_enum {
    qt5
};

struct options
{
    // Address to bind listener ('*' is any address)
    std::string listener_addr4;
    std::uint16_t listener_port;
    std::string listener_interface;

    // Addresses "*" or "255.255.255.255" are broadcast.
    // Addresses starting from 224 through 239 are multicast.
    // Addresses in other range is unicast.
    std::string peer_addr4;
};

struct result_status
{
    enum code_enum {
          no_error
        , bad_network_interface
        , bad_address
        , bind_failure
        , join_milticast_failure
        , leave_milticast_failure
    } code;

    std::string error;

    inline operator bool () const noexcept
    {
        return code != no_error;
    }
};

class discoverer final
{
    class impl;
    std::unique_ptr<impl> _p;

public:
// private:
// #if QT5_NETWORK_ENABLED
//     using udp_socket_type = std::unique_ptr<QUdpSocket>;
// #endif

private:
//     bool _started {false};
//     options _opts;
//     udp_socket_type _listener;
//     udp_socket_type _radio;
//     radiocast_timer * _radiocast_timer {nullptr};

private:
//     result_status make_result (result_status::code_enum code, std::string && str);
//
//     inline result_status make_result ()
//     {
//         return make_result(result_status::no_error, "");
//     }
//
//     enum class muticast_group_op { join, leave };
//     result_status process_multicast_group (muticast_group_op op);
//
//     void process_incoming_datagrams ();
//     void radiocast ();
//
//     static bool is_broadcast_address (std::string const & addr);
//     static bool is_multicast_address (std::string const & addr);

private:
    discoverer (/*radiocast_timer & timer*/);

public:
    ~discoverer ();

    discoverer (discoverer const &) = delete;
    discoverer (discoverer &&) = default;
    discoverer & operator = (discoverer const &) = delete;
    discoverer & operator = (discoverer &&) = default;

//     void set_options (options && opts);
//
//     result_status start ();
//     result_status stop ();
//
//     bool started () const noexcept
//     {
//         return _started;
//     }
//
//     pfs::emitter<std::string const &> incoming_data_received;

    template <backend_enum Backend>
    friend discoverer make_discoverer (/*radiocast_timer & timer*/);
};

template <backend_enum Backend>
discoverer make_discoverer (/*radiocast_timer & timer*/);

}}} // namespace pfs::net::discovery
