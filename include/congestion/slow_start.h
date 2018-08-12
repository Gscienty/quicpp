#ifndef _QUICPP_CONGESTION_SLOW_START_
#define _QUICPP_CONGESTION_SLOW_START_

#include <cstdint>
#include <chrono>

namespace quicpp {
namespace congestion {

const uint64_t hybrid_start_low_wnd = 16;
const uint32_t hybrid_start_min_sample = 8;
const int hybrid_start_delay_factor_exp = 3;
const uint64_t hybrid_start_delay_min_threhold_us = 4000;
const uint64_t hybrid_start_delay_max_threhold_us = 16000;

class slow_start {
private:
    uint64_t end_pn;
    uint64_t last_sent_pn;
    bool started;
    std::chrono::microseconds current_min_rtt;
    uint32_t rtt_sample_count;
    bool hystart_found;
public:
    void start_receive_round(const uint64_t last_sent);
    bool is_end_of_round(const uint64_t ack) const;
    bool should_exist_slowstart(const std::chrono::microseconds latest_rtt,
                                const std::chrono::microseconds min_rtt,
                                const uint64_t cwnd);
    void on_packet_sent(const uint64_t pn);
    void on_packet_acked(const uint64_t acked_pn);
    void restart();
};

}
}

#endif
