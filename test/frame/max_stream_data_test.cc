#include "frame/max_stream_data.h"
#include "gtest/gtest.h"

TEST(max_stream_data, encode) {
    quicpp::frame::max_stream_data frame;
    frame.stream_id() = 0x1337;
    frame.maximum_stream_data() = 0x123456;

    EXPECT_EQ(7, frame.size());
    uint8_t expect[7];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 7);
    out.put(quicpp::frame::frame_type_max_stream_data);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0x123456).encode(out);

    uint8_t buffer[7];
    out.rdbuf()->pubsetbuf(buffer, 7);
    frame.encode(out);
    for (int i = 0; i < 7; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(max_stream_data, decode) {
    uint8_t buffer[7];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 7);
    out.put(quicpp::frame::frame_type_max_stream_data);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0x123456).encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 7);

    quicpp::frame::max_stream_data frame(in);
    
    EXPECT_EQ(0x1337, frame.stream_id());
    EXPECT_EQ(0x123456, frame.maximum_stream_data());
}

int main() {
    return RUN_ALL_TESTS();
}
