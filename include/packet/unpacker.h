#ifndef _QUICPP_PACKET_UNPACKER_
#define _QUICPP_PACKET_UNPACKER_

#include "packet/quic_aead.h"
#include "packet/header.h"
#include "packet/unpacked_packet.h"
#include <memory>
#include <vector>

namespace quicpp {
namespace packet {

class unpacker {
private:
    quicpp::packet::quic_aead &aead;
public:
    unpacker(quicpp::packet::quic_aead &aead);
    std::pair<std::shared_ptr<quicpp::packet::unpacked_packet>,
        quicpp::base::error_t>
    unpack(std::basic_string<uint8_t> &header_binary,
           quicpp::packet::header &header,
           std::basic_string<uint8_t> &data);
    std::pair<std::vector<std::shared_ptr<quicpp::frame::frame>>,
        quicpp::base::error_t>
    parse_frames(std::basic_string<uint8_t> &decrypted,
                 quicpp::packet::header &header);
};

}
}

#endif
