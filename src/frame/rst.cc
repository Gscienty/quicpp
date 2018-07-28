#include "frame/rst.h"

quicpp::frame::rst::rst(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->stream_id = quicpp::base::varint(in);
    this->application_error_code = quicpp::bigendian_decode<uint16_t>(in);
    this->final_offset = quicpp::base::varint(in);
}

uint8_t quicpp::frame::rst::type() const {
    return quicpp::frame::frame_type_rst;
}

size_t quicpp::frame::rst::size() const {
    return 1 + this->stream_id.size() + 2 + this->final_offset.size();
}

void quicpp::frame::rst::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->stream_id.encode(out);
    quicpp::bigendian_encode(out, this->application_error_code);
    this->final_offset.encode(out);
}
