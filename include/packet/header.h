#ifndef _QUICPP_PACKET_HEADER_
#define _QUICPP_PACKET_HEADER_

#include "base/conn_id.h"
#include "base/varint.h"
#include "encodable.h"
#include <stdint.h>
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
        bool header_form;
        bool key_phase;
        uint8_t packet_type;
        uint32_t version;
        quicpp::base::conn_id dest_conn_id;
        quicpp::base::conn_id src_conn_id;
        quicpp::base::varint payload_length;
        size_t packet_number_length;
        uint32_t packet_number;

    public:
        virtual size_t size() const override;
        virtual void encode(std::basic_ostream<uint8_t> &in) const override;
    };

}
}

#endif

