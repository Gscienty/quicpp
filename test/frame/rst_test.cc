#include "frame/rst.h"
#include "gtest/gtest.h"

TEST(rst, encode) {
    quicpp::frame::rst frame;
    frame.stream_id() = 0x1337;
    frame.application_error_code() = 0x1234;
    frame.final_offset() = 0x123456;

    EXPECT_EQ(9, frame.size());

    uint8_t expect[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 9);
    out.put(0x01);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::bigendian_encode(out, static_cast<uint16_t>(0x1234));
    quicpp::base::varint(0x123456).encode(out);

    uint8_t buffer[9];
    out.rdbuf()->pubsetbuf(buffer, 9);
    frame.encode(out);

    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(rst, decode) {
    uint8_t buffer[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 9);
    out.put(0x01);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::bigendian_encode(out, static_cast<uint16_t>(0x1234));
    quicpp::base::varint(0x123456).encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 9);

    quicpp::frame::rst frame(in);

    EXPECT_EQ(0x1337, static_cast<uint16_t>(frame.stream_id()));
    EXPECT_EQ(0x1234, static_cast<uint16_t>(frame.application_error_code()));
    EXPECT_EQ(0x123456, static_cast<uint32_t>(frame.final_offset()));
}

int main() {
    return RUN_ALL_TESTS();
}
