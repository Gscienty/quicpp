#include "congestion/cubic_sender.h"
#include <algorithm>

quicpp::congestion::cubic_sender::cubic_sender(quicpp::congestion::rtt &rtt,
                                               const uint64_t cwnd,
                                               const uint64_t max_cwnd)
    : rtt(rtt)
    , _cwnd(cwnd)
    , min_cwnd(quicpp::congestion::default_min_cwnd)
    , max_cwnd(max_cwnd)
    , _slowstart_threhold(max_cwnd)
    , num_conns(quicpp::congestion::default_num_conns)
    , initial_cwnd(cwnd)
    , initial_max_cwnd(cwnd) {}


std::chrono::microseconds
quicpp::congestion::cubic_sender::time_until_send(const uint64_t bytes_inflight) {
    if (this->in_recovery()) {
        if (this->prr.can_send(this->cwnd(),
                               bytes_inflight,
                               this->slowstart_threhold())) {
            return std::chrono::microseconds::zero();
        }
    }
    std::chrono::microseconds delay(this->rtt.smoothed() / 
                                    std::chrono::microseconds(2 * this->cwnd()));
    if (!this->in_slowstart()) {
        delay = delay * 8 / 5;
    }
    return delay;
}

void
quicpp::congestion::cubic_sender::on_packet_sent(const uint64_t pn,
                                                 const uint64_t bytes,
                                                 const bool is_retransmittable) {
    if (!is_retransmittable) {
        return;
    }
    if (this->in_recovery()) {
        this->prr.on_packet_send(bytes);
    }
    this->largest_sent_pn = pn;
    this->_slow_start.on_packet_sent(pn);
}

bool quicpp::congestion::cubic_sender::in_recovery() const {
    return this->largest_acked_pn <= this->largest_sent_at_last_cutback &&
        this->largest_acked_pn != 0;
}

bool quicpp::congestion::cubic_sender::in_slowstart() const {
    return this->_cwnd < this->_slowstart_threhold;
}

uint64_t &quicpp::congestion::cubic_sender::cwnd() {
    return this->_cwnd;
}

uint64_t &quicpp::congestion::cubic_sender::slowstart_threhold() {
    return this->_slowstart_threhold;
}

void quicpp::congestion::cubic_sender::exit_slowstart() {
    this->_slowstart_threhold = this->_cwnd;
}

void quicpp::congestion::cubic_sender::maybe_exit_slowstart() {
    if (this->in_slowstart() &&
        this->_slow_start
            .should_exist_slowstart(this->rtt.latest(),
                                    this->rtt.min(),
                                    this->cwnd() / quicpp::default_tcp_mss)) {
        this->exit_slowstart();
    }
}

void
quicpp::congestion::cubic_sender::
on_packet_acked(const uint64_t acked_pn,
                const uint64_t acked_bytes,
                const uint64_t prior_inflight,
                const std::chrono::system_clock::time_point event_time) {
    this->largest_acked_pn = std::max(acked_pn, this->largest_acked_pn);
    if (this->in_recovery()) {
        this->prr.on_packet_acked(acked_bytes);
        return;
    }
    this->maybe_increase_cwnd(acked_bytes, prior_inflight, event_time);
    if (this->in_slowstart()) {
        this->_slow_start.on_packet_acked(acked_pn);
    }
}

void quicpp::congestion::cubic_sender::on_packet_lost(const uint64_t pn,
                                                      const uint64_t lost_bytes,
                                                      const uint64_t prior_inflight) {
    if (pn <= this->largest_sent_at_last_cutback) {
        if (this->last_cutback_exited_slowstart) {
            this->stats.slowstart_packets_lost++;
            this->stats.slowstart_bytes_lost += lost_bytes;
            if (this->slowstart_large_reduction) {
                this->_cwnd = std::max(this->_cwnd - lost_bytes,
                                       this->min_slowstart_exit_wnd);
                this->_slowstart_threhold = this->_cwnd;
            }
        }
        return;
    }
    this->last_cutback_exited_slowstart = this->in_slowstart();
    if (this->in_slowstart()) {
        this->stats.slowstart_packets_lost++;
    }

    this->prr.on_packet_lost(prior_inflight);

    if (this->slowstart_large_reduction && this->in_slowstart()) {
        if (this->_cwnd >= 2 * this->initial_cwnd) {
            this->min_slowstart_exit_wnd = this->_cwnd / 2;
        }
        this->_cwnd = this->_cwnd - quicpp::default_tcp_mss;
    }
    else {
        this->_cwnd = this->cubic.cwnd_after_packet_loss(this->_cwnd);
    }

    this->_cwnd = std::max(this->_cwnd, this->min_cwnd);
    this->_slowstart_threhold = this->_cwnd;
    this->largest_sent_at_last_cutback = this->largest_sent_pn;
    this->num_acked_packets = 0;
}

void
quicpp::congestion::cubic_sender::
maybe_increase_cwnd(const uint64_t acked_bytes,
                    const uint64_t prior_inflight,
                    const std::chrono::system_clock::time_point event_time) {
    if (!this->is_cwnd_limited(prior_inflight)) {
        this->cubic.on_application_limited();
        return;
    }
    if (this->_cwnd >= this->max_cwnd) {
        return;
    }

    if (this->in_slowstart()) {
        this->_cwnd += quicpp::default_tcp_mss;
        return;
    }
    this->_cwnd = std::min(this->max_cwnd,
                           this->cubic.cwnd_after_ack(acked_bytes,
                                                      this->_cwnd,
                                                      this->rtt.min(),
                                                      event_time));
}

bool
quicpp::congestion::cubic_sender::is_cwnd_limited(const uint64_t bytes_inflight) {
    uint64_t cwnd = this->cwnd();
    if (bytes_inflight >= cwnd) {
        return true;
    }
    uint64_t available_bytes = cwnd - bytes_inflight;
    bool slowstart_limited = this->in_slowstart() && bytes_inflight > cwnd / 2;
    return slowstart_limited || 
        available_bytes <= quicpp::congestion::max_burst_bytes;
}
