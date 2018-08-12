#ifndef _QUICPP_CONGESTION_STATS_
#define _QUICPP_CONGESTION_STATS_

#include <cstdint>

namespace quicpp {
namespace congestion {

struct connection_stats {
    uint64_t slowstart_packets_lost;
    uint64_t slowstart_bytes_lost;
};

}
}

#endif
