#include "frame/application_close.h"
#include "base/varint.h"
#include "gtest/gtest.h"

TEST(application_close, encode) {
    quicpp::frame::application_close frame;
    frame.error() = 0x1337;
    std::string foo = "foobar";
    frame.reason_phrase().assign(foo.begin(), foo.end());

    EXPECT_EQ(10, frame.size());

    uint8_t expect[10];
    std::basic_stringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(expect, 10);
    out.put(quicpp::frame::frame_type_application_close);
    quicpp::bigendian_encode(out, static_cast<uint16_t>(0x1337));
    quicpp::base::varint(foo.size()).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    uint8_t buffer[10];
    out.rdbuf()->pubsetbuf(buffer, 10);

    frame.encode(out);

    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(expect[i], buffer[i]);
    }
}

TEST(application_close, decode) {
    uint8_t buffer[10];
    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(buffer, 10);
    out.put(quicpp::frame::frame_type_application_close);
    std::string foo("foobar");
    quicpp::bigendian_encode(out, static_cast<uint16_t>(0x1337));
    quicpp::base::varint(foo.size()).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(foo.data()), foo.size());

    std::basic_istringstream<uint8_t> in;
    in.rdbuf()->pubsetbuf(buffer, 10);

    quicpp::frame::application_close frame(in);

    EXPECT_EQ(0x1337, frame.error());
    for (int i = 0; i < 6; i++) {
        EXPECT_EQ(foo[i], frame.reason_phrase()[i]);
    }
}

int main() {
    return RUN_ALL_TESTS();
}
