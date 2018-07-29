#include "frame/stream_id_blocked.h"
#include "gtest/gtest.h"

TEST(stream_id_blocked, encode) {
    quicpp::frame::stream_id_blocked frame;
    frame.stream_id() = 0xDECAFBAD;

    EXPECT_EQ(9, frame.size());

    uint8_t expect_buffer[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect_buffer, 9);
    out.put(0x0A);
    quicpp::base::varint(0xDECAFBAD).encode(out);

    uint8_t buffer[9];
    out.rdbuf()->pubsetbuf(buffer, 9);
    frame.encode(out);

    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(expect_buffer[i], buffer[i]);
    }
}

TEST(stream_id_blocked, decode) {
    uint8_t serialized_buffer[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(serialized_buffer, 9);
    out.put(0x0A);
    quicpp::base::varint(0xDECAFBAD).encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(serialized_buffer, 9);
    quicpp::frame::stream_id_blocked frame(in);

    EXPECT_EQ(0xDECAFBAD, uint64_t(frame.stream_id()));
}

int main() {
    return RUN_ALL_TESTS();
}
