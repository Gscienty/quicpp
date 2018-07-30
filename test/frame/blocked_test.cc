#include "frame/blocked.h"
#include "gtest/gtest.h"

TEST(blocked, encode) {
    quicpp::frame::blocked frame;
    frame.offset() = 0x1337;

    EXPECT_EQ(3, frame.size());
    uint8_t expect[3];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 3);
    out.put(quicpp::frame::frame_type_blocked);
    quicpp::base::varint(0x1337).encode(out);

    uint8_t buffer[3];
    out.rdbuf()->pubsetbuf(buffer, 3);

    frame.encode(out);

    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(blocked, decode) {
    uint8_t buffer[3];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 3);
    out.put(quicpp::frame::frame_type_blocked);
    quicpp::base::varint(0x1337).encode(out);
    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 3);

    quicpp::frame::blocked frame(in);
    
    EXPECT_EQ(0x1337, frame.offset());
}

int main() {
    return RUN_ALL_TESTS();
}
