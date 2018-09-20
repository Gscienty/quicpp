#ifndef _QUICPP_PACKET_HEADER_
#define _QUICPP_PACKET_HEADER_

#include "base/conn_id.h"
#include "base/varint.h"
#include "encodable.h"
#include <cstdint>
#include <string>

namespace quicpp {
namespace packet {

const bool header_form_long_header = true;
const bool header_form_short_header = false;

const uint8_t header_type_initial = 0x7F;
const uint8_t header_type_retry = 0x7E;
const uint8_t header_type_handshake = 0x7D;
const uint8_t header_type_0RTT_protected = 0x7C;

class header : public quicpp::encodable {
private:
    bool _header_form;
    bool _key_phase;
    uint8_t _type;
    uint32_t _version;
    quicpp::base::conn_id _dest_conn_id;
    quicpp::base::conn_id _src_conn_id;
    quicpp::base::varint _payload_length;
    size_t _packet_number_length;
    uint32_t _packet_number;

public:
    header(std::basic_istream<uint8_t> &in, const size_t dest_conn_id_len = 0);
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    bool &header_form() { return this->_header_form; }
    bool &key_phase() { return this->_key_phase; }
    uint8_t &type() { return this->_type; }
    uint32_t &version() { return this->_version; }
    quicpp::base::conn_id &dest_connid() { return this->_dest_conn_id; }
    quicpp::base::conn_id &src_connid() { return this->_src_conn_id; }
    quicpp::base::varint &payload_length() { return this->_payload_length; }
    size_t &packet_number_length() { return this->_packet_number_length; }
    uint32_t &packet_number() { return this->_packet_number; } 
};

}
}

#endif

