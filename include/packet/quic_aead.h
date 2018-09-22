#ifndef _QUICPP_PACKET_QUIC_AEAD_
#define _QUICPP_PACKET_QUIC_AEAD_

#include "base/error.h"
#include <string>
#include <cstdint>
#include <utility>

namespace quicpp {
namespace packet {

class quic_aead {
public:
    virtual
    std::pair<std::basic_string<uint8_t>, quicpp::base::error_t>
    open_handshake(std::basic_string<uint8_t> &src,
                   uint64_t packet_number,
                   std::basic_string<uint8_t> &associated_data) = 0;

    virtual
    std::pair<std::basic_string<uint8_t>, quicpp::base::error_t>
    open_1rtt(std::basic_string<uint8_t> &src,
              uint64_t packet_number,
              std::basic_string<uint8_t> &associated_data) = 0;
};

}
}

#endif
