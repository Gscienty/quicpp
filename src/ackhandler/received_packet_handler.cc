#include "ackhandler/received_packet_handler.h"
#include <algorithm>

quicpp::ackhandler::received_packet_handler::
received_packet_handler(quicpp::congestion::rtt &rtt)
    : largest_observed(0)
    , _ignore_below(0)
    , largest_observed_received_time(std::chrono::system_clock::time_point::min())
    , ack_send_delay(std::chrono::milliseconds::zero())
    , rtt(rtt)
    , packets_received_since_last_ack(0)
    , retransmittable_packets_reveived_since_last_ack(0)
    , ack_alarm(std::chrono::system_clock::time_point::min())
    , last_ack(nullptr) {}

quicpp::base::error_t
quicpp::ackhandler::received_packet_handler::
received_packet(uint64_t packet_number,
                std::chrono::system_clock::time_point rcv_time,
                bool should_instigate_ack) {
    if (packet_number < this->_ignore_below) {
        return quicpp::error::success;
    }

    bool is_missing = this->is_missing(packet_number);
    if (packet_number > this->largest_observed) {
        this->largest_observed = packet_number;
        this->largest_observed_received_time = rcv_time;
    }

    quicpp::base::error_t err = this->packet_history
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
    this->packet_history.delete_below(p);
}

bool quicpp::ackhandler::received_packet_handler::is_missing(uint64_t p) {
    if (this->last_ack == nullptr || p < this->_ignore_below) {
        return false;
    }
    return p < this->last_ack->largest() && !this->last_ack->acks_packet(p);
}

bool quicpp::ackhandler::received_packet_handler::has_new_missing_packets() {
    if (this->last_ack == nullptr) {
        return false;
    }
    auto highest_range = this->packet_history.get_highest_ack_range();
    return std::get<quicpp::frame::ack_range_smallest>(highest_range) >=
        this->last_ack->largest() && 
        std::get<quicpp::frame::ack_range_largest>(highest_range) -
        std::get<quicpp::frame::ack_range_smallest>(highest_range) + 1 <=
        quicpp::ackhandler::max_packets_after_new_missing;
}

void quicpp::ackhandler::received_packet_handler::
maybe_queue_ack(uint64_t packet_number,
                std::chrono::system_clock::time_point rcv_time,
                bool should_instigate_ack,
                bool was_missing) {
    this->packets_received_since_last_ack++;

    if (this->last_ack == nullptr) {
        this->ack_queued = true;
        return;
    }

    if (was_missing) {
        this->ack_queued = true;
    }

    if (!this->ack_queued && should_instigate_ack) {
        this->retransmittable_packets_reveived_since_last_ack++;

        if (packet_number >
            quicpp::ackhandler::min_received_before_ack_decimation) {
            if (this->retransmittable_packets_reveived_since_last_ack >=
                quicpp::ackhandler::retransmittalbe_packets_before_ack)     {
                this->ack_queued = true;
            }
            else if (this->ack_alarm ==
                     std::chrono::system_clock::time_point::min()) {
                std::chrono::milliseconds ack_delay =
                    std::min(quicpp::ackhandler::ack_send_delay,
                             std::chrono::milliseconds(uint64_t(double(this->rtt.min().count()) *
                                                       double(quicpp::ackhandler::ack_decimation_delay))));
                this->ack_alarm = rcv_time + ack_delay;
            }
        }
        else {
            if (this->retransmittable_packets_reveived_since_last_ack >=
                quicpp::ackhandler::initial_retransmittable_packets_befor_ack) {
                this->ack_queued = true;
            }
            else if (this->ack_alarm == std::chrono::system_clock::time_point::min()) {
                this->ack_alarm = rcv_time + quicpp::ackhandler::ack_send_delay;
            }
        }

        if (this->has_new_missing_packets()) {
            std::chrono::milliseconds ack_delay =
                std::chrono::milliseconds(uint64_t(double(this->rtt.min().count()) *
                                                   double(quicpp::ackhandler::short_ack_decimation_delay)));

            std::chrono::system_clock::time_point ack_time = rcv_time + ack_delay;
            if (this->ack_alarm == std::chrono::system_clock::time_point::min() ||
                this->ack_alarm > ack_time){
                this->ack_alarm = ack_time;
            }
        }
    }

    if (this->ack_queued) {
        this->ack_alarm = std::chrono::system_clock::time_point::min();
    }
}

std::shared_ptr<quicpp::frame::ack>
quicpp::ackhandler::received_packet_handler::get_ack_frame() {
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    if (!this->ack_queued &&
        (this->ack_alarm == std::chrono::system_clock::time_point::min() ||
         this->ack_alarm > now)) {
        return nullptr;
    }

    std::shared_ptr<quicpp::frame::ack> ack = std::make_shared<quicpp::frame::ack>();

    ack->ranges() = this->packet_history.get_ack_ranges();
    ack->delay() = now - this->largest_observed_received_time;

    this->last_ack = ack;
    this->ack_alarm = std::chrono::system_clock::time_point::min();
    this->ack_queued = false;
    this->packets_received_since_last_ack = 0;
    this->retransmittable_packets_reveived_since_last_ack = 0;

    return ack;
}

std::chrono::system_clock::time_point &
quicpp::ackhandler::received_packet_handler::alarm_timeout() {
    return this->ack_alarm;
}
