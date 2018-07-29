#include "frame/path_challenge.h"

quicpp::frame::path_challenge::path_challenge()
    : _data(0) {}

quicpp::frame::path_challenge::path_challenge(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_data = quicpp::bigendian_decode<uint64_t>(in);
}

uint8_t quicpp::frame::path_challenge::type() const {
    return quicpp::frame::frame_type_path_challenge;
}

size_t quicpp::frame::path_challenge::size() const {
    return 1 + 8;
}

void quicpp::frame::path_challenge::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    quicpp::bigendian_encode(out, this->_data);
}

uint64_t &quicpp::frame::path_challenge::data() {
    return this->_data;
}
