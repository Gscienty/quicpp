#include "frame/stop_sending.h"

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
