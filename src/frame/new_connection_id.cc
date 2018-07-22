#include "frame/new_connection_id.h"

quicpp::frame::new_connection_id::new_connection_id(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);

    this->sequence = quicpp::base::varint(in);
    size_t conn_id_len = quicpp::bigendian_decode<uint8_t>(in);
    this->conn_id = quicpp::base::conn_id(in, conn_id_len);
    this->reset_token = quicpp::base::reset_token(in);
}

uint8_t quicpp::frame::new_connection_id::get_type() const {
    return quicpp::frame::frame_type_new_connection_id;
}

size_t quicpp::frame::new_connection_id::size() const {
    return 1 + this->sequence.size() + 1 + this->conn_id.size() + this->reset_token.size();
}

void quicpp::frame::new_connection_id::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->get_type());
    this->sequence.encode(out);
    quicpp::bigendian_encode(out, uint8_t(this->conn_id.size()));
    this->conn_id.encode(out);
    this->reset_token.encode(out);
}
