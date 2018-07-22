#include "packet/header.h"

quicpp::packet::header::header(std::basic_istream<uint8_t> &in, const size_t dest_conn_id_len) {
    uint8_t first_byte = in.get();
    if ((first_byte & 0x80) != 0x00) {
        // long packet type
        this->header_form = quicpp::packet::header_form_long_header;
        // packet type
        this->packet_type = 0x7F & first_byte;
        // version
        this->version = quicpp::bigendian_decode<uint32_t>(in);
        // id length
        uint8_t id_length = in.get();
        size_t dest_conn_id_len = ((id_length & 0xF0) >> 4) + 3;
        size_t src_conn_id_len = ((id_length & 0x0F)) + 3;
        // destination connection id
        this->dest_conn_id = quicpp::base::conn_id(in, dest_conn_id_len);
        // source connection id
        this->src_conn_id = quicpp::base::conn_id(in, src_conn_id_len);
        if (this->version == 0x00000000) {
            // version negotiation packet
            return;
        }
        // payload length
        this->payload_length = quicpp::base::varint(in);
        // packet number
        this->packet_number = quicpp::bigendian_decode<uint32_t>(in);
    }
    else {
        // short header
        this->header_form = quicpp::packet::header_form_short_header;
        // key phase bit
        this->key_phase = (first_byte & 0x40) != 0;
        // destinaction connection id
        this->dest_conn_id = quicpp::base::conn_id(in, dest_conn_id_len);
        switch ((this->packet_number & 0x03)) {
            case 0x00:
                this->packet_number = uint32_t(quicpp::bigendian_decode<uint8_t>(in));
                break;
            case 0x01:
                this->packet_number = uint32_t(quicpp::bigendian_decode<uint16_t>(in));
                break;
            case 0x02:
                this->packet_number = uint32_t(quicpp::bigendian_decode<uint32_t>(in));
                break;
            default:
                this->packet_number = 0;
        }
    }

}

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
        uint8_t first_byte = 0x00;
        if (this->key_phase) {
            first_byte |= 0x40;
        }
        first_byte |= 0x30;
        if (this->packet_number < 0x100) {
            // one byte
            first_byte |= 0x00;
        }
        else if (this->packet_number < 0x10000) {
            // two byte
            first_byte |= 0x01;
        }
        else {
            first_byte |= 0x02;
        }
        out.put(first_byte);
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
