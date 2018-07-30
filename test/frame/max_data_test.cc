#include "frame/max_data.h"
#include "gtest/gtest.h"

TEST(max_data, encode) {
    quicpp::frame::max_data frame;
    frame.maximum_data() = 0x123456;

    EXPECT_EQ(5, frame.size());
    uint8_t expect[5];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 5);
    out.put(quicpp::frame::frame_type_max_data);
    quicpp::base::varint(0x123456).encode(out);

    uint8_t buffer[5];
    out.rdbuf()->pubsetbuf(buffer, 5);
    frame.encode(out);

    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(max_data, decode) {
    uint8_t buffer[5];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 5);
    out.put(quicpp::frame::frame_type_max_data);
    quicpp::base::varint(0x123456).encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 5);
    quicpp::frame::max_data frame(in);
    EXPECT_EQ(0x123456, frame.maximum_data());
}

int main() {
    return RUN_ALL_TESTS();
}
