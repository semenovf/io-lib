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
#include "pfs/io/file.hpp"
#include "utils.hpp"
#include <cstring>
#include <iostream>

// TODO Make real unique filename
static std::string tmp_path ()
{
    return tmp_dir() + "/loremipsum.txt";
}

static std::string test_file_path;

TEST_CASE("File / basic") {
    pfs::io::error_code ec;
    auto d = pfs::io::make_file("!@#$%", pfs::io::read_only, ec);
    REQUIRE(d.is_null());
    REQUIRE_THROWS_AS(pfs::io::make_file("!@#$%", pfs::io::read_only), pfs::io::exception);
}

TEST_CASE("File / write") {
    std::string source{loremipsum};
    std::error_code ec;
    test_file_path = tmp_path();
    auto d = pfs::io::make_file(test_file_path, pfs::io::write_only, ec);

    if (ec)
        std::cerr << "ERROR: " << test_file_path << ": " << ec.message() << "\n";

    REQUIRE(!ec);
    REQUIRE(!d.is_null());
    REQUIRE(d.opened());
    REQUIRE(!d.is_readable());
    REQUIRE(d.is_writable());

    // Attempt to read from writable only device
    REQUIRE(d.read(nullptr, 0, ec) < 0);
    REQUIRE(ec == pfs::io::make_error_code(pfs::io::errc::bad_file_descriptor));

    ec = std::error_code{};

    ssize_t n = 0;
    size_t chunk_size = 32;

    while (!source.empty()) {
        n = d.write(source.data()
                , std::min(chunk_size, source.size())
                , ec);

        if (n <= 0)
            break;

        source.erase(0, n);
    }

    CHECK(n >= 0);
    REQUIRE(!ec);
}

TEST_CASE("File / read") {
    std::error_code ec;
    test_file_path = tmp_path();
    auto d = pfs::io::make_file(test_file_path, pfs::io::read_only, ec);

    if (ec)
        std::cerr << "ERROR: " << test_file_path << ": " << ec.message() << "\n";

    REQUIRE(!ec);
    REQUIRE(!d.is_null());
    REQUIRE(d.opened());
    REQUIRE(d.is_readable());
    REQUIRE(!d.is_writable());

    char buf[32];
    ssize_t n = 0;

    ec = std::error_code{};

    // Attempt to write to readable only device
    REQUIRE(d.write(buf, sizeof(buf), ec) < 0);
    REQUIRE(ec == pfs::io::make_error_code(pfs::io::errc::bad_file_descriptor));

    ec = std::error_code{};
    std::string result;

    while ((n = d.read(buf, 32, ec)) > 0) {
        result.append(buf, n);
    }

    REQUIRE(!ec);
    CHECK(result == loremipsum);
}
