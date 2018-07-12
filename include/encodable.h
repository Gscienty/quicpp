#ifndef _QUICPP_ENCODABLE_
#define _QUICPP_ENCODABLE_

#include <ostream>
#include <stdint.h>

namespace quicpp {
    class encodable {
    public:
        virtual size_t size() const = 0;
        virtual void encode(std::basic_ostream<uint8_t> &out) const = 0;
    };
}

#endif

