#ifndef _QUICPP_FRAME_TYPE_
#define _QUICPP_FRAME_TYPE_

#include <stdint.h>
#include "encodable.h"

namespace quicpp {
namespace frame {
const uint8_t frame_type_padding            = 0x00;
const uint8_t frame_type_rst                = 0x01;
const uint8_t frame_type_connection_close   = 0x02;
const uint8_t frame_type_application_close  = 0x03;
const uint8_t frame_type_max_data           = 0x04;
const uint8_t frame_type_max_stream_data    = 0x05;
const uint8_t frame_type_max_stream_id      = 0x06;
const uint8_t frame_type_ping               = 0x07;
const uint8_t frame_type_blocked            = 0x08;
const uint8_t frame_type_stream_blocked     = 0x09;
const uint8_t frame_type_stream_id_blockd   = 0x0A;
const uint8_t frame_type_new_connection_id  = 0x0B;
const uint8_t frame_type_stop_sending       = 0x0C;
const uint8_t frame_type_ack                = 0x0D;
const uint8_t frame_type_path_challenge     = 0x0E;
const uint8_t frame_type_path_response      = 0x0F;

class frame : public encodable {
public:
    virtual uint8_t type() const = 0;
};
}
}

#endif

