#include "frame/max_stream_data.h"

quicpp::frame::max_stream_data::max_stream_data(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->stream_id = quicpp::base::varint(in);
    this->maximum_stream_data = quicpp::base::varint(in);
}

uint8_t quicpp::frame::max_stream_data::type() const {
    return quicpp::frame::frame_type_max_stream_data;
}

size_t quicpp::frame::max_stream_data::size() const {
    return 1 + this->stream_id.size() + this->maximum_stream_data.size();
}

void quicpp::frame::max_stream_data::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->stream_id.encode(out);
    this->maximum_stream_data.encode(out);
}
