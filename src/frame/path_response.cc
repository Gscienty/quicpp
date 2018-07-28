#include "frame/path_response.h"

quicpp::frame::path_response::path_response(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->data = quicpp::bigendian_decode<uint64_t>(in);
}

uint8_t quicpp::frame::path_response::type() const {
    return quicpp::frame::frame_type_path_response;
}

size_t quicpp::frame::path_response::size() const {
    return 1 + 8;
}

void quicpp::frame::path_response::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    quicpp::bigendian_encode(out, this->data);
}
