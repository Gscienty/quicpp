#include "base/varint.h"

quicpp::base::varint::varint()
    : value(0) { }

quicpp::base::varint::varint(uint64_t &&value)
    : value(value) { }

quicpp::base::varint::varint(std::basic_istream<uint8_t> &buf) {
    this->value = 0;
    uint8_t first_byte = buf.get();
    uint8_t type = first_byte >> 6;
    size_t size = 0;
    switch (type) {
        case 0x00:
            // 1 bytes
            this->value = first_byte & 0x3F;
            size = 1;
            break;
        case 0x01:
            // 2 bytes
            size = 2;
            reinterpret_cast<uint8_t *>(&this->value)[size - 1] = first_byte & 0x3F;
            break;
        case 0x02:
            // 4 bytes
            size = 4;
            reinterpret_cast<uint8_t *>(&this->value)[size - 1] = first_byte & 0x3F;
            break;
        case 0x03:
            // 8 bytes
            size = 8;
            reinterpret_cast<uint8_t *>(&this->value)[size - 1] = first_byte & 0x3F;
            break;
    }

    if (size == 0) { return ; }
    
    for (size_t i = 1; i < size; i++) {
        reinterpret_cast<uint8_t *>(&this->value)[size - 1 - i] = buf.get();
    }
}

size_t quicpp::base::varint::size(const uint64_t value) {
    if (value < quicpp::base::VARINT_MAX_1) {
        return quicpp::base::VARINT_LENGTH_1;
    }
    else if (value < quicpp::base::VARINT_MAX_2) {
        return quicpp::base::VARINT_LENGTH_2;
    }
    else if (value < quicpp::base::VARINT_MAX_4) {
        return quicpp::base::VARINT_LENGTH_4;
    }
    else if (value < quicpp::base::VARINT_MAX_8) {
        return quicpp::base::VARINT_LENGTH_8;
    }
    else {
        return 0;
    }
}

size_t quicpp::base::varint::size() const {
    return quicpp::base::varint::size(this->value);
}

void quicpp::base::varint::encode(std::basic_ostream<uint8_t> &out) const {
    size_t size = this->size();
    uint8_t mask = 0;
    // append prefix at first byte
    switch (size) {
        case quicpp::base::VARINT_LENGTH_1:
            mask = 0x00;
            break;
        case quicpp::base::VARINT_LENGTH_2:
            mask = 0x40;
            break;
        case quicpp::base::VARINT_LENGTH_4:
            mask = 0x80;
            break;
        case quicpp::base::VARINT_LENGTH_8:
            mask = 0xC0;
            break;
    }
    // host byte order: little endian
    // network byte order: big endian
    // transform host byte order into network byte order
    out.put((reinterpret_cast<const uint8_t *>(&this->value)[size - 1]) | mask);
    for (size_t i = 1; i < size; i++) {
        out.put(reinterpret_cast<const uint8_t *>(&this->value)[size - 1 - i]);
    }
}

uint64_t &quicpp::base::varint::get_value() {
    return this->value;
}

quicpp::base::varint::operator uint64_t() const {
    return this->value;
}
