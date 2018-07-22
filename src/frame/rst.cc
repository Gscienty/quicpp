#include "frame/rst.h"

quicpp::frame::rst::rst(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->stream_id = quicpp::base::varint(in);
    this->application_error_code = quicpp::bigendian_decode<uint16_t>(in);
    this->final_offset = quicpp::base::varint(in);
}

size_t quicpp::frame::rst::size() const {
    return this->stream_id.size() + 2 + this->final_offset.size();
}

void quicpp::frame::rst::encode(std::basic_ostream<uint8_t> &out) const {
    this->stream_id.encode(out);
    quicpp::bigendian_encode(out, this->application_error_code);
    this->final_offset.encode(out);
}
