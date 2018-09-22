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
    std::shared_ptr<quicpp::packet::header> _header;
    std::basic_string<uint8_t> _raw;
    std::vector<std::shared_ptr<quicpp::frame::frame>> _frames;
    uint8_t _encryption_level;
public:
    packed_packet() : _encryption_level(0) {}
    std::shared_ptr<quicpp::packet::header> &header() { return this->_header; }
    std::basic_string<uint8_t> &raw() { return this->_raw; }
    std::vector<std::shared_ptr<quicpp::frame::frame>> &frames() { return this->_frames; }
    uint8_t &encryption_level() { return this->_encryption_level; }
    std::shared_ptr<quicpp::ackhandler::packet> to_ack_handler_packet();
};

}
}

#endif
