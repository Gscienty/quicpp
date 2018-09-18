#include "base/stream_id_t.h"

quicpp::base::stream_id_t::stream_id_t()
    : quicpp::base::varint() {}

quicpp::base::stream_id_t::stream_id_t(std::basic_istream<uint8_t> &buf)
    : quicpp::base::varint(buf) {}

quicpp::base::stream_id_t::stream_id_t(uint64_t &&val)
    : quicpp::base::varint(val) {}

quicpp::base::stream_id_t::stream_id_t(const uint64_t &val)
    : quicpp::base::varint(val) {}

quicpp::base::stream_id_t::stream_id_t(const quicpp::base::varint &val)
    : quicpp::base::varint(val) {}

bool quicpp::base::stream_id_t::bidirectional() const {
    return (*this & 0x02) == 0x00;
}

bool quicpp::base::stream_id_t::client_initiated() const {
    return (*this & 0x01) == 0x00;
}

quicpp::base::stream_id_t & quicpp::base::stream_id_t::operator+=(const uint64_t val) {
    this->value() += val;
    return *this;
}
