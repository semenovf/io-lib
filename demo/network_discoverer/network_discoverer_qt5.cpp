////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2021 Vladislav Trifochkin
//
// This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
//
// Changelog:
//      2021.09.15 Initial version.
////////////////////////////////////////////////////////////////////////////////
#include "network_discoverer.hpp"
#include "pfs/memory.hpp"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QUdpSocket>

#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
#   include <QNetworkDatagram>
#endif

namespace pfs {
namespace net {
namespace discovery {

class network_discoverer_qt5
{
    using udp_socket_type = std::unique_ptr<QUdpSocket>;

};

#if __COMMENT__
bool network_discoverer::is_broadcast_address (std::string const & addr)
{
    if (addr == "*" || addr == "255.255.255.255")
        return true;

    QHostAddress host_addr{QString::fromStdString(addr)};
    return host_addr.isBroadcast();
}

bool network_discoverer::is_multicast_address (std::string const & addr)
{
    QHostAddress host_addr{QString::fromStdString(addr)};
    return host_addr.isMulticast();
}

network_discoverer::result_status
network_discoverer::process_multicast_group (muticast_group_op op)
{
    QHostAddress group_addr4 {QString::fromStdString(_opts.peer_addr4)};
    QNetworkInterface iface;

    if (!_opts.listener_interface.empty() && _opts.listener_interface != "*") {
        iface = QNetworkInterface::interfaceFromName(QString::fromStdString(
            _opts.listener_interface));

        if (!iface.isValid()) {
            return make_result(result_status::bad_network_interface
                , "bad listener interface specified");
        }
    }

    if (_listener->state() != QUdpSocket::BoundState) {
        return make_result(result_status::join_milticast_failure
            , "listener is not bound");
    }

    if (op == muticast_group_op::join) {
        auto joining_result = iface.isValid()
            ? _listener->joinMulticastGroup(group_addr4, iface)
            : _listener->joinMulticastGroup(group_addr4);

        if (!joining_result) {
            return make_result(result_status::join_milticast_failure
                , "joining listener to multicast group failure");
        }
    } else {
        auto leaving_result = iface.isValid()
            ? _listener->leaveMulticastGroup(group_addr4, iface)
            : _listener->leaveMulticastGroup(group_addr4);

        if (!leaving_result) {
            return make_result(result_status::leave_milticast_failure
                , "leaving listener from multicast group failure");
        }
    }

    return make_result();
}

network_discoverer::result_status network_discoverer::start ()
{
    if (!_started) {
        QHostAddress bind_addr4;

        if (_opts.listener_addr4 == "*")
            bind_addr4 = QHostAddress::AnyIPv4;
        else
            bind_addr4 = QHostAddress{QString::fromStdString(_opts.listener_addr4)};

        if (bind_addr4.isNull())
            return make_result(result_status::bad_address, "bad listener address");

        _listener = pfs::make_unique<QUdpSocket>();

        QUdpSocket::BindMode bind_mode = QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint;

        if (!_listener->bind(bind_addr4, _opts.listener_port, bind_mode))
            return make_result(result_status::bind_failure, "listener socket binding failure");

        if (is_broadcast_address(_opts.peer_addr4)) {
            // TODO Implement
        } else if (is_multicast_address(_opts.peer_addr4)) {
            auto joining_result = process_multicast_group(muticast_group_op::join);

            if (!joining_result)
                return joining_result;
        } else {
            // TODO Implement unicast
        }

        _listener->connect(& *_listener, & QUdpSocket::readyRead, [this] {
            this->process_incoming_datagrams();
        });
    }

    _started = true;
    return make_result();
}

network_discoverer::result_status network_discoverer::stop ()
{
    result_status result;

    if (_started) {
        if (is_multicast_address(_opts.peer_addr4)) {
            auto leaving_result = process_multicast_group(muticast_group_op::join);

            if (!leaving_result && !result)
                result = leaving_result;
        }

        _listener.reset();
        _started = false;
    }

    return result;
}

void network_discoverer::process_incoming_datagrams ()
{
    while (_listener->hasPendingDatagrams()) {

#if QT_VERSION < QT_VERSION_CHECK(5,8,0)
    // using QUdpSocket::readDatagram (API since Qt 4)
    QByteArray datagram;
    datagram.resize(static_cast<int>(_listener->pendingDatagramSize()));
    _listener->readDatagram(datagram.data(), datagram.size());
    this->incoming_data_received(std::string(datagram.data(), datagram.size()));
#else
    // using QUdpSocket::receiveDatagram (API since Qt 5.8)
    QNetworkDatagram datagram = _listener->receiveDatagram();
    QByteArray bytes = datagram.data();
    this->incoming_data_received(std::string(bytes.data(), bytes.size()));
#endif

    }
}

void network_discoverer::radiocast ()
{
    QByteArray datagram = "HELO";

//         if (is_broadcast_address(_opts.peer_addr4)) {
//             // TODO Implement
//         } else if (is_multicast_address(_opts.peer_addr4)) {
//             auto joining_result = process_multicast_group(muticast_group_op::join);
//
//             if (!joining_result)
//                 return joining_result;
//         } else {
//             // TODO Implement unicast
//         }
//
//
//     udpSocket4.writeDatagram(datagram, groupAddress4, 45454);
//     if (udpSocket6.state() == QAbstractSocket::BoundState)
//         udpSocket6.writeDatagram(datagram, groupAddress6, 45454);
}

#endif

}}} // namespace pfs::net::discovery
