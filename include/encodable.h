#ifndef _QUICPP_ENCODABLE_
#define _QUICPP_ENCODABLE_

#include <ostream>
#include <istream>
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
static T bigendian_decode(std::basic_istream<uint8_t> &in) {
    size_t size = sizeof(T);

    T result = 0;
    for (size_t off = 0; off < size; off++) {
        reinterpret_cast<uint8_t *>(&result)[size - 1 - off] = in.get();
    }
    return result;
}

template <typename T>
static void littleendian_encode(std::basic_ostream<uint8_t> &out, const T value) {
    size_t size = sizeof(T);

    for (size_t off = 0; off < size; off++) {
        out.put(reinterpret_cast<const uint8_t *>(&value)[off]);
    }
}

template <typename T>
static T littleendian_decode(std::basic_istream<uint8_t> &in) {
    size_t size = sizeof(T);

    T result = 0;
    for (size_t off = 0; off < size; off++) {
        reinterpret_cast<uint8_t *>(&result)[off] = in.get();
    }
    return result;
}
}

#endif

