#include "base/reset_token.h"
#include "gtest/gtest.h"
#include <sstream>
#include <string>

TEST(reset_token, encode) {
    std::basic_string<uint8_t> expect;
    for (uint8_t byte = 0x00; byte < 0x10; byte++) {
        expect.push_back(byte);
    }

    quicpp::base::reset_token token(expect.begin(), expect.end());
    
    EXPECT_EQ(16, token.size());

    std::basic_ostringstream<uint8_t> out;
    uint8_t buffer[16];
    out.rdbuf()->pubsetbuf(buffer, 16);

    token.encode(out);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(reset_token, decode) {
    std::basic_string<uint8_t> expect;
    for (uint8_t byte = 0x00; byte < 0x10; byte++) {
        expect.push_back(byte);
    }

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(const_cast<uint8_t *>(expect.data()), 16);
    quicpp::base::reset_token token(in);
    
    EXPECT_EQ(16, token.size());

    std::basic_ostringstream<uint8_t> out;
    uint8_t buffer[16];
    out.rdbuf()->pubsetbuf(buffer, 16);

    token.encode(out);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

int main() {
    return RUN_ALL_TESTS();
}
