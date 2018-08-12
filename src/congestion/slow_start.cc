#include "congestion/slow_start.h"
#include <algorithm>

void
quicpp::congestion::slow_start::
start_receive_round(const uint64_t last_sent) {
    this->end_pn = last_sent;
    this->current_min_rtt = std::chrono::microseconds::zero();
    this->rtt_sample_count = 0;
    this->started = true;
}

bool
quicpp::congestion::slow_start::
is_end_of_round(const uint64_t ack) const {
    return this->end_pn < ack;
}

bool
quicpp::congestion::slow_start::
should_exist_slowstart(const std::chrono::microseconds latest_rtt,
                       const std::chrono::microseconds min_rtt,
                       const uint64_t cwnd) {

    using namespace quicpp::congestion;

    if (!this->started) {
        this->start_receive_round(this->last_sent_pn);
    }
    if (this->hystart_found) {
        return true;
    }

    this->rtt_sample_count++;
    if (this->rtt_sample_count <= hybrid_start_min_sample) {
        if (this->current_min_rtt == std::chrono::microseconds::zero() ||
            this->current_min_rtt > latest_rtt) {
            this->current_min_rtt = latest_rtt;
        }
    }

    if (this->rtt_sample_count == hybrid_start_min_sample) {
        uint64_t increase(min_rtt.count() >> hybrid_start_delay_factor_exp);
        increase = std::min(increase, hybrid_start_delay_max_threhold_us);

        std::chrono::microseconds 
            increase_dur(std::max(increase, hybrid_start_delay_min_threhold_us));

        if (this->current_min_rtt > (min_rtt + increase_dur)) {
            this->hystart_found = true;
        }
    }

    return cwnd >= hybrid_start_low_wnd && this->hystart_found;
}

void quicpp::congestion::slow_start::on_packet_sent(const uint64_t pn) {
    this->last_sent_pn = pn;
}

void quicpp::congestion::slow_start::on_packet_acked(const uint64_t acked_pn) {
    if (this->is_end_of_round(acked_pn)) {
        this->started = false;
    }
}

void quicpp::congestion::slow_start::restart() {
    this->started = false;
    this->hystart_found = false;
}
