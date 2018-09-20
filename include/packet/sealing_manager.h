#ifndef _QUICPP_PACKET_SEALING_MANAGER_
#define _QUICPP_PACKET_SEALING_MANAGER_

#include "crypt/encryption_level.h"
#include "handshake/sealer.h"
#include "base/error.h"
#include <utility>

namespace quicpp {
namespace packet {

class sealing_manager {
public:
    virtual
    std::pair<uint8_t, quicpp::handshake::sealer &>
    get_sealer() = 0;
    virtual
    std::pair<uint8_t, quicpp::handshake::sealer &>
    get_sealer_for_crypto_stream() = 0;
    virtual
    std::pair<quicpp::handshake::sealer &, quicpp::base::error_t>
    get_sealer_with_encryption_level(uint8_t encryption_level) = 0;
};

}
}

#endif
