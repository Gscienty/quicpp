#include "frame/new_connection_id.h"
#include "gtest/gtest.h"

TEST(new_connection_id, encode) {
    quicpp::frame::new_connection_id frame;
    frame.sequence() = 0x1234;
    uint8_t id[4] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t token[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    frame.conn_id().id().assign(id, id + 4);
    frame.reset_token().token().assign(token, token + 16);

    EXPECT_EQ(24, frame.size());

    uint8_t expect[24];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 24);

    out.put(quicpp::frame::frame_type_new_connection_id);
    quicpp::base::varint(0x1234).encode(out);
    out.put(uint8_t(4));
    out.write(id, 4);
    out.write(token, 16);

    uint8_t buffer[24];
    out.rdbuf()->pubsetbuf(buffer, 24);

    frame.encode(out);

    for (int i = 0; i < 24; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(new_connection_id, decode) {
    uint8_t buffer[24];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 24);
    out.put(quicpp::frame::frame_type_new_connection_id);
    quicpp::base::varint(0x1234).encode(out);
    out.put(uint8_t(4));
    uint8_t id[4] = { 0x00, 0x01, 0x02, 0x03 };
    uint8_t token[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    out.write(id, 4);
    out.write(token, 16);


    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 24);

    quicpp::frame::new_connection_id frame(in);

    EXPECT_EQ(0x1234, static_cast<uint32_t>(frame.sequence()));

    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(id[i], frame.conn_id().id()[i]);
    }

    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(token[i], frame.reset_token().token()[i]);
    }
}

int main() {
    return RUN_ALL_TESTS();
}
