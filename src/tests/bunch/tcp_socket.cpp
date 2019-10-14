#include "pfs/io/tcp_server.hpp"
#include "pfs/io/tcp_socket.hpp"
#include "utils.hpp"
#include <cstring>
#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>
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
    int const MAX_CONNECTIONS = 5;
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
        auto s = pfs::io::make_tcp_server(servername, port, nonblocking, ec);
        char buf[32];
        int good_connections = 0;

        CHECK_FALSE(ec);

        if (ec) {
            std::cerr << "ERROR: create local server: " << ec.message() << "\n";
            return;
        }

        int n = MAX_CONNECTIONS;

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

        cond_var.notify_all();
        CHECK(good_connections == MAX_CONNECTIONS);
    });

    auto client = [] (std::string const & name) {
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

        auto hello = std::string{"Hello, "} + name + '!';
        auto n = d.write(hello.c_str(), hello.size(), ec);

        CHECK(n >= 0);

        if (n < 0) {
            std::cerr << "ERROR: write TCP socket: " << ec.message() << "\n";
        }
    };

    // Wait for starting server
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        std::thread client_threads(client, "Client " + std::to_string(i));
        client_threads.join();
    }

    server_thread.join();
    watchdog_thread.join();
}
