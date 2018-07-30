#include "frame/ack.h"
#include "gtest/gtest.h"
#include "base/varint.h"

TEST(ack, encode_simple_frame) {
    quicpp::frame::ack frame;
    frame.ranges().push_back(std::make_pair(100, 1337));

    std::basic_string<uint8_t> expect;
    expect.resize(frame.size());

    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(const_cast<uint8_t *>(expect.data()), expect.size());

    out.put(0x0D);
    quicpp::base::varint(1337).encode(out);
    out.put(0);
    quicpp::base::varint(0).encode(out);
    quicpp::base::varint(1337 - 100).encode(out);

    std::basic_string<uint8_t> buffer;
    buffer.resize(frame.size());
    out.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buffer.data()), buffer.size());
    frame.encode(out);

    for (size_t i = 0; i < frame.size(); i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(ack, encode_simple_packet) {
    quicpp::frame::ack frame;
    frame.ranges().push_back(std::make_pair(0xdeadbeef, 0xdeadbeef));
    frame.delay() = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(18));

    std::basic_string<uint8_t> expect;
    expect.resize(frame.size());

    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(const_cast<uint8_t *>(expect.data()), expect.size());
    frame.encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(const_cast<uint8_t *>(expect.data()), expect.size());

    quicpp::frame::ack parse_frame(in);

    EXPECT_FALSE(parse_frame.has_missing_ranges());
    EXPECT_EQ(frame.delay(), parse_frame.delay());
    EXPECT_EQ(0, in.rdbuf()->in_avail());
}

TEST(ack, encode_with_a_single_gap) {
    quicpp::frame::ack frame;
    frame.ranges().push_back(std::make_pair(400, 1000));
    frame.ranges().push_back(std::make_pair(100, 200));

    std::basic_string<uint8_t> buffer;
    buffer.resize(frame.size());

    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buffer.data()), buffer.size());

    frame.encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buffer.data()), buffer.size());

    quicpp::frame::ack p_frame(in);

    EXPECT_TRUE(p_frame.has_missing_ranges());
    EXPECT_EQ(0, in.rdbuf()->in_avail());
}

TEST(ack, encode_multiple_ranges) {
    quicpp::frame::ack frame;
    frame.ranges().push_back(std::make_pair(10, 10));
    frame.ranges().push_back(std::make_pair(8, 8));
    frame.ranges().push_back(std::make_pair(5, 6));
    frame.ranges().push_back(std::make_pair(1, 3));

    std::basic_string<uint8_t> buffer;
    buffer.resize(frame.size());

    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buffer.data()), buffer.size());

    frame.encode(out);

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buffer.data()), buffer.size());

    quicpp::frame::ack p_frame(in);

    EXPECT_TRUE(p_frame.has_missing_ranges());
    EXPECT_EQ(0, in.rdbuf()->in_avail());
}

TEST(ack, check_ranges) {
    quicpp::frame::ack frame;
    frame.ranges().push_back(std::make_pair(15, 20));
    frame.ranges().push_back(std::make_pair(5, 8));
}

int main() {
    return RUN_ALL_TESTS();
}
