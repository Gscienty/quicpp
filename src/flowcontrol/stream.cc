#include "flowcontrol/stream.h"
#include "params.h"
#include <algorithm>

quicpp::flowcontrol::stream::stream(quicpp::base::stream_id_t stream_id,
                                    bool contributes_to_connection,
                                    quicpp::flowcontrol::connection &connection,
                                    uint64_t rwnd,
                                    uint64_t max_rwnd,
                                    uint64_t swnd,
                                    std::function<void (const quicpp::base::stream_id_t &)> update_func,
                                    quicpp::congestion::rtt &rtt)
    : quicpp::flowcontrol::base(rwnd, max_rwnd, swnd, rtt)
    , stream_id(stream_id)
    , update_func([&, this] () -> void { this->_update_func(this->stream_id); })
    , _update_func(update_func)
    , connection(connection)
    , contributes_to_connection(contributes_to_connection)
    , received_final_offset(0) {}

quicpp::base::error_t
quicpp::flowcontrol::stream::update_highest_received(uint64_t offset, bool final) {
    std::lock_guard<std::mutex> locker(this->mtx);

    if (final && this->received_final_offset && offset != this->highest_received) {
        return quicpp::error::stream_data_after_termination;
    }

    if (this->received_final_offset && offset > this->highest_received) {
        return quicpp::error::stream_data_after_termination;
    }
    if (final) {
        this->received_final_offset = true;
    }
    if (offset == this->highest_received) {
        return quicpp::error::success;
    }
    if (offset <= this->highest_received) {
        if (final) {
            return quicpp::error::stream_data_after_termination;
        }
        return quicpp::error::success;
    }

    uint64_t increment = offset - this->highest_received;
    this->highest_received = offset;
    if (this->check_flowcontrol_violation()) {
        return quicpp::error::flowcontrol_recv_too_much_data;
    }
    if (this->contributes_to_connection) {
        this->connection.increment_highest_received(increment);
    }
    return quicpp::error::success;
}

void quicpp::flowcontrol::stream::sent(const uint64_t &n) {
    this->base::sent(n);
    if (this->contributes_to_connection) {
        this->connection.sent(n);
    }
}

void quicpp::flowcontrol::stream::read(const uint64_t &n) {
    this->base::read(n);
    if (this->contributes_to_connection) {
        this->connection.read(n);
    }
}

uint64_t quicpp::flowcontrol::stream::send_window() const {
    uint64_t wnd = this->base::_send_window();
    if (this->contributes_to_connection) {
        wnd = std::min(wnd, this->connection.send_window());
    }
    return wnd;
}

void quicpp::flowcontrol::stream::maybe_update() {
    bool has_update = false;
    {
        std::lock_guard<std::mutex> locker(this->mtx);
        has_update = !this->received_final_offset && this->has_update();
    }

    if (has_update) {
        this->update_func();
    }

    if (this->contributes_to_connection) {
        this->connection.base::maybe_update();
    }
}

uint64_t quicpp::flowcontrol::stream::update() {
    std::lock_guard<std::mutex> locker(this->mtx);

    if (this->received_final_offset) {
        return 0;
    }

    uint64_t old_wnd = this->rwnd_size;
    uint64_t offset = this->_update();
    if (this->rwnd_size > old_wnd) {
        if (this->contributes_to_connection) {
            this->connection.ensure_min_wnd(uint64_t(double(this->rwnd_size) * 
                                                     quicpp::connection_flowcontrol_multiplier));
        }
    }
    return offset;
}
