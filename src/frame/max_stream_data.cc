#include "frame/max_stream_data.h"

quicpp::frame::max_stream_data::max_stream_data()
    : _stream_id(0)
    , _maximum_stream_data(0) {}

quicpp::frame::max_stream_data::max_stream_data(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_stream_id = quicpp::base::varint(in);
    this->_maximum_stream_data = quicpp::base::varint(in);
}

uint8_t quicpp::frame::max_stream_data::type() const {
    return quicpp::frame::frame_type_max_stream_data;
}

size_t quicpp::frame::max_stream_data::size() const {
    return 1 + this->_stream_id.size() + this->_maximum_stream_data.size();
}

void quicpp::frame::max_stream_data::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_stream_id.encode(out);
    this->_maximum_stream_data.encode(out);
}

quicpp::base::varint &quicpp::frame::max_stream_data::stream_id() {
    return this->_stream_id;
}

quicpp::base::varint &quicpp::frame::max_stream_data::maximum_stream_data() {
    return this->_maximum_stream_data;
}
