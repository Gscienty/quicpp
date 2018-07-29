#include "frame/stop_sending.h"
#include "gtest/gtest.h"

TEST(stop_sending, encode) {
    quicpp::frame::stop_sending frame;
    frame.stream_id() = 0x1337;
    frame.application_error_code() = 0x1234;

    EXPECT_EQ(5, frame.size());

    uint8_t expect[5];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 5);
    out.put(0x0C);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::bigendian_encode(out, uint16_t(0x1234));
    uint8_t buffer[5];
    out.rdbuf()->pubsetbuf(buffer, 5);
    frame.encode(out);

    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(stop_sending, decode) {
    uint8_t expect[5];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 5);
    out.put(0x0C);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::bigendian_encode(out, uint16_t(0x1234));
    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(expect, 5);

    quicpp::frame::stop_sending frame(in);
    
    EXPECT_EQ(0x1337, static_cast<uint16_t>(frame.stream_id()));
    EXPECT_EQ(0x1234, static_cast<uint16_t>(frame.application_error_code()));

}

int main() {
    return RUN_ALL_TESTS();
}
