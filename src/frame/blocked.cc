#include "frame/blocked.h"

quicpp::frame::blocked::blocked(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->offset = quicpp::base::varint(in);
}

uint8_t quicpp::frame::blocked::type() const {
    return quicpp::frame::frame_type_blocked;
}

size_t quicpp::frame::blocked::size() const {
    return 1 + this->offset.size();
}

void quicpp::frame::blocked::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->offset.encode(out);
}
