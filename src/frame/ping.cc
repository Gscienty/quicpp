#include "frame/ping.h"

quicpp::frame::ping::ping() {}

quicpp::frame::ping::ping(std::basic_istream<uint8_t> &in) {
    in.ignore();
}

size_t quicpp::frame::ping::size() const {
    return 1;
}

uint8_t quicpp::frame::ping::type() const {
    return quicpp::frame::frame_type_ping;
}

void quicpp::frame::ping::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
}


