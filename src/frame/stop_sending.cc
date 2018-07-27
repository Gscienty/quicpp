#include "frame/stop_sending.h"

quicpp::frame::stop_sending::stop_sending(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);

    this->stream_id = quicpp::base::varint(in);
    this->application_error_code = quicpp::bigendian_decode<uint16_t>(in);
}

uint8_t quicpp::frame::stop_sending::get_type() const {
    return quicpp::frame::frame_type_stop_sending;
}

size_t quicpp::frame::stop_sending::size() const {
    return 1 + this->stream_id.size() + 2;
}

void quicpp::frame::stop_sending::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->get_type());
    this->stream_id.encode(out);
    quicpp::bigendian_encode(out, this->application_error_code);
}
