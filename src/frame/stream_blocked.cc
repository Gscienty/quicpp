#include "frame/stream_blocked.h"

quicpp::frame::stream_blocked::stream_blocked(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->stream_id = quicpp::base::varint(in);
    this->offset = quicpp::base::varint(in);
}

uint8_t quicpp::frame::stream_blocked::get_type() const {
    return quicpp::frame::frame_type_stream_blocked;
}

size_t quicpp::frame::stream_blocked::size() const {
    return 1 + this->stream_id.size() + this->offset.size();
}

void quicpp::frame::stream_blocked::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->get_type());
    this->stream_id.encode(out);
    this->offset.encode(out);
}
