#include "frame/new_connection_id.h"

quicpp::frame::new_connection_id::new_connection_id()
    : _sequence(0) {}

quicpp::frame::new_connection_id::new_connection_id(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);

    this->_sequence = quicpp::base::varint(in);
    size_t conn_id_len = quicpp::bigendian_decode<uint8_t>(in);
    this->_conn_id = quicpp::base::conn_id(in, conn_id_len);
    this->_reset_token = quicpp::base::reset_token(in);
}

uint8_t quicpp::frame::new_connection_id::type() const {
    return quicpp::frame::frame_type_new_connection_id;
}

size_t quicpp::frame::new_connection_id::size() const {
    return 1 + this->_sequence.size() + 1 + this->_conn_id.size() + this->_reset_token.size();
}

void quicpp::frame::new_connection_id::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_sequence.encode(out);
    quicpp::bigendian_encode(out, uint8_t(this->_conn_id.size()));
    this->_conn_id.encode(out);
    this->_reset_token.encode(out);
}

quicpp::base::varint &quicpp::frame::new_connection_id::sequence() {
    return this->_sequence;
}

quicpp::base::conn_id &quicpp::frame::new_connection_id::conn_id() {
    return this->_conn_id;
}

quicpp::base::reset_token &quicpp::frame::new_connection_id::reset_token() {
    return this->_reset_token;
}
