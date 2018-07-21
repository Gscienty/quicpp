#include "packet/header.h"

size_t quicpp::packet::header::size() const {
    if (this->header_form == quicpp::packet::header_form_long_header) {
        return 1 + 4 + 1 + this->dest_conn_id.size() + 
            this->src_conn_id.size() + this->payload_length.size() + this->packet_number_length;
    }
    return 1 + this->dest_conn_id.size() + this->packet_number_length;
}

void quicpp::packet::header::encode(std::basic_ostream<uint8_t> &out) const {
    if (this->header_form == quicpp::packet::header_form_long_header) {
        // header form | long packet type
        out.put(0x80 | this->packet_type);
        // version
        quicpp::bigendian_encode(out, this->version);
        // id length
        uint8_t id_length = 0x00;
        if (this->dest_conn_id.size() != 0) {
            id_length = (this->dest_conn_id.size() - 3) << 4;
        }
        if (this->src_conn_id.size() != 0) {
            id_length += this->src_conn_id.size() - 3;
        }
        out.put(id_length);
        // destination connection id
        this->dest_conn_id.encode(out);
        // source connection id
        this->src_conn_id.encode(out);
        // payload length
        this->payload_length.encode(out);
        // packet number
        quicpp::bigendian_encode(out, this->packet_number);
    }
    else {
        // header form | short packet type
        uint8_t first_bit = 0x00;
        if (this->key_phase) {
            first_bit |= 0x40;
        }
        first_bit |= 0x30;
        if (this->packet_number < 0x100) {
            // one byte
            first_bit |= 0x00;
        }
        else if (this->packet_number < 0x10000) {
            // two byte
            first_bit |= 0x01;
        }
        else {
            first_bit |= 0x02;
        }
        out.put(first_bit);
        // destination connection id
        this->dest_conn_id.encode(out);
        // packet number
        if (this->packet_number < 0x100) {
            quicpp::bigendian_encode(out, uint8_t(this->packet_number));
        }
        else if (this->packet_number < 0x10000) {
            quicpp::bigendian_encode(out, uint16_t(this->packet_number));
        }
        else {
            quicpp::bigendian_encode(out, this->packet_number);
        }
    }
}
