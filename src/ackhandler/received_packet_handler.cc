#include "ackhandler/received_packet_handler.h"
#include <algorithm>

quicpp::ackhandler::received_packet_handler::
received_packet_handler(quicpp::congestion::rtt &rtt)
    : _largest_observed(0)
    , _ignore_below(0)
    , _largest_observed_received_time(std::chrono::system_clock::time_point::min())
    , _ack_send_delay(std::chrono::milliseconds::zero())
    , _rtt(rtt)
    , _packets_received_since_last_ack(0)
    , _retransmittable_packets_reveived_since_last_ack(0)
    , _ack_alarm(std::chrono::system_clock::time_point::min())
    , _last_ack(nullptr) {}

quicpp::base::error_t
quicpp::ackhandler::received_packet_handler::
received_packet(uint64_t packet_number,
                std::chrono::system_clock::time_point rcv_time,
                bool should_instigate_ack) {
    if (packet_number < this->_ignore_below) {
        return quicpp::error::success;
    }

    bool is_missing = this->is_missing(packet_number);
    if (packet_number > this->_largest_observed) {
        this->_largest_observed = packet_number;
        this->_largest_observed_received_time = rcv_time;
    }

    quicpp::base::error_t err = this->_packet_history
        .received_packet(packet_number);

    if (err != quicpp::error::success) {
        return err;
    }
    this->maybe_queue_ack(packet_number,
                          rcv_time,
                          should_instigate_ack,
                          is_missing);

    return quicpp::error::success;
}

void quicpp::ackhandler::received_packet_handler::ignore_below(uint64_t p) {
    if (p <= this->_ignore_below) {
        return;
    }

    this->_ignore_below = p;
    this->_packet_history.delete_below(p);
}

bool quicpp::ackhandler::received_packet_handler::is_missing(uint64_t p) {
    if (this->_last_ack == nullptr || p < this->_ignore_below) {
        return false;
    }
    return p < this->_last_ack->largest() && !this->_last_ack->acks_packet(p);
}

bool quicpp::ackhandler::received_packet_handler::has_new_missing_packets() {
    if (this->_last_ack == nullptr) {
        return false;
    }
    auto highest_range = this->_packet_history.get_highest_ack_range();
    return std::get<quicpp::frame::ack_range_smallest>(highest_range) >=
        this->_last_ack->largest() && 
        std::get<quicpp::frame::ack_range_largest>(highest_range) -
        std::get<quicpp::frame::ack_range_smallest>(highest_range) + 1 <=
        quicpp::ackhandler::max_packets_after_new_missing;
}

void quicpp::ackhandler::received_packet_handler::
maybe_queue_ack(uint64_t packet_number,
                std::chrono::system_clock::time_point rcv_time,
                bool should_instigate_ack,
                bool was_missing) {
    this->_packets_received_since_last_ack++;

    if (this->_last_ack == nullptr) {
        this->_ack_queued = true;
        return;
    }

    if (was_missing) {
        this->_ack_queued = true;
    }

    if (!this->_ack_queued && should_instigate_ack) {
        this->_retransmittable_packets_reveived_since_last_ack++;

        if (packet_number >
            quicpp::ackhandler::min_received_before_ack_decimation) {
            if (this->_retransmittable_packets_reveived_since_last_ack >=
                quicpp::ackhandler::retransmittalbe_packets_before_ack)     {
                this->_ack_queued = true;
            }
            else if (this->_ack_alarm ==
                     std::chrono::system_clock::time_point::min()) {
                std::chrono::milliseconds ack_delay =
                    std::min(quicpp::ackhandler::ack_send_delay,
                             std::chrono::milliseconds(uint64_t(double(this->_rtt.min().count()) *
                                                       double(quicpp::ackhandler::ack_decimation_delay))));
                this->_ack_alarm = rcv_time + ack_delay;
            }
        }
        else {
            if (this->_retransmittable_packets_reveived_since_last_ack >=
                quicpp::ackhandler::initial_retransmittable_packets_befor_ack) {
                this->_ack_queued = true;
            }
            else if (this->_ack_alarm == std::chrono::system_clock::time_point::min()) {
                this->_ack_alarm = rcv_time + quicpp::ackhandler::ack_send_delay;
            }
        }

        if (this->has_new_missing_packets()) {
            std::chrono::milliseconds ack_delay =
                std::chrono::milliseconds(uint64_t(double(this->_rtt.min().count()) *
                                                   double(quicpp::ackhandler::short_ack_decimation_delay)));

            std::chrono::system_clock::time_point ack_time = rcv_time + ack_delay;
            if (this->_ack_alarm == std::chrono::system_clock::time_point::min() ||
                this->_ack_alarm > ack_time){
                this->_ack_alarm = ack_time;
            }
        }
    }

    if (this->_ack_queued) {
        this->_ack_alarm = std::chrono::system_clock::time_point::min();
    }
}

std::shared_ptr<quicpp::frame::ack>
quicpp::ackhandler::received_packet_handler::get_ack_frame() {
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    if (!this->_ack_queued &&
        (this->_ack_alarm == std::chrono::system_clock::time_point::min() ||
         this->_ack_alarm > now)) {
        return nullptr;
    }

    std::shared_ptr<quicpp::frame::ack> ack = std::make_shared<quicpp::frame::ack>();

    ack->ranges() = this->_packet_history.get_ack_ranges();
    ack->delay() = std::chrono::duration_cast<std::chrono::microseconds>(now - this->_largest_observed_received_time);

    this->_last_ack = ack;
    this->_ack_alarm = std::chrono::system_clock::time_point::min();
    this->_ack_queued = false;
    this->_packets_received_since_last_ack = 0;
    this->_retransmittable_packets_reveived_since_last_ack = 0;

    return ack;
}

std::chrono::system_clock::time_point &
quicpp::ackhandler::received_packet_handler::alarm_timeout() {
    return this->_ack_alarm;
}
