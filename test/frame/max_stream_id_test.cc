#include "frame/max_stream_id.h"
#include "gtest/gtest.h"

TEST(max_stream_id, encode) {
    quicpp::frame::max_stream_id frame;
    frame.maximum_stream_id() = 0x1337;
    EXPECT_EQ(3, frame.size());

    uint8_t expect[3];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 3);
    out.put(quicpp::frame::frame_type_max_stream_id);
    quicpp::base::varint(0x1337).encode(out);

    uint8_t buffer[3];
    out.rdbuf()->pubsetbuf(buffer, 3);
    frame.encode(out);

    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(max_stream_id, decode) {
    uint8_t buffer[3];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 3);
    out.put(quicpp::frame::frame_type_max_stream_id);
    quicpp::base::varint(0x1337).encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 3);

    quicpp::frame::max_stream_id frame(in);

    EXPECT_EQ(0x1337, frame.maximum_stream_id());
}

int main() {
    return RUN_ALL_TESTS();
}
