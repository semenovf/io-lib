#include "pfs/io/buffer.hpp"
#include <cstring>
#include <iostream>
#include "../catch.hpp"

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

TEST_CASE("Buffer / basic") {
    std::string sample{loremipsum, std::strlen(loremipsum)};
    auto d = pfs::io::make_buffer(sample, pfs::io::read_only);

    REQUIRE(d.type() == pfs::io::device_type::buffer);
    REQUIRE(d.opened());
    CHECK(d.is_readable());
    CHECK_FALSE(d.is_writable());
    
    d.close();
    CHECK_FALSE(d.opened());
    CHECK(d.open_mode() == pfs::io::not_open);
}
    
TEST_CASE("Buffer / read") {
    std::string sample{loremipsum, std::strlen(loremipsum)};
    auto d = pfs::io::make_buffer(sample, pfs::io::read_only);

    REQUIRE(d.is_readable());
    
    std::string result;
    std::error_code ec;
    char buf[32];
    ssize_t n = 0;
   
    // Attempt to write to readable only device
    REQUIRE(d.write(buf, sizeof(buf), ec) < 0);
    REQUIRE(ec == pfs::io::make_error_code(pfs::io::errc::invalid_argument));
    
    ec = std::error_code{};
    
    while ((n = d.read(buf, 32, ec)) > 0) {
        result.append(buf, n);
    }

    REQUIRE(ec == std::error_code{});
    CHECK(result == sample);
}

TEST_CASE("Buffer / write") {
    std::string source{loremipsum};
    std::string result;
    auto d = pfs::io::make_buffer(result, pfs::io::write_only);
    
    REQUIRE(d.is_writable());
    
    ssize_t n = 0;
    size_t chunk_size = 32;
    std::error_code ec;

    // Attempt to read from writable only device
    REQUIRE(d.read(nullptr, 0, ec) < 0);
    REQUIRE(ec == pfs::io::make_error_code(pfs::io::errc::invalid_argument));
    
    ec = std::error_code{};
    
    while (!source.empty()) {
        n = d.write(source.data()
                , std::min(chunk_size, source.size())
                , ec);
        
        if (n <= 0)
            break;
        
        source.erase(0, n);
    }
    
    CHECK(n >= 0);
    REQUIRE(ec == std::error_code{});
    CHECK(result == loremipsum);
}
