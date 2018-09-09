#include "congestion/cubic.h"
#include <iostream>
#include <algorithm>

quicpp::congestion::cubic::cubic()
    : num_conns(quicpp::congestion::default_num_conns) {
    this->reset();
}

void quicpp::congestion::cubic::reset() {
    this->epoch = std::chrono::system_clock::time_point::min();
    this->last_max_cwnd = 0;
    this->acked_bytes_count = 0;
    this->estimated_tcp_cwnd = 0;
    this->origin_point_cwnd = 0;
    this->time_to_origin_point = 0;
    this->last_target_cwnd = 0;
}

double quicpp::congestion::cubic::alpha() const {
    double b = this->beta();
    return 3 * double(this->num_conns) * double(this->num_conns) * (1 - b) / (1 + b);
}

double quicpp::congestion::cubic::beta() const {
    return (double(this->num_conns) - 1 + quicpp::congestion::_beta) / 
        double(this->num_conns);
}

double quicpp::congestion::cubic::beta_last_max() const {
    return (double(this->num_conns) - 1 + quicpp::congestion::beta_last_max) /
        double(this->num_conns);
}

void quicpp::congestion::cubic::on_application_limited() {
    this->epoch = std::chrono::system_clock::time_point::min();
}

uint64_t quicpp::congestion::cubic::cwnd_after_packet_loss(const uint64_t cwnd) {
    if (cwnd + quicpp::default_tcp_mss < this->last_max_cwnd) {
        this->last_max_cwnd = uint64_t(this->beta_last_max() * double(cwnd));
    }
    else {
        this->last_max_cwnd = cwnd;
    }

    this->epoch = std::chrono::system_clock::time_point::min();
    return uint64_t(double(cwnd) * this->beta());
}

uint64_t
quicpp::congestion::cubic::
cwnd_after_ack(const uint64_t acked_bytes,
               const uint64_t cwnd,
               const std::chrono::microseconds delay_min,
               const std::chrono::system_clock::time_point event_time) {

    using namespace quicpp::congestion;

    this->acked_bytes_count += acked_bytes;

    if (this->epoch == std::chrono::system_clock::time_point::min()) {
        this->epoch = event_time;
        this->acked_bytes_count = acked_bytes;
        this->estimated_tcp_cwnd = cwnd;
        if (this->last_max_cwnd <= cwnd) {
            this->time_to_origin_point = 0;
            this->origin_point_cwnd = cwnd;
        }
        else {
            this->time_to_origin_point =
                uint32_t(std::cbrt(double(cube_factor *
                                          (this->last_max_cwnd - cwnd))));
            this->origin_point_cwnd = this->last_max_cwnd;
        }
    }

    int64_t elapsed_time((std::chrono::duration_cast<std::chrono::microseconds>(
                         event_time + delay_min - this->epoch).count() << 10) /
                         (1000 * 1000));
    
    int64_t offset = std::abs(int64_t(this->time_to_origin_point) - elapsed_time);
    uint64_t delta_cwnd = uint64_t(cube_cwnd_scale * offset * offset * offset) *
        quicpp::default_tcp_mss >> cube_scale;
    uint64_t target_cwnd;
    if (elapsed_time > int64_t(this->time_to_origin_point)) {
        target_cwnd = this->origin_point_cwnd + delta_cwnd;
    }
    else {
        target_cwnd = this->origin_point_cwnd - delta_cwnd;
    }

    target_cwnd = std::min(target_cwnd, cwnd + this->acked_bytes_count / 2);

    this->estimated_tcp_cwnd +=
        uint64_t(double(this->acked_bytes_count) *
                 this->alpha() *
                 double(quicpp::default_tcp_mss) /
                 double(this->estimated_tcp_cwnd));

    this->acked_bytes_count = 0;
    this->last_target_cwnd = target_cwnd;

    if (target_cwnd < this->estimated_tcp_cwnd) {
        target_cwnd = this->estimated_tcp_cwnd;
    }
    return target_cwnd;
}

int &quicpp::congestion::cubic::num_connections() {
    return this->num_conns;
}
