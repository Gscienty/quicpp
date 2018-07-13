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

    template <typename T>
    static void bigendian_encode(std::basic_ostream<uint8_t> &out, const T value) {
        size_t size = sizeof(T);

        for (size_t off = 0; off < size; off++) {
            out.put(reinterpret_cast<const uint8_t *>(&value)[size - 1 - off]);
        }
    }

    template <typename T>
    static void littleendian_encode(std::basic_ostream<uint8_t> &out, const T value) {
        size_t size = sizeof(T);

        for (size_t off = 0; off < size; off++) {
            out.put(reinterpret_cast<const uint8_t *>(&value)[off]);
        }
    }
}

#endif

