#include "pfs/io/tcp_server.hpp"
#include "pfs/io/tcp_socket.hpp"
#include "utils.hpp"
#include <cstring>
#include <chrono>
#include <iostream>
#include <thread>
#include "../catch.hpp"

static const std::string servername = "localhost";
static uint16_t const port = 41972;

TEST_CASE("TCP socket / basic") {
    pfs::io::error_code ec;
    auto d = pfs::io::make_tcp_socket("!@#$%", 0, false, ec);
    REQUIRE(d.is_null());
    CHECK(ec == pfs::io::make_error_code(pfs::io::errc::host_not_found));
    REQUIRE_THROWS_AS(pfs::io::make_tcp_socket("!@#$%", 0, false), pfs::io::exception);
}

TEST_CASE("TCP socket / server") {
    std::atomic_flag server_finished = ATOMIC_FLAG_INIT;

    std::thread watchdog_thread([& server_finished] {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        bool x = server_finished.test_and_set();

        if (!x) {
            std::cerr << "ERROR: force quit expired test\n";
            std::terminate();
        }
    });

    std::thread server_thread([& server_finished] {
        bool nonblocking = false;
        pfs::io::error_code ec;
        auto s = pfs::io::make_tcp_server(servername, port, nonblocking, ec);
        char buf[32];
        int const max_connections = 3;
        int good_connections = 0;

        CHECK_FALSE(ec);

        if (ec) {
            std::cerr << "ERROR: create local server: " << ec.message() << "\n";
            return;
        }

        int n = max_connections;

        while (n) {
            std::cout << "Waiting for connections ...\n";
            auto peer = s.accept(ec);

            if (!peer.is_null()) {
                std::cout << "New connection accepted\n";
                auto n = peer.read(buf, sizeof(buf), ec);

                if (n < 0) {
                    std::cerr << "ERROR: read peer: " << ec.message() << "\n";
                } else {
                    buf[n] = '\0';
                    std::cout << "Data read from peer: " << buf << "\n";
                    good_connections++;
                }
            } else {
                std::cerr << "ERROR: accept peer: " << ec.message() << "\n";
            }

            n--;
        }

        server_finished.test_and_set();
        CHECK(good_connections == max_connections);
    });

    auto client = [] () {
        bool nonblocking = false;
        pfs::io::error_code ec;
        auto d = pfs::io::make_tcp_socket(servername, port, nonblocking, ec);

        CHECK_FALSE(d.is_null());
        CHECK_FALSE(ec);

        if (ec) {
            std::cerr << "ERROR: create TCP socket: " << ec.message() << "\n";
            return;
        }

        pfs::io::underlying_device<pfs::io::tcp_socket>(d)->enable_keep_alive(true);

        char const * hello = "Hello!";
        auto n = d.write(hello, std::strlen(hello), ec);

        CHECK(n >= 0);

        if (n < 0) {
            std::cerr << "ERROR: write TCP socket: " << ec.message() << "\n";
        }
    };

    // Wait for starting server
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread client1_threads(client);
    std::thread client2_threads(client);
    std::thread client3_threads(client);

    server_thread.join();

    client1_threads.join();
    client2_threads.join();
    client3_threads.join();
    watchdog_thread.join();
}
