#include "frame/max_stream_id.h"

quicpp::frame::max_stream_id::max_stream_id()
    : _maximum_stream_id(0) {}

quicpp::frame::max_stream_id::max_stream_id(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_maximum_stream_id = quicpp::base::stream_id_t(in);
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

quicpp::base::stream_id_t &quicpp::frame::max_stream_id::maximum_stream_id() {
    return this->_maximum_stream_id;
}

bool quicpp::frame::max_stream_id::
operator==(const quicpp::frame::max_stream_id &frame) const {
    return this->_maximum_stream_id == frame._maximum_stream_id;
}
