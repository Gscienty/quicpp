#include "frame/rst.h"

quicpp::frame::rst::rst()
    : _stream_id(0)
    , _application_error_code(0)
    , _final_offset(0) {}

quicpp::frame::rst::rst(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_stream_id = quicpp::base::stream_id_t(in);
    this->_application_error_code = quicpp::bigendian_decode<uint16_t>(in);
    this->_final_offset = quicpp::base::varint(in);
}

uint8_t quicpp::frame::rst::type() const {
    return quicpp::frame::frame_type_rst;
}

size_t quicpp::frame::rst::size() const {
    return 1 + this->_stream_id.size() + 2 + this->_final_offset.size();
}

void quicpp::frame::rst::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_stream_id.encode(out);
    quicpp::bigendian_encode(out, this->_application_error_code);
    this->_final_offset.encode(out);
}

quicpp::base::stream_id_t &quicpp::frame::rst::stream_id() {
    return this->_stream_id;
}

uint16_t &quicpp::frame::rst::application_error_code() {
    return this->_application_error_code;
}

quicpp::base::varint &quicpp::frame::rst::final_offset() {
    return this->_final_offset;
}
