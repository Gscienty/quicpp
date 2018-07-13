#ifndef _QUICPP_BASE_VARINT_
#define _QUICPP_BASE_VARINT_

#include "encodable.h"
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

    class varint : public quicpp::encodable {
    private:
        uint64_t value;
    public:
        // decode constructor
        varint(std::basic_istream<uint8_t> &buf);
        // constructor
        varint(uint64_t &&);
        // encoded variable length int's size
        virtual size_t size() const override;
        static size_t size(const uint64_t value);
        // encode
        virtual void encode(std::basic_ostream<uint8_t> &out) const override;
        // get value
        uint64_t &get_value();
        // get value
        operator uint64_t() const;
    };
}
}

#endif

