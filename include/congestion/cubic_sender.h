#ifndef _QUICPP_CONGESTION_CUBIC_SENDER_
#define _QUICPP_CONGESTION_CUBIC_SENDER_

#include "congestion/stats.h"
#include "congestion/rtt.h"
#include "congestion/prr.h"
#include "congestion/slow_start.h"
#include "congestion/cubic.h"
#include "congestion/send_algo.h"
#include "params.h"
#include <chrono>

namespace quicpp {
namespace congestion {

const uint64_t max_burst_bytes = 3 * quicpp::default_tcp_mss;
const uint64_t default_min_cwnd = 2 * quicpp::default_tcp_mss;

class cubic_sender : public quicpp::congestion::send_algo {
private:
    quicpp::congestion::slow_start _slow_start;
    quicpp::congestion::prr prr;
    quicpp::congestion::rtt &rtt;
    quicpp::congestion::cubic cubic;
    quicpp::congestion::connection_stats stats;

    uint64_t largest_sent_pn;
    uint64_t largest_acked_pn;
    uint64_t largest_sent_at_last_cutback;
    bool last_cutback_exited_slowstart;
    bool slowstart_large_reduction;
    uint64_t _cwnd;
    uint64_t min_cwnd;
    uint64_t max_cwnd;
    uint64_t _slowstart_threhold;
    int num_conns;
    uint64_t num_acked_packets;
    uint64_t initial_cwnd;
    uint64_t initial_max_cwnd;
    uint64_t min_slowstart_exit_wnd;

public:
    cubic_sender(quicpp::congestion::rtt &rtt,
                 const uint64_t cwnd,
                 const uint64_t max_cwnd);

    virtual
    std::chrono::microseconds 
    time_until_send(const uint64_t bytes_inflight) override;
    
    virtual
    void 
    on_packet_sent(const uint64_t pn,
                   const uint64_t bytes,
                   const bool is_retransmittable) override;

    bool in_recovery() const;
    bool in_slowstart() const;
    virtual uint64_t &cwnd() override;
    uint64_t &slowstart_threhold();
    void exit_slowstart();
    virtual void maybe_exit_slowstart() override;

    virtual
    void
    on_packet_acked(const uint64_t acked_pn,
                    const uint64_t acked_bytes,
                    const uint64_t prior_inflight,
                    const std::chrono::system_clock::time_point event_time) override;

    virtual
    void 
    on_packet_lost(const uint64_t pn,
                   const uint64_t lost_bytes,
                   const uint64_t prior_inflight) override;

    void maybe_increase_cwnd(const uint64_t acked_bytes,
                             const uint64_t prior_inflight,
                             const std::chrono::system_clock::time_point event_time);
    bool is_cwnd_limited(const uint64_t bytes_inflight);
    quicpp::congestion::slow_start &slow_start();
    uint64_t bandwidth_estimate();
    void set_num_emulated_connections(int n);
    void on_retransmission_timeout(bool packets_retransmitted);
    void on_connection_migration();
    void set_slowstart_large_reduction(bool enable);
};

}
}

#endif
