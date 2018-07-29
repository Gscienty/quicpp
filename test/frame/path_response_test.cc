#include "frame/path_response.h"
#include "gtest/gtest.h"

TEST(path_response, encode) {
    quicpp::frame::path_response frame;
    frame.data() = 0x123456789ABCDEUL;
    
    uint8_t expect[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 9);
    out.put(0x0F);
    quicpp::bigendian_encode(out, 0x123456789ABCDEUL);

    uint8_t buffer[9];
    out.rdbuf()->pubsetbuf(buffer, 9);
    frame.encode(out);

    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(path_response, decode) {
    uint8_t buffer[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 9);
    out.put(0x0F);
    quicpp::bigendian_encode(out, 0x123456789ABCDEUL);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 9);

    quicpp::frame::path_response frame(in);
    EXPECT_EQ(0x123456789ABCDEUL, frame.data());
}

int main() {
    return RUN_ALL_TESTS();
}
