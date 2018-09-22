#ifndef _QUICPP_PACKET_UNPACKET_PACKET_
#define _QUICPP_PACKET_UNPACKET_PACKET_

#include "frame/type.h"
#include <vector>
#include <memory>

namespace quicpp {
namespace packet {

class unpacked_packet {
private:
    uint8_t _encryption_level;
    std::vector<std::shared_ptr<quicpp::frame::frame>> _frames;
public:
    unpacked_packet() : _encryption_level(0) {}

    uint8_t &encryption_level() { return this->_encryption_level; }
    std::vector<std::shared_ptr<quicpp::frame::frame>> &frames() {
        return this->_frames;
    }
};

}
}

#endif
