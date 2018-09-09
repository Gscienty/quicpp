#ifndef _QUICPP_ACKHANDLER_RECEIVED_PACKET_HISTORY_
#define _QUICPP_ACKHANDLER_RECEIVED_PACKET_HISTORY_

#include "base/error.h"
#include <list>
#include <utility>
#include <cstdint>

namespace quicpp {
namespace ackhandler {

const int packet_interval_start = 1;
const int packet_interval_end = 0;

class received_packet_history {
private:
    std::list<std::pair<uint64_t, uint64_t>> ranges;
    uint64_t lowest_in_received_packet_numbers;
public:
    quicpp::base::error_t received_packet(uint64_t p);
};

}
}

#endif
