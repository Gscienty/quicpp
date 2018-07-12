#include "base/varint.h"
#include <iostream>
#include "gtest/gtest.h"
#include <iostream>

TEST(varint, length) {
    quicpp::base::varint len1(37);
    quicpp::base::varint len2(15293);
    quicpp::base::varint len4(494878333);
    quicpp::base::varint len8(151288809941952652);

    EXPECT_EQ(quicpp::base::VARINT_LENGTH_1, len1.size());
    EXPECT_EQ(quicpp::base::VARINT_LENGTH_2, len2.size());
    EXPECT_EQ(quicpp::base::VARINT_LENGTH_4, len4.size());
    EXPECT_EQ(quicpp::base::VARINT_LENGTH_8, len8.size());
}

void __inl_expcet_encoded(uint8_t expect[], size_t size, uint8_t str[]) {
    for (size_t i = 0; i < size; i++) {
        EXPECT_EQ(expect[i], str[i]);
    }
}

TEST(varint, encode) {
    quicpp::base::varint len1(37);
    quicpp::base::varint len2(15293);
    quicpp::base::varint len4(494878333);
    quicpp::base::varint len8(151288809941952652);

    std::basic_stringstream<uint8_t> out;
    uint8_t out_buf[8];
    out.rdbuf()->pubsetbuf(out_buf, 8);

    len1.encode(out);
    uint8_t expect1[] = { 0x25 };
    __inl_expcet_encoded(expect1, sizeof(expect1), out_buf);

    out.rdbuf()->pubseekpos(0);
    len2.encode(out);
    uint8_t expect2[] = { 0x7B, 0xBD };
    __inl_expcet_encoded(expect2, sizeof(expect2), out_buf);

    out.rdbuf()->pubseekpos(0);
    len4.encode(out);
    uint8_t expect4[] = { 0x9d, 0x7f, 0x3e, 0x7d };
    __inl_expcet_encoded(expect4, sizeof(expect4), out_buf);

    out.rdbuf()->pubseekpos(0);
    len8.encode(out);
    uint8_t expect8[] = { 0xc2, 0x19, 0x7c, 0x5e, 0xff, 0x14, 0xe8, 0x8c };
    __inl_expcet_encoded(expect8, sizeof(expect8), out_buf);

}

TEST(varint, decode) {
    uint8_t expect1[] = { 0x25 };
    std::basic_string<uint8_t> encode1(expect1, expect1 + 1);
    std::basic_istringstream<uint8_t> in1(encode1);
    quicpp::base::varint int1(in1);
    EXPECT_EQ(37, uint64_t(int1));

    uint8_t expect2[] = { 0x7B, 0xBD };
    std::basic_string<uint8_t> encode2(expect2, expect2 + 2);
    std::basic_istringstream<uint8_t> in2(encode2);
    quicpp::base::varint int2(in2);
    EXPECT_EQ(15293, uint64_t(int2));
    
    uint8_t expect4[] = { 0x9d, 0x7f, 0x3e, 0x7d };
    std::basic_string<uint8_t> encode4(expect4, expect4 + 4);
    std::basic_istringstream<uint8_t> in4(encode4);
    quicpp::base::varint int4(in4);
    EXPECT_EQ(494878333, uint64_t(int4));
    
    uint8_t expect8[] = { 0xc2, 0x19, 0x7c, 0x5e, 0xff, 0x14, 0xe8, 0x8c };
    std::basic_string<uint8_t> encode8(expect8, expect8 + 8);
    std::basic_istringstream<uint8_t> in8(encode8);
    quicpp::base::varint int8(in8);
    EXPECT_EQ(151288809941952652, uint64_t(int8));
}

int main() {
    return RUN_ALL_TESTS();
}
