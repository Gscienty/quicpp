#include "ackhandler/received_packet_handler.h"

quicpp::ackhandler::received_packet_handler::
received_packet_handler(quicpp::congestion::rtt &rtt)
    : largest_observed(0)
    , ignore_below(0)
    , largest_observed_received_time(std::chrono::system_clock::time_point::min())
    , ack_send_delay(std::chrono::milliseconds::zero())
    , rtt(rtt)
    , packets_received_since_last_ack(0)
    , retransmittable_packets_reveived_since_last_ack(0)
    , ack_alarm(std::chrono::system_clock::time_point::min())
    , last_ack(nullptr) {}
