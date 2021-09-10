////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2021 Vladislav Trifochkin
//
// License: see LICENSE file
//
// This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
//
// Changelog:
//      2019.??.?? Initial version
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/io/udp_server.hpp"
#include "pfs/io/udp_socket.hpp"
#include "utils.hpp"
#include <cstring>
#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

static const std::string servername = "localhost";
static uint16_t const port = 41972;

TEST_CASE("UDP socket / basic") {
    pfs::io::error_code ec;
    auto d = pfs::io::make_udp_socket("!@#$%", 0, false, ec);
    REQUIRE(d.is_null());
    CHECK(ec == pfs::io::make_error_code(pfs::io::errc::host_not_found));
    REQUIRE_THROWS_AS(pfs::io::make_udp_socket("!@#$%", 0, false), pfs::io::exception);
}

TEST_CASE("UDP socket / server") {
    int MAX_CLIENTS = 5;
    std::mutex mutex;
    std::condition_variable cond_var;

    std::thread watchdog_thread([& cond_var, & mutex] {
        std::unique_lock<std::mutex> lk(mutex);

        if (cond_var.wait_for(lk, std::chrono::seconds{5}) == std::cv_status::timeout) {
            std::cerr << "ERROR: force quit expired test\n";
            std::terminate();
        }
    });

    std::thread server_thread([& cond_var] {
        bool nonblocking = false;
        pfs::io::error_code ec;
        auto s = pfs::io::make_udp_server(servername, port, nonblocking, ec);
        char buf[32];
        int good_connections = 0;

        CHECK_FALSE(ec);

        if (ec) {
            std::cerr << "ERROR: create UDP server: " << ec.message() << "\n";
            return;
        }

        // Wait for starting clients writing
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        while (s.has_pending_data()) {
            ssize_t n = 0;

            n = s.read(buf, sizeof(buf), ec);

            if (n < 0) {
                std::cerr << "ERROR: read: " << ec.message() << "\n";
            } else {
                buf[n] = '\0';
                std::cout << "Data read (count=" << n << "): " << buf << "\n";
                good_connections++;
            }
        }

        cond_var.notify_all();
    });

    auto client = [] (std::string const & name) {
        bool nonblocking = false;
        pfs::io::error_code ec;
        auto d = pfs::io::make_udp_socket(servername, port, nonblocking, ec);

        CHECK_FALSE(d.is_null());
        CHECK_FALSE(ec);

        if (ec) {
            std::cerr << "ERROR: create UDP socket: " << ec.message() << "\n";
            return;
        }

        auto hello = std::string{"Hello, "} + name + '!';
        auto n = d.write(hello.c_str(), hello.size(), ec);

        CHECK(n >= 0);

        if (n < 0) {
            std::cerr << "ERROR: write UDP socket: " << ec.message() << "\n";
        } else {
            std::cout << "Bytes written: " << n << "\n";
        }
    };

    // Wait for starting server
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (int i = 0; i < MAX_CLIENTS; i++) {
        std::thread client_threads(client, "Client " + std::to_string(i));
        client_threads.join();
    }

    server_thread.join();
    watchdog_thread.join();
}
