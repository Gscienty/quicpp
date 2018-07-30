#include "flowcontrol/base.h"
#include "params.h"
#include <algorithm>

quicpp::flowcontrol::base::base(uint64_t rwnd, uint64_t max_rwnd, quicpp::congestion::rtt &rtt)
    : sent_bytes(0)
    , swnd(0)
    , last_blocked_at(0)
    , read_bytes(0)
    , highest_received(0)
    , rwnd(rwnd)
    , rwnd_size(rwnd)
    , max_rwnd_size(max_rwnd)
    , epoch_start_time(std::chrono::system_clock::now())
    , epoch_start_offset(0)
    , rtt(rtt) {}

quicpp::flowcontrol::base::base(uint64_t rwnd, uint64_t max_rwnd, uint64_t swnd, quicpp::congestion::rtt &rtt)
    : sent_bytes(0)
    , swnd(swnd)
    , last_blocked_at(0)
    , read_bytes(0)
    , highest_received(0)
    , rwnd(rwnd)
    , rwnd_size(rwnd)
    , max_rwnd_size(max_rwnd)
    , epoch_start_time(std::chrono::system_clock::now())
    , epoch_start_offset(0)
    , rtt(rtt)  {}

std::pair<bool, uint64_t> quicpp::flowcontrol::base::is_newly_blocked() {
    if (this->send_window() != 0 || this->swnd == this->last_blocked_at) {
        return std::make_pair(false, 0);
    }

    this->last_blocked_at = this->swnd;
    return std::make_pair(true, this->swnd);
}

void quicpp::flowcontrol::base::sent(const uint64_t &n) {
    this->sent_bytes += n;
}

void quicpp::flowcontrol::base::send_window(const uint64_t &offset) {
    if (offset > this->swnd) {
        this->swnd = offset;
    }
}

uint64_t quicpp::flowcontrol::base::_send_window() const {
    if (this->sent_bytes > this->swnd) {
        return 0;
    }

    return this->swnd - this->sent_bytes;
}

void quicpp::flowcontrol::base::read(const uint64_t &n) {
    std::lock_guard<std::mutex> locker(this->mtx);

    if (this->read_bytes == 0) {
        this->start_new_auto_tuning_epoch();
    }
    this->read_bytes += n;
}

bool quicpp::flowcontrol::base::has_update() const {
    uint64_t bytes_remaining = this->rwnd - this->read_bytes;
    return bytes_remaining <= double(this->rwnd_size) * double(1 - quicpp::window_update_threshole);
}

uint64_t quicpp::flowcontrol::base::_update() {
    if (this->has_update() == false) {
        return 0;
    }

    this->maybe_adjust_window();
    this->rwnd = this->read_bytes + this->rwnd_size;
    return this->rwnd;
}

void quicpp::flowcontrol::base::maybe_adjust_window() {
    uint64_t bytes_read_in_epoch = this->read_bytes - this->epoch_start_offset;
    if (bytes_read_in_epoch <= this->rwnd_size / 2) {
        return;
    }
    
    std::chrono::microseconds rtt(this->rtt.smoothed());
    if (rtt == std::chrono::microseconds::zero()) {
        return;
    }
    
    double fraction = double(bytes_read_in_epoch) / double(this->rwnd_size);
    if (std::chrono::system_clock::now() - this->epoch_start_time < 4 * fraction * rtt) {
        this->rwnd_size = std::min(2 * this->rwnd_size, this->max_rwnd_size);
    }
    this->start_new_auto_tuning_epoch();
}

void quicpp::flowcontrol::base::start_new_auto_tuning_epoch() {
    this->epoch_start_time = std::chrono::system_clock::now();
    this->epoch_start_offset = this->read_bytes;
}

bool quicpp::flowcontrol::base::check_flowcontrol_violation() const {
    return this->highest_received > this->rwnd;
}
