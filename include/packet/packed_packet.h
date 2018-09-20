#ifndef _QUICPP_PACKET_PACKED_PACKET_
#define _QUICPP_PACKET_PACKED_PACKET_

#include "packet/header.h"
#include "frame/type.h"
#include "crypt/encryption_level.h"
#include "ackhandler/packet.h"
#include <string>
#include <cstdint>
#include <vector>
#include <memory>

namespace quicpp {
namespace packet {

class packed_packet {
private:
    std::shared_ptr<quicpp::packet::header> header;
    std::basic_string<uint8_t> raw;
    std::vector<quicpp::frame::frame *> frames;
    uint8_t encryption_level;
public:
    std::shared_ptr<quicpp::ackhandler::packet> to_ack_handler_packet();
};

}
}

#endif
