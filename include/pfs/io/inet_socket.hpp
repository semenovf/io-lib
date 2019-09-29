#pragma once
#include "device.hpp"

namespace pfs { 
namespace io {

enum class socket_option
{
//  so_debug      = 0x00000000 // SO_DEBUG	1
//, so_reuse_addr = 0x00000001 // SO_REUSEADDR	2
//, so_type       = 0x00000002 // SO_TYPE		3
// 0x00000004 //#define SO_ERROR	4
// 0x00000008 //#define SO_DONTROUTE	5
// 0x00000010 //#define SO_BROADCAST	6
// 0x00000020 //#define SO_SNDBUF	7
// 0x00000040 //#define SO_RCVBUF	8
// 0x00000080 //#define SO_SNDBUFFORCE	32
// 0x00000100 //#define SO_RCVBUFFORCE	33
    keep_alive = 0x0200 // SO_KEEPALIVE	9
//#define SO_OOBINLINE	10
//#define SO_NO_CHECK	11
//#define SO_PRIORITY	12
//#define SO_LINGER	13
//#define SO_BSDCOMPAT	14
//#define SO_REUSEPORT	15
//#ifndef SO_PASSCRED /* powerpc only differs in these */
//#define SO_PASSCRED	16
//#define SO_PEERCRED	17
//#define SO_RCVLOWAT	18
//#define SO_SNDLOWAT	19
//#define SO_RCVTIMEO	20
//#define SO_SNDTIMEO	21
};

struct inet_socket_open_params
{
//     net::inet4_addr addr;
//     uint16_t port;
//     open_mode_flags oflags;
//     uint32_t socketopts;
//     device inet_socket_open_params (char const * addr
//             , uint16_t port
//             , open_mode_flags flags
//             , uint32_t sockopts)
//         : addr(a)
//         , port(p)
//         , oflags(of)
//         , socketopts(sso)
//     {}
};

// template <>
// struct open_params<tcp_socket> : public open_params<inet_socket>
// {
//     typedef open_params<inet_socket> base_class;
// 
//     open_params ()
//         : base_class(net::inet4_addr(), 0, 0, 0)
//     {}
// 
//     open_params (net::inet4_addr a, uint16_t p, open_mode_flags of, uint32_t sso)
//         : base_class(a, p, of, sso)
//     {}
// 
//     open_params (net::inet4_addr a, uint16_t p, open_mode_flags of)
//         : base_class(a, p, of, 0)
//     {}
// 
//     open_params (net::inet4_addr a, uint16_t p)
//         : base_class(a, p, read_write | non_blocking, 0)
//     {}
// };
// 
// template <>
// struct open_params<udp_socket> : public open_params<inet_socket>
// {
//     typedef open_params<inet_socket> base_class;
// 
//     open_params ()
//         : base_class(net::inet4_addr(), 0, 0, 0)
//     {}
// 
//     open_params (net::inet4_addr a, uint16_t p, open_mode_flags of, uint32_t sso)
//         : base_class(a, p, of, sso)
//     {}
// 
//     open_params (net::inet4_addr a, uint16_t p, open_mode_flags of)
//         : base_class(a, p, of, 0)
//     {}
// 
//     open_params (net::inet4_addr a, uint16_t p)
//         : base_class(a, p, read_write | non_blocking, 0)
//     {}
// };
// 
// template <>
// device_ptr open_device<tcp_socket> (open_params<tcp_socket> const & op, error_code & ec);
// 
// template <>
// device_ptr open_device<udp_socket> (open_params<udp_socket> const & op, error_code & ec);

device open_inet_socket (char const * addr
        , uint16_t port
        , open_mode_flags flags
        , uint32_t sockopts);

}} // pfs::io
