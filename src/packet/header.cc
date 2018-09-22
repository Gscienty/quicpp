#include "packet/header.h"

quicpp::packet::header::header()
    : _header_form(false)
    , _key_phase(false)
    , _type(0)
    , _version(0)
    , _packet_number_length(0)
    , _packet_number(0) {}

quicpp::packet::header::header(std::basic_istream<uint8_t> &in, const size_t dest_conn_id_len) {
    uint8_t first_byte = in.get();
    if ((first_byte & 0x80) != 0x00) {
        // long packet type
        this->_header_form = quicpp::packet::header_form_long_header;
        // packet type
        this->_type = 0x7F & first_byte;
        // version
        this->_version = quicpp::bigendian_decode<uint32_t>(in);
        // id length
        uint8_t id_length = in.get();
        size_t dest_conn_id_len = ((id_length & 0xF0) >> 4) + 3;
        size_t src_conn_id_len = ((id_length & 0x0F)) + 3;
        // destination connection id
        this->_dest_conn_id = quicpp::base::conn_id(in, dest_conn_id_len);
        // source connection id
        this->_src_conn_id = quicpp::base::conn_id(in, src_conn_id_len);
        if (this->_version == 0x00000000) {
            // version negotiation packet
            return;
        }
        // payload length
        this->_payload_length = quicpp::base::varint(in);
        // packet number
        this->_packet_number = quicpp::bigendian_decode<uint32_t>(in);
    }
    else {
        // short header
        this->_header_form = quicpp::packet::header_form_short_header;
        // key phase bit
        this->_key_phase = (first_byte & 0x40) != 0;
        // destinaction connection id
        this->_dest_conn_id = quicpp::base::conn_id(in, dest_conn_id_len);
        switch ((this->_packet_number & 0x03)) {
            case 0x00:
                this->_packet_number = uint32_t(quicpp::bigendian_decode<uint8_t>(in));
                break;
            case 0x01:
                this->_packet_number = uint32_t(quicpp::bigendian_decode<uint16_t>(in));
                break;
            case 0x02:
                this->_packet_number = uint32_t(quicpp::bigendian_decode<uint32_t>(in));
                break;
            default:
                this->_packet_number = 0;
        }
    }

}

size_t quicpp::packet::header::size() const {
    if (this->_header_form == quicpp::packet::header_form_long_header) {
        size_t result = 1 + 4 + 1 + this->_dest_conn_id.size() + this->_src_conn_id.size();
        if (this->_version != 0x00000000) {
            // version negotiation
            result += this->_payload_length.size() + this->_packet_number_length;
        }
        return result;
    }
    return 1 + this->_dest_conn_id.size() + this->_packet_number_length;
}

void quicpp::packet::header::encode(std::basic_ostream<uint8_t> &out) const {
    if (this->_header_form == quicpp::packet::header_form_long_header) {
        // header form | long packet type
        out.put(0x80 | this->_type);
        // version
        quicpp::bigendian_encode(out, this->_version);
        // id length
        uint8_t id_length = 0x00;
        if (this->_dest_conn_id.size() != 0) {
            id_length = (this->_dest_conn_id.size() - 3) << 4;
        }
        if (this->_src_conn_id.size() != 0) {
            id_length += this->_src_conn_id.size() - 3;
        }
        out.put(id_length);
        // destination connection id
        this->_dest_conn_id.encode(out);
        // source connection id
        this->_src_conn_id.encode(out);
        if (this->_version == 0x00000000) {
            // version negotiation
            return;
        }
        // payload length
        this->_payload_length.encode(out);
        // packet number
        quicpp::bigendian_encode(out, this->_packet_number);
    }
    else {
        // header form | short packet type
        uint8_t first_byte = 0x00;
        if (this->_key_phase) {
            first_byte |= 0x40;
        }
        first_byte |= 0x30;
        if (this->_packet_number < 0x100) {
            // one byte
            first_byte |= 0x00;
        }
        else if (this->_packet_number < 0x10000) {
            // two byte
            first_byte |= 0x01;
        }
        else {
            first_byte |= 0x02;
        }
        out.put(first_byte);
        // destination connection id
        this->_dest_conn_id.encode(out);
        // packet number
        if (this->_packet_number < 0x100) {
            quicpp::bigendian_encode(out, uint8_t(this->_packet_number));
        }
        else if (this->_packet_number < 0x10000) {
            quicpp::bigendian_encode(out, uint16_t(this->_packet_number));
        }
        else {
            quicpp::bigendian_encode(out, this->_packet_number);
        }
    }
}
