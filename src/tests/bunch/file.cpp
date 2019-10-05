#include "pfs/io/file.hpp"
#include <cstring>
#include <iostream>
#include "../catch.hpp"

#if defined(PFS_OS_WIN)
    // TODO Implement using GetTempPath()
#elif defined(PFS_OS_UNIX)
#   include <cstdlib>
    static std::string tmp_dir () {
        static char const * vars[] = {
            "TMPDIR", "TMP", "TEMP", "TEMPDIR"
        };
        
        int count = sizeof(vars) / sizeof(vars[0]);
        
        for (int i = 0; i < count; i++) {
            auto t = getenv(vars[i]);
            
            if (t)
                return std::string{t};
        }
        
        return std::string{"/tmp"};
    }
    
    // TODO Make real unique filename
    static std::string tmp_path () 
    {
        return tmp_dir() + "/loremipsum.txt";
    }
    
#else
#   error "Unsupported operation system"
#endif

static std::string test_file_path;

char loremipsum[] =
"1.Lorem ipsum dolor sit amet, consectetuer adipiscing elit,    \n\
2.sed diam nonummy nibh euismod tincidunt ut laoreet dolore     \n\
3.magna aliquam erat volutpat. Ut wisi enim ad minim veniam,    \n\
4.quis nostrud exerci tation ullamcorper suscipit lobortis      \n\
5.nisl ut aliquip ex ea commodo consequat. Duis autem vel eum   \n\
6.iriure dolor in hendrerit in vulputate velit esse molestie    \n\
7.consequat, vel illum dolore eu feugiat nulla facilisis at     \n\
8.vero eros et accumsan et iusto odio dignissim qui blandit     \n\
9.praesent luptatum zzril delenit augue duis dolore te feugait  \n\
10.nulla facilisi. Nam liber tempor cum soluta nobis eleifend    \n\
11.option congue nihil imperdiet doming id quod mazim placerat   \n\
12.facer possim assum. Typi non habent claritatem insitam; est   \n\
13.usus legentis in iis qui facit eorum claritatem.              \n\
14.Investigationes demonstraverunt lectores legere me lius quod  \n\
15.ii legunt saepius. Claritas est etiam processus dynamicus,    \n\
16.qui sequitur mutationem consuetudium lectorum. Mirum est      \n\
17.notare quam littera gothica, quam nunc putamus parum claram,  \n\
18.anteposuerit litterarum formas humanitatis per seacula quarta \n\
19.decima et quinta decima. Eodem modo typi, qui nunc nobis      \n\
20.videntur parum clari, fiant sollemnes in futurum.             \n\
21.Lorem ipsum dolor sit amet, consectetuer adipiscing elit,     \n\
22.sed diam nonummy nibh euismod tincidunt ut laoreet dolore     \n\
23.magna aliquam erat volutpat. \"Ut wisi enim ad minim veniam,  \n\
24.quis nostrud exerci tation ullamcorper suscipit lobortis      \n\
25.nisl ut aliquip ex ea commodo consequat. Duis autem vel eum   \n\
26.iriure dolor in hendrerit in vulputate velit esse molestie    \n\
27.consequat, vel illum dolore eu feugiat nulla facilisis at     \n\
28.vero eros et accumsan et iusto odio dignissim qui blandit     \n\
29.praesent luptatum zzril delenit augue duis dolore te feugait  \n\
30.nulla facilisi. Nam liber tempor cum soluta nobis eleifend    \n\
31.option congue nihil imperdiet doming id quod mazim placerat   \n\
32.facer possim assum. Typi non habent claritatem insitam; est   \n\
33.usus legentis in iis qui facit eorum claritatem.              \n\
34.Investigationes demonstraverunt lectores legere me lius quod  \n\
35.ii legunt saepius. Claritas est etiam processus dynamicus,    \n\
36.qui sequitur mutationem consuetudium lectorum. Mirum est      \n\
37.notare quam littera gothica, quam nunc putamus parum claram,  \n\
38.anteposuerit litterarum formas humanitatis per seacula quarta \n\
39.decima et quinta decima.\" Eodem modo typi, qui nunc nobis    \n\
40.videntur parum clari, fiant sollemnes in futurum.";

TEST_CASE("File / basic") {
    pfs::io::error_code ec;
    auto d = pfs::io::make_file("!@#$%", pfs::io::read_only, ec);
    REQUIRE(d.is_null());
}

TEST_CASE("File / write") {
    std::string source{loremipsum};
//     std::string result;
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
