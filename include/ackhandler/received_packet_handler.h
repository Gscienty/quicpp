#ifndef _QUICPP_ACKHANDLER_RECEIVED_PACKET_HANDLER_
#define _QUICPP_ACKHANDLER_RECEIVED_PACKET_HANDLER_

#include "ackhandler/received_packet_history.h"
#include "congestion/rtt.h"
#include "base/error.h"
#include "frame/ack.h"
#include <cstdint>
#include <chrono>

namespace quicpp {
namespace ackhandler {

const std::chrono::milliseconds ack_send_delay(25);
const int initial_retransmittable_packets_befor_ack = 2;
const int retransmittalbe_packets_before_ack = 10;
const float ack_decimation_delay = 1.0 / 4;
const float short_ack_decimation_delay = 1.0 / 8;
const int min_received_before_ack_decimation = 100;
const int max_packets_after_new_missing = 4;

class received_packet_handler {
private:
    uint64_t largest_observed;
    uint64_t _ignore_below;
    std::chrono::system_clock::time_point largest_observed_received_time;

    quicpp::ackhandler::received_packet_history packet_history;

    std::chrono::milliseconds ack_send_delay;
    quicpp::congestion::rtt &rtt;

    int packets_received_since_last_ack;
    int retransmittable_packets_reveived_since_last_ack;
    bool ack_queued;
    std::chrono::system_clock::time_point ack_alarm;
    quicpp::frame::ack *last_ack;

public:
    received_packet_handler(quicpp::congestion::rtt &rtt);
    quicpp::base::error_t 
    received_packet(uint64_t packet_number,
                    std::chrono::system_clock::time_point rcv_time,
                    bool should_instigate_ack);
    void ignore_below(uint64_t p);
    bool is_missing(uint64_t packet_number);
    bool has_new_missing_packets();
    void maybe_queue_ack(uint64_t packet_number,
                         std::chrono::system_clock::time_point rcv_time,
                         bool should_instigate_ack,
                         bool was_missing);
    quicpp::frame::ack *get_ack_frame();
    std::chrono::system_clock::time_point &alarm_timeout();
};

}
}

#endif
