#include "frame/stream_blocked.h"

quicpp::frame::stream_blocked::stream_blocked()
    : _stream_id(0)
    , _offset(0) {}

quicpp::frame::stream_blocked::stream_blocked(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_stream_id = quicpp::base::stream_id_t(in);
    this->_offset = quicpp::base::varint(in);
}

uint8_t quicpp::frame::stream_blocked::type() const {
    return quicpp::frame::frame_type_stream_blocked;
}

size_t quicpp::frame::stream_blocked::size() const {
    return 1 + this->_stream_id.size() + this->_offset.size();
}

void quicpp::frame::stream_blocked::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_stream_id.encode(out);
    this->_offset.encode(out);
}

quicpp::base::stream_id_t &quicpp::frame::stream_blocked::stream_id() {
    return this->_stream_id;
}

quicpp::base::varint &quicpp::frame::stream_blocked::offset() {
    return this->_offset;
}
