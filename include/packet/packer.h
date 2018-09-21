#ifndef _QUICPP_PACKET_PACKER_
#define _QUICPP_PACKET_PACKER_

#include "packet/sealing_manager.h"
#include "packet/stream_frame_source.h"
#include "packet/header.h"
#include "base/packet_number.h"
#include "base/conn_id.h"
#include "frame/type.h"
#include "frame/ack.h"
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <utility>

namespace quicpp {
namespace packet {

class packer {
private:
    quicpp::base::conn_id dest_connid;
    quicpp::base::conn_id src_connid;

    bool is_client;
    uint64_t version;
    std::basic_string<uint8_t> div_nonce;
    quicpp::packet::sealing_manager &crypto_setup;

    quicpp::base::packet_number_generator packet_number_generator;
    std::function<size_t (uint64_t packet_number)> get_packet_number_length;
    quicpp::packet::stream_frame_source &streams;

    std::mutex control_frame_mutex;
    std::vector<std::shared_ptr<quicpp::frame::frame>> control_frames;
    std::shared_ptr<quicpp::frame::ack> ack_frame;
    bool omit_connid;
    uint64_t max_packet_size;
    bool has_sent_packet;
    int num_non_retransmittable_acks;
public:
    packer(quicpp::base::conn_id dest_connid,
           quicpp::base::conn_id src_connid,
           uint64_t initial_packet_number,
           std::function<size_t (uint64_t packet_number)> get_packet_number_length,
           size_t max_packet_size,
           std::basic_string<uint8_t> div_nonce,
           quicpp::packet::sealing_manager &crypto_setup,
           quicpp::packet::stream_frame_source &stream_framer,
           bool is_client);

    std::pair<std::basic_string<uint8_t>, quicpp::base::error_t>
    write_and_seal_packet(quicpp::packet::header &header,
                          std::vector<std::shared_ptr<quicpp::frame::frame>> &payload_frames,
                          quicpp::handshake::sealer &sealer);
};

}
}

#endif
