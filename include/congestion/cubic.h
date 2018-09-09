#ifndef _QUICPP_CONGESTION_CUBIC_
#define _QUICPP_CONGESTION_CUBIC_

#include "params.h"
#include <chrono>
#include <cstdint>

namespace quicpp {
namespace congestion {

const int cube_scale = 40;
const int cube_cwnd_scale = 410;
const uint64_t cube_factor = 1 <<
    cube_scale / cube_cwnd_scale / quicpp::default_tcp_mss;
const int default_num_conns = 2;
const double _beta = 0.7;
const double beta_last_max = 0.85;


class cubic {
private:
    int num_conns;
    std::chrono::system_clock::time_point epoch;
    uint64_t last_max_cwnd;
    uint64_t acked_bytes_count;
    uint64_t estimated_tcp_cwnd;
    uint64_t origin_point_cwnd;
    uint32_t time_to_origin_point;
    uint64_t last_target_cwnd;
public:
    cubic();
    uint64_t &last_max_congestion_window() { return this->last_max_cwnd; }
    void reset();
    double alpha() const;
    double beta() const;
    double beta_last_max() const;
    void on_application_limited();
    uint64_t cwnd_after_packet_loss(const uint64_t cwnd);
    uint64_t cwnd_after_ack(const uint64_t acked_bytes,
                            const uint64_t cwnd,
                            const std::chrono::microseconds delay_min,
                            const std::chrono::system_clock::time_point event_time);
    int &num_connections();
};

}
}

#endif
