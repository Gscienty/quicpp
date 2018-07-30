#include "congestion/rtt.h"
#include <algorithm>

quicpp::congestion::rtt::rtt()
    : _min(0)
    , _latest(0)
    , _smoothed(0)
    , _mean_deviation(0) {}

std::chrono::microseconds &quicpp::congestion::rtt::min() {
    return this->_min;
}

std::chrono::microseconds &quicpp::congestion::rtt::latest() {
    return this->_latest;
}

std::chrono::microseconds &quicpp::congestion::rtt::smoothed() {
    return this->_smoothed;
}

std::chrono::microseconds &quicpp::congestion::rtt::mean_deviation() {
    return this->_mean_deviation;
}

std::chrono::microseconds quicpp::congestion::rtt::smoothed_or_initial() const {
    if (this->_smoothed != std::chrono::microseconds::zero()) {
        return this->_smoothed;
    }
    return quicpp::congestion::default_initial_rtt;
}

void quicpp::congestion::rtt::update(std::chrono::microseconds send_delta,
                                     std::chrono::microseconds ack_delay,
                                     std::chrono::system_clock::time_point now) {
    if (send_delta == std::chrono::microseconds::max() || send_delta <= std::chrono::nanoseconds::zero()) {
        return;
    }

    if (this->_min == std::chrono::microseconds::zero() || this->_min > send_delta) {
        this->_min = send_delta;
    }

    std::chrono::microseconds sample = send_delta;

    if (sample - this->_min >= ack_delay) {
        sample -= ack_delay;
    }
    this->_latest = sample;

    if (this->_smoothed == std::chrono::microseconds::zero()) {
        this->_smoothed = sample;
        this->_mean_deviation = sample / 2;
    }
    else {
        this->_mean_deviation = std::chrono::microseconds(uint64_t((1 - quicpp::congestion::rtt_beta) * double(this->_mean_deviation.count()) +
                                                                   quicpp::congestion::rtt_beta * std::abs((this->_smoothed - sample).count())));
        this->_smoothed = std::chrono::microseconds(uint64_t((1 - quicpp::congestion::rtt_alpha) * double(this->_smoothed.count()) +
                                                             quicpp::congestion::rtt_alpha * sample.count()));
    }
}

void quicpp::congestion::rtt::on_connection_migration() {
    this->_latest = std::chrono::microseconds::zero();
    this->_min = std::chrono::microseconds::zero();
    this->_smoothed = std::chrono::microseconds::zero();
    this->_mean_deviation = std::chrono::microseconds::zero();
}

void quicpp::congestion::rtt::expire_smoothed_metrics() {
    this->_mean_deviation = std::max(this->_mean_deviation, std::chrono::microseconds(std::abs(this->_smoothed.count() - this->_latest.count())));
    this->_smoothed = std::max(this->_smoothed, this->_latest);
}
