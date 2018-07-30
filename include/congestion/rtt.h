#ifndef _QUICPP_CONGESTION_RTT_
#define _QUICPP_CONGESTION_RTT_

#include <chrono>

namespace quicpp {
namespace congestion {

const std::chrono::microseconds default_initial_rtt = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(100));
const double rtt_alpha = 0.125;
const double rtt_beta = 0.25;

class rtt {
private:
    std::chrono::microseconds _min;
    std::chrono::microseconds _latest;
    std::chrono::microseconds _smoothed;
    std::chrono::microseconds _mean_deviation;
public:
    rtt();

    std::chrono::microseconds &min();
    std::chrono::microseconds &latest();
    std::chrono::microseconds &smoothed();
    std::chrono::microseconds &mean_deviation();
    std::chrono::microseconds smoothed_or_initial() const;

    void update(std::chrono::microseconds send_delta,
                std::chrono::microseconds ack_delay,
                std::chrono::system_clock::time_point now);

    void on_connection_migration();
    void expire_smoothed_metrics();
};

}
}

#endif
