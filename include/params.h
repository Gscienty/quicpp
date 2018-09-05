#ifndef _QUICPP_PARAMS_
#define _QUICPP_PARAMS_

#include <cstdint>
#include <chrono>

namespace quicpp {

const double window_update_threshole = 0.25;
const double connection_flowcontrol_multiplier = 1.5;
const uint64_t default_tcp_mss = 1460;

const int default_max_cwnd_packets = 1000;
const uint64_t initial_cwnd = 32 * default_tcp_mss;
const uint64_t default_max_cwnd = default_max_cwnd_packets * default_tcp_mss;
const int max_tracked_skipped_packets = 10;
const int max_outstanding_sent_packets = 2 * default_max_cwnd_packets;
const std::chrono::microseconds min_pacing_delay(100);


}

#endif
