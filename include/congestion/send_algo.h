#ifndef _QUICPP_CONGESTION_SEND_ALGO_
#define _QUICPP_CONGESTION_SEND_ALGO_

#include <chrono>

namespace quicpp {
namespace congestion {

class send_algo {
    virtual
    std::chrono::microseconds 
    time_until_send(const uint64_t bytes_inflight) = 0;
    virtual
    void 
    on_packet_sent(const uint64_t pn,
                   const uint64_t bytes,
                   const bool is_retransmittable) = 0;

    virtual uint64_t &cwnd() = 0;

    virtual void maybe_exit_slowstart() = 0;

    virtual
    void
    on_packet_acked(const uint64_t acked_pn,
                    const uint64_t acked_bytes,
                    const uint64_t prior_inflight,
                    const std::chrono::system_clock::time_point event_time) = 0;

    virtual
    void 
    on_packet_lost(const uint64_t pn,
                   const uint64_t lost_bytes,
                   const uint64_t prior_inflight) = 0;

    virtual void set_num_emulated_connections(int n) = 0;
    virtual void on_retransmission_timeout(bool packets_retransmitted) = 0;
    virtual void on_connection_migration() = 0;
    virtual void set_slowstart_large_reduction(bool enabled) = 0;
};
}

#endif
