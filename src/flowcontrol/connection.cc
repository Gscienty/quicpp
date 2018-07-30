#include "flowcontrol/connection.h"
#include <algorithm>

quicpp::flowcontrol::connection::connection(uint64_t rwnd,
                                            uint64_t max_rwnd,
                                            std::function<void ()> update_func,
                                            quicpp::congestion::rtt &rtt)
    : quicpp::flowcontrol::base(rwnd, max_rwnd, rtt)
    , update_func(update_func) {}

uint64_t quicpp::flowcontrol::connection::send_window() const {
    return this->_send_window();
}

uint64_t quicpp::flowcontrol::connection::update() {
    std::lock_guard<std::mutex> lock(this->mtx);
    return this->_update();
}

void quicpp::flowcontrol::connection::maybe_update() {
    bool has_update = false;
    {
        std::lock_guard<std::mutex> lock(this->mtx);
        has_update = this->has_update();
    }

    if (has_update) {
        this->update_func();
    }
}

quicpp::base::error_t
quicpp::flowcontrol::connection::increment_highest_received(uint64_t increment) {
    std::lock_guard<std::mutex> lock(this->mtx);
    this->highest_received += increment;
    if (this->check_flowcontrol_violation()) {
        return quicpp::error::flowcontrol_recv_too_much_data;
    }
    return quicpp::error::success;
}

void quicpp::flowcontrol::connection::ensure_min_wnd(uint64_t inc) {
    std::lock_guard<std::mutex> lock(this->mtx);
    if (inc > this->rwnd_size) {
        this->rwnd_size = std::min(inc, this->max_rwnd_size);
        this->start_new_auto_tuning_epoch();
    }
}
