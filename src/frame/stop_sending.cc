#include "frame/stop_sending.h"

quicpp::frame::stop_sending::stop_sending()
    : _stream_id(0)
    , _application_error_code(0) {}

quicpp::frame::stop_sending::stop_sending(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);

    this->_stream_id = quicpp::base::stream_id_t(in);
    this->_application_error_code = quicpp::bigendian_decode<uint16_t>(in);
}

uint8_t quicpp::frame::stop_sending::type() const {
    return quicpp::frame::frame_type_stop_sending;
}

size_t quicpp::frame::stop_sending::size() const {
    return 1 + this->_stream_id.size() + 2;
}

void quicpp::frame::stop_sending::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_stream_id.encode(out);
    quicpp::bigendian_encode(out, this->_application_error_code);
}

quicpp::base::stream_id_t &quicpp::frame::stop_sending::stream_id() {
    return this->_stream_id;
}

uint16_t &quicpp::frame::stop_sending::application_error_code() {
    return this->_application_error_code;
}
