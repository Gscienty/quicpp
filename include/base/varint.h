#ifndef _QUICPP_VARINT_
#define _QUICPP_VARINT_

#include <unistd.h>
#include <stdint.h>
#include <string>
#include <istream>

namespace quicpp {
namespace base {
    const size_t VARINT_LENGTH_1 = 1;
    const size_t VARINT_LENGTH_2 = 2;
    const size_t VARINT_LENGTH_4 = 4;
    const size_t VARINT_LENGTH_8 = 8;

    const uint64_t VARINT_MAX_1 = 63;
    const uint64_t VARINT_MAX_2 = 16383;
    const uint64_t VARINT_MAX_4 = 1073741823;
    const uint64_t VARINT_MAX_8 = 4611686018427387903;

    class varint {
    private:
        uint64_t value;
    public:
        varint(std::basic_istream<uint8_t> &buf);
        varint(uint64_t &&);
        size_t size() const;
        std::basic_string<uint8_t> encode() const;
        uint64_t &get_value();
        operator uint64_t() const;
    };
}
}

#endif

