////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2021 Vladislav Trifochkin
//
// This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
//
// Changelog:
//      2021.09.10 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "zmq.hpp" // https://github.com/zeromq/cppzmq
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

using zmq_socket_ptr = std::unique_ptr<zmq::socket_t>;

namespace {

inline void sleep_for_seconds (int secs)
{
    std::this_thread::sleep_for(std::chrono::seconds(secs));
}

using random_engine_t = std::mt19937;

inline auto engine () -> random_engine_t
{
    static std::random_device __rd; // Will be used to obtain a seed for the random number engine
    return random_engine_t{__rd()}; // Standard mersenne_twister_engine seeded with rd()
}

inline auto random_integer (int from, int to) -> int
{
    std::uniform_int_distribution<int> distrib{from, to + 1};
    auto rnd = engine();
    return distrib(rnd);
}

inline auto within (int n) -> int
{
    return random_integer(0, n);
}

zmq_socket_ptr make_client_socket (zmq::context_t & context)
{
    std::cout << "I: connecting to server..." << std::endl;
    zmq_socket_ptr client {new zmq::socket_t(context, ZMQ_REQ)};
    client->connect("tcp://localhost:5555");

    // Configure socket to not wait at close time
    int linger = 0;
    client->set(zmq::sockopt::linger, linger);

    return client;
}

//  Convert string to 0MQ string and send to socket
inline bool s_send (zmq::socket_t & socket
    , std::string const & string
    , zmq::send_flags flags = zmq::send_flags::none)
{
    zmq::message_t message(string.size());
    memcpy(message.data(), string.data(), string.size());

    zmq::send_result_t rc = socket.send(message, flags);
    return rc.has_value() ? true : false;
}

//  Receive 0MQ string from socket and convert into string
inline std::string s_recv (zmq::socket_t & socket
    , zmq::recv_flags flags = zmq::recv_flags::none)
{
    zmq::message_t message;
    socket.recv(message, flags);

    return std::string(static_cast<char*>(message.data()), message.size());
}

} // namespace
