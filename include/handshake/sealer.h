#ifndef _QUICPP_HANDSHAKE_SEALER_
#define _QUICPP_HANDSHAKE_SEALER_

#include <cstdint>
#include <string>

namespace quicpp {
namespace handshake {

class sealer {
public:
    virtual
    std::basic_string<uint8_t>
    seal(std::basic_string<uint8_t> src,
         uint64_t packet_number,
         std::basic_string<uint8_t> associated_data) = 0;
    virtual size_t overhead() = 0;
};

}
}

#endif
