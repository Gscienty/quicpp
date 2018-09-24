#include "frame/parser.h"
#include "frame/rst.h"
#include "frame/connection_close.h"
#include "frame/max_data.h"
#include "frame/max_stream_data.h"
#include "frame/max_stream_id.h"
#include "frame/blocked.h"
#include "frame/stream_blocked.h"
#include "frame/stop_sending.h"
#include "frame/ack.h"
#include "gtest/gtest.h"
#include <string>
#include <sstream>
#include <cstdint>
#include <memory>

TEST(parser, nil) {
    std::basic_string<uint8_t> buf;
    std::basic_stringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_FALSE(bool(f));
}

TEST(parser, skips_padding_frames) {
    std::basic_string<uint8_t> buf;
    buf.push_back(0);
    std::basic_stringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_FALSE(bool(f));
}

TEST(parser, handle_padding_frames) {
    std::basic_string<uint8_t> buf;
    buf.push_back(0);
    buf.push_back(0);
    buf.push_back(0);
    std::basic_stringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    
    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_FALSE(bool(f));
}

TEST(parser, unpacks_rst) {
    quicpp::frame::rst frame;
    frame.stream_id() = 0xdeadbeef;
    frame.final_offset() = 0xdecafbad1234;
    frame.application_error_code() = 0x1337;

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);

    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_rst, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::rst>(f), frame);
}

TEST(parser, unpacks_connection_close) {
    quicpp::frame::connection_close frame;
    std::string foo = "foo";
    frame.reason_phrase().assign(foo.begin(), foo.end());

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_connection_close, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::connection_close>(f),
              frame);
}

TEST(parser, unpacks_max_data) {
    quicpp::frame::max_data frame;

    frame.maximum_data() = 0xcafe;

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_max_data, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::max_data>(f),
              frame);
}

TEST(parser, unpacks_max_stream_data) {
    quicpp::frame::max_stream_data frame;

    frame.stream_id() = 0xdeadbeef;
    frame.maximum_stream_data() = 0xdecafbad;

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_max_stream_data, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::max_stream_data>(f),
              frame);
}

TEST(parser, unpacks_max_stream_id) {
    quicpp::frame::max_stream_id frame;

    frame.maximum_stream_id() = 0x1337;

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_max_stream_id, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::max_stream_id>(f),
              frame);
}

TEST(parser, unpacks_block) {
    quicpp::frame::blocked frame;

    frame.offset() = 0x1234;

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_blocked, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::blocked>(f),
              frame);
}

TEST(parser, unpacks_stream_block) {
    quicpp::frame::stream_blocked frame;

    frame.stream_id() = 0xdeadbeef;
    frame.offset() = 0x1234;

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_stream_blocked, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::stream_blocked>(f),
              frame);
}

TEST(parser, unpacks_stop_sending) {
    quicpp::frame::stop_sending frame;

    frame.stream_id() = 0xdeadbeef;
    frame.application_error_code() = 0x1234;

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_stop_sending, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::stop_sending>(f),
              frame);
}

TEST(parser, unpacks_ack) {
    quicpp::frame::ack frame;

    frame.delay() = std::chrono::microseconds(10);
    frame.ranges().push_back(std::make_pair(1, 0x13));

    std::basic_string<uint8_t> buf;
    buf.resize(frame.size());
    std::basic_ostringstream<uint8_t> out_stream;
    out_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());
    frame.encode(out_stream);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_TRUE(bool(f));
    EXPECT_EQ(quicpp::frame::frame_type_ack, f->type());
    EXPECT_EQ(*std::dynamic_pointer_cast<quicpp::frame::ack>(f),
              frame);
}

TEST(parser, invalid) {
    std::basic_string<uint8_t> buf;
    buf.push_back(0x42);

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(buf.data()), buf.size());

    auto f = quicpp::frame::parse_next_frame(in_stream);
    EXPECT_FALSE(bool(f));
}

int main() {
    return RUN_ALL_TESTS();
}
