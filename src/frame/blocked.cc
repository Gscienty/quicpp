#include "frame/blocked.h"

quicpp::frame::blocked::blocked()
    : _offset(0) {}

quicpp::frame::blocked::blocked(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_offset = quicpp::base::varint(in);
}

uint8_t quicpp::frame::blocked::type() const {
    return quicpp::frame::frame_type_blocked;
}

size_t quicpp::frame::blocked::size() const {
    return 1 + this->_offset.size();
}

void quicpp::frame::blocked::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_offset.encode(out);
}

quicpp::base::varint &quicpp::frame::blocked::offset() {
    return this->_offset;
}

bool quicpp::frame::blocked::operator==(const quicpp::frame::blocked &frame) const {
    return this->_offset == frame._offset;
}
