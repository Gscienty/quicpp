#include "frame/stream_id_blocked.h"

quicpp::frame::stream_id_blocked::stream_id_blocked(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->stream_id = quicpp::base::varint(in);
}

uint8_t quicpp::frame::stream_id_blocked::type() const {
    return quicpp::frame::frame_type_stream_id_blockd;
}

size_t quicpp::frame::stream_id_blocked::size() const {
    return 1 + this->stream_id.size();
}

void quicpp::frame::stream_id_blocked::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->stream_id.encode(out);
}
