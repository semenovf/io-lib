#include "pfs/io/local_server.hpp"
#include "pfs/io/local_socket.hpp"
#include "utils.hpp"
#include <cstring>
#include <chrono>
#include <iostream>
#include <thread>
#include "../catch.hpp"

// TODO Make real unique filename
static std::string server_name ()
{
    return tmp_dir() + "/pfs_local_socket";
}

TEST_CASE("Local socket / basic") {
    pfs::io::error_code ec;
    auto d = pfs::io::make_local_socket("!@#$%", false, ec);
    REQUIRE(d.is_null());
    CHECK(ec == pfs::io::make_error_code(pfs::io::errc::resource_not_found));
    REQUIRE_THROWS_AS(pfs::io::make_local_socket("!@#$%", false), pfs::io::exception);
}

TEST_CASE("Local socket / server") {
    std::thread server_thread([] {
        bool non_blocking = false;
        pfs::io::error_code ec;
        auto s = pfs::io::make_local_server(server_name(), non_blocking, ec);
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
            auto peer = s.accept(ec);

            if (!peer.is_null()) {
                std::cout << "New connection accepted" << "\n";
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

        CHECK(good_connections == max_connections);
    });

    auto client = [] () {
        bool non_blocking = false;
        pfs::io::error_code ec;
        auto d = pfs::io::make_local_socket(server_name(), non_blocking, ec);
        CHECK_FALSE(d.is_null());
        CHECK_FALSE(ec);

        if (ec) {
            std::cerr << "ERROR: create local socket: " << ec.message() << "\n";
            return;
        }

        char const * hello = "Hello!";
        auto n = d.write(hello, std::strlen(hello), ec);

        CHECK(n >= 0);

        if (n < 0) {
            std::cerr << "ERROR: write local socket: " << ec.message() << "\n";
        }
    };

    // Wait for starting server
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread client1_threads(client);
    std::thread client2_threads(client);
    std::thread client3_threads(client);

    server_thread.join();

    client1_threads.join();
    client2_threads.join();
    client3_threads.join();
}
