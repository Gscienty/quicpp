#ifndef _QUICPP_ACKHANDLER_RECEIVED_PACKET_HISTORY_
#define _QUICPP_ACKHANDLER_RECEIVED_PACKET_HISTORY_

#include "base/error.h"
#include <list>
#include <utility>
#include <cstdint>
#include <vector>

namespace quicpp {
namespace ackhandler {

const int packet_interval_start = 0;
const int packet_interval_end = 1;

class received_packet_history {
private:
    std::list<std::pair<uint64_t, uint64_t>> _ranges;
    uint64_t _lowest_in_received_packet_numbers;
public:
    received_packet_history();
    quicpp::base::error_t received_packet(uint64_t p);
    void delete_below(uint64_t p);
    std::vector<std::pair<uint64_t, uint64_t>> get_ack_ranges();
    std::pair<uint64_t, uint64_t> get_highest_ack_range();

    std::list<std::pair<uint64_t, uint64_t>> &ranges();
    uint64_t &lowest_in_received_packet_numbers();
};

}
}

#endif
