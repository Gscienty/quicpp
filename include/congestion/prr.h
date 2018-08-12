#ifndef _QUICPP_CONGESTION_PRR_
#define _QUICPP_CONGESTION_PRR_

#include <cstdint>

namespace quicpp {
namespace congestion {

class prr {
private:
    uint64_t sent_since_loss;
    uint64_t delivered_since_loss;
    uint64_t ack_count_since_loss;
    uint64_t inflight_since_loss;
public:
    void on_packet_send(const uint64_t sent_bytes);
    void on_packet_lost(const uint64_t prior_inflight);
    void on_packet_acked(const uint64_t acked_bytes);
    bool can_send(const uint64_t cwnd,
                  const uint64_t inflight,
                  const uint64_t slowstart_threhold);
};

}
}

#endif
