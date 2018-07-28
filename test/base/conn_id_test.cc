#include "base/conn_id.h"
#include "gtest/gtest.h"
#include <sstream>

TEST(conn_id, encode) {
    uint8_t expect[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

    std::basic_string<uint8_t> id(expect, expect + 8);
    quicpp::base::conn_id conn_id(id);

    uint8_t buffer[8];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 8);

    conn_id.encode(out);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(conn_id, size) {
    uint8_t expect1[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    std::basic_string<uint8_t> id1(expect1, expect1 + 8);
    quicpp::base::conn_id conn_id1(id1);
    EXPECT_EQ(8, conn_id1.size());

    uint8_t expect2[] = { 0x00, 0x01, 0x02, 0x03 };
    std::basic_string<uint8_t> id2(expect2, expect2 + 4);
    quicpp::base::conn_id conn_id2(id2);
    EXPECT_EQ(4, conn_id2.size());
}

TEST(conn_id, decode) {
    uint8_t expect[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(expect, 8);
    quicpp::base::conn_id conn_id(in, 8);

    uint8_t buffer[8];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 8);

    conn_id.encode(out);

    for (int i = 0; i < 8; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

int main() {
    return RUN_ALL_TESTS();
}
