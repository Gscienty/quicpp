#include "frame/max_stream_id.h"

quicpp::frame::max_stream_id::max_stream_id()
    : _maximum_stream_id(0) {}

quicpp::frame::max_stream_id::max_stream_id(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_maximum_stream_id = quicpp::base::varint(in);
}

uint8_t quicpp::frame::max_stream_id::type() const {
    return quicpp::frame::frame_type_max_stream_id;
}

size_t quicpp::frame::max_stream_id::size() const {
    return 1 + this->_maximum_stream_id.size();
}

void quicpp::frame::max_stream_id::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_maximum_stream_id.encode(out);
}

quicpp::base::varint &quicpp::frame::max_stream_id::maximum_stream_id() {
    return this->_maximum_stream_id;
}
