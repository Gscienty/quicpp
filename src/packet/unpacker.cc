#include "packet/unpacker.h"
#include "frame/parser.h"
#include "crypt/encryption_level.h"
#include <sstream>

quicpp::packet::unpacker::unpacker(quicpp::packet::quic_aead &aead)
    : aead(aead) {}

std::pair<std::shared_ptr<quicpp::packet::unpacked_packet>,
    quicpp::base::error_t>
quicpp::packet::unpacker::
unpack(std::basic_string<uint8_t> &header_binary,
       quicpp::packet::header &header,
       std::basic_string<uint8_t> &data) {
    std::basic_string<uint8_t> decrypted;
    uint8_t encryption_level;
    quicpp::base::error_t err;

    if (header.header_form() == quicpp::packet::header_form_long_header) {
        std::tie(decrypted, err) =
            this->aead.open_handshake(data,
                                      header.packet_number(),
                                      header_binary);
        encryption_level = quicpp::crypt::encryption_unencrypted;
    }
    else {
        std::tie(decrypted, err) =
            this->aead.open_1rtt(data,
                                 header.packet_number(),
                                 header_binary);
        encryption_level = quicpp::crypt::encryption_forward_secure;
    }
    
    if (err != quicpp::error::success) {
        return std::make_pair(nullptr, err);
    }

    auto fs = this->parse_frames(decrypted, header);
    if (std::get<1>(fs) != quicpp::error::success) {
        return std::make_pair(nullptr, std::get<1>(fs));
    }

    std::shared_ptr<quicpp::packet::unpacked_packet> packet;
    packet->encryption_level() = encryption_level;
    packet->frames() = std::get<0>(fs);
    
    return std::make_pair(packet, quicpp::error::success);
}

std::pair<std::vector<std::shared_ptr<quicpp::frame::frame>>,
    quicpp::base::error_t>
quicpp::packet::unpacker::
parse_frames(std::basic_string<uint8_t> &decrypted,
             quicpp::packet::header &header) {
    if (decrypted.empty()) {
        return std::make_pair(std::vector<std::shared_ptr<quicpp::frame::frame>>(),
                              quicpp::error::missing_payload);
    }

    std::vector<std::shared_ptr<quicpp::frame::frame>> fs;

    std::basic_istringstream<uint8_t> in_stream;
    in_stream.rdbuf()->pubsetbuf(const_cast<uint8_t *>(decrypted.data()),
                                 decrypted.size());

    for (;;) {
        auto frame = quicpp::frame::parse_next_frame(in_stream);
        if (bool(frame) == false) {
            break;
        }

        fs.push_back(frame);
    }

    return std::make_pair(fs, quicpp::error::success);
}
