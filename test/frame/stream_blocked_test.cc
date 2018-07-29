#include "frame/stream_blocked.h"
#include "gtest/gtest.h"

TEST(stream_blocked, encode) {
    quicpp::frame::stream_blocked frame;
    frame.stream_id() = 0x1337;
    frame.offset() = 0xDEADBEEF;

    EXPECT_EQ(1 + 2 + 8, frame.size());

    uint8_t expect[1 + 2 + 8];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 1 + 2 + 8);
    out.put(0x09);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0xDEADBEEF).encode(out);

    uint8_t buffer[1 + 2 + 8];
    out.rdbuf()->pubsetbuf(buffer, 1 + 2 + 8);
    frame.encode(out);

    for (int i = 0; i < 1 + 2 + 8; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(stream_blocked, decode) {
    uint8_t expect[1 + 2 + 8];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 1 + 2 + 8);
    out.put(0x09);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0xDEADBEEF).encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(expect, 1 + 2 + 8);

    quicpp::frame::stream_blocked frame(in);

    EXPECT_EQ(0x1337, uint16_t(frame.stream_id()));
    EXPECT_EQ(0xDEADBEEF, uint32_t(frame.offset()));

}

int main() {
    return RUN_ALL_TESTS();
}
