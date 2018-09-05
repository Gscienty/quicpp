#ifndef _QUICPP_ACKHANDLER_PACKET_
#define _QUICPP_ACKHANDLER_PACKET_

#include "frame/type.h"
#include <cstdint>
#include <vector>
#include <chrono>

namespace quicpp {
namespace ackhandler {

struct packet {
    uint64_t packet_number;
    uint8_t packet_type;
    std::vector<quicpp::frame::frame *> frames;
    uint64_t len;
    uint8_t encryption_level;
    std::chrono::system_clock::time_point send_time;

    uint64_t largest_acked;
    bool can_be_retransmitted;
    bool included_in_bytes_inflight;
    std::vector<uint64_t> retransmitted_as;
    bool is_retransmission;
    uint64_t retransmitted_of;
};
}
}

#endif
