#include "frame/stream.h"
#include "gtest/gtest.h"
#include <string>

TEST(stream, encode) {
    quicpp::frame::stream frame;
    frame.stream_id() = 0x1337;
    std::string foo("foobar");
    frame.data().assign(foo.begin(), foo.end());

    EXPECT_EQ(9, frame.size());

    uint8_t expect[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 9);
    out.put(0x10);
    quicpp::base::varint(0x1337).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    uint8_t buffer[9];
    out.rdbuf()->pubsetbuf(buffer, 9);
    frame.encode(out);

    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(stream, encode_offset) {
    quicpp::frame::stream frame;
    frame.stream_id() = 0x1337;
    frame.offset() = 0x123456;
    frame.offset_flag() = true;
    std::string foo("foobar");
    frame.data().assign(foo.begin(), foo.end());

    EXPECT_EQ(13, frame.size());

    uint8_t expect[13];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 13);
    out.put(0x10 | 0x04);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0x123456).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    uint8_t buffer[13];
    out.rdbuf()->pubsetbuf(buffer, 13);
    frame.encode(out);

    for (int i = 0; i < 13; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(string, encode_len) {
    quicpp::frame::stream frame;
    frame.stream_id() = 0x1337;
    frame.len() = 0x123456;
    frame.len_flag() = true;
    std::string foo("foobar");
    frame.data().assign(foo.begin(), foo.end());

    EXPECT_EQ(13, frame.size());

    uint8_t expect[13];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 13);
    out.put(0x10 | 0x02);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0x123456).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    uint8_t buffer[13];
    out.rdbuf()->pubsetbuf(buffer, 13);
    frame.encode(out);

    for (int i = 0; i < 13; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(string, encode_fin) {
    quicpp::frame::stream frame;
    frame.stream_id() = 0x1337;
    frame.final_flag() = true;
    std::string foo("foobar");
    frame.data().assign(foo.begin(), foo.end());

    EXPECT_EQ(9, frame.size());

    uint8_t expect[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 9);
    out.put(0x10 | 0x01);
    quicpp::base::varint(0x1337).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    uint8_t buffer[9];
    out.rdbuf()->pubsetbuf(buffer, 9);
    frame.encode(out);

    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(stream, encode_offset_and_len) {
    quicpp::frame::stream frame;
    frame.stream_id() = 0x1337;
    frame.offset() = 0x123456;
    frame.offset_flag() = true;
    frame.len() = 6;
    frame.len_flag() = true;
    std::string foo("foobar");
    frame.data().assign(foo.begin(), foo.end());

    EXPECT_EQ(14, frame.size());

    uint8_t expect[14];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 14);
    out.put(0x10 | 0x04 | 0x02);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0x123456).encode(out);
    quicpp::base::varint(6).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());


    uint8_t buffer[14];
    out.rdbuf()->pubsetbuf(buffer, 14);
    frame.encode(out);

    for (int i = 0; i < 14; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(stream, decode) {
    uint8_t expect[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 9);
    out.put(0x10);
    quicpp::base::varint(0x1337).encode(out);
    std::string foo("foobar");
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());
    
    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(expect, 9);
    quicpp::frame::stream frame(in);

    EXPECT_EQ(0x1337, static_cast<uint16_t>(frame.stream_id()));
    for (int i = 0; i < 6; i++) {
        EXPECT_EQ(foo[i], frame.data()[i]);
    }
}

TEST(stream, decode_offset) {
    uint8_t buffer[9];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 9);
    out.put(0x10 | 0x01);
    quicpp::base::varint(0x1337).encode(out);
    std::string foo("foobar");
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 9);
    quicpp::frame::stream frame(in);

    EXPECT_EQ(0x1337, static_cast<uint16_t>(frame.stream_id()));

    for (int i = 0; i < 6; i++) {
        EXPECT_EQ(foo[i], frame.data()[i]);
    }
}

TEST(stream, decode_len) {
    uint8_t buffer[13];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 13);
    out.put(0x10 | 0x02);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(6).encode(out);
    std::string foo("foobar");
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());
    
    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 13);
    quicpp::frame::stream frame(in);

    EXPECT_EQ(0x1337, static_cast<uint16_t>(frame.stream_id()));
    EXPECT_EQ(6, static_cast<uint32_t>(frame.len()));
}

TEST(stream, decode_offset_and_len) {
    uint8_t buffer[14];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 14);
    out.put(0x10 | 0x04 | 0x02);
    quicpp::base::varint(0x1337).encode(out);
    quicpp::base::varint(0x123456).encode(out);
    quicpp::base::varint(6).encode(out);
    std::string foo("foobar");
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 14);
    quicpp::frame::stream frame(in);

    EXPECT_EQ(0x1337, static_cast<uint16_t>(frame.stream_id()));
    EXPECT_EQ(0x123456, static_cast<uint32_t>(frame.offset()));
    EXPECT_EQ(6, static_cast<uint8_t>(frame.len()));

    for (int i = 0; i < 6; i++) {
        EXPECT_EQ(foo[i], frame.data()[i]);
    }
}

int main() {
    return RUN_ALL_TESTS();
}
