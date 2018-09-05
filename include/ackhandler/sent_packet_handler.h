#ifndef _QUICPP_ACKHANDLER_SEND_PACKET_HANDLER_
#define _QUICPP_ACKHANDLER_SEND_PACKET_HANDLER_

#include "ackhandler/send_packet_history.h"
#include "ackhandler/packet.h"
#include "ackhandler/send_mode.h"
#include "congestion/send_algo.h"
#include "congestion/rtt.h"
#include "frame/ack.h"
#include <cstdint>
#include <chrono>
#include <vector>
#include <list>

namespace quicpp {
namespace ackhandler {

const double time_reordering_fraction = 1.0 / 8;
const std::chrono::milliseconds default_rto_timeout(500);
const std::chrono::milliseconds min_tlp_timeout(10);
const int max_tlps = 2;
const std::chrono::milliseconds min_rto_timeout(200);
const std::chrono::seconds max_rto_timeout(60);


class sent_packet_handler {
private:
    uint64_t ls_pn; 
    std::chrono::system_clock::time_point ls_retransmittable_pt;
    std::chrono::system_clock::time_point ls_handshake_pt;

    std::chrono::system_clock::time_point next_packet_st;
    std::vector<uint64_t> skipped_packets;

    uint64_t largest_acked;
    uint64_t largest_received_packet_with_ack;

    uint64_t _lowest_packet_not_confirmed_acked;
    uint64_t largest_sent_before_rto;
    
    quicpp::ackhandler::send_packet_history packet_history;
    std::list<quicpp::ackhandler::packet *> retransmission_queue;

    uint64_t bytes_inflight;

    quicpp::congestion::send_algo *congestion;
    quicpp::congestion::rtt &rtt;

    bool handshake_complete;
    uint32_t handshake_count;

    uint32_t tlp_count;
    bool allow_tlp;

    uint32_t rto_count;
    int num_rtos;

    std::chrono::system_clock::time_point loss_time;
    std::chrono::system_clock::time_point alarm;

public:
    sent_packet_handler(quicpp::congestion::rtt &rtt);
    virtual ~sent_packet_handler();

    uint64_t lowest_unacked() const;
    void set_handshake_complete();
    void sent_packet(quicpp::ackhandler::packet *p);
    void sent_packets_retransmission(std::vector<quicpp::ackhandler::packet *> packets,
                                     uint64_t retransmission_of);
    bool sent_packet_implement(quicpp::ackhandler::packet *p);
    void update_loss_detection_alarm();
    quicpp::base::error_t received_ack(quicpp::frame::ack *ack,
                                       uint64_t with_packet_number,
                                       uint8_t encryption_level,
                                       std::chrono::system_clock::time_point rcv_time);
    uint64_t &lowest_packet_not_confirmed_acked();
    bool skipped_packets_acked(quicpp::frame::ack *ack);
    bool maybe_update_rtt(uint64_t largest_acked,
                          std::chrono::microseconds ack_delay,
                          std::chrono::system_clock::time_point rcv_time);
    std::pair<std::vector<quicpp::ackhandler::packet *>, quicpp::base::error_t>
    determine_newly_acked_packets(quicpp::frame::ack *ack);
    quicpp::base::error_t on_packet_acked(quicpp::ackhandler::packet *p);
    quicpp::base::error_t
    detect_lost_packets(std::chrono::system_clock::time_point now,
                        uint64_t prior_inflight);
    std::chrono::system_clock::time_point &alarm_timeout();
    std::chrono::microseconds compute_handshake_timeout();
    std::chrono::microseconds compute_tlp_timeout();
    quicpp::base::error_t
    queue_packet_for_retransmission(quicpp::ackhandler::packet *p);
    quicpp::base::error_t
    queue_handshake_packets_for_retransmission();
    quicpp::base::error_t queue_rtos();
    quicpp::base::error_t on_alarm();
    void verify_rto(uint64_t pn);
    quicpp::base::error_t stop_retransmission_for(quicpp::ackhandler::packet *p);
    quicpp::ackhandler::packet *dequeue_packet_for_retransmission();
    size_t packet_number_len(uint64_t p) const;
    quicpp::ackhandler::send_mode_t send_mode();
    std::chrono::system_clock::time_point time_until_send();
    int should_send_num_packets();
    std::chrono::microseconds compute_rto_timeout();

    void gc_skipped_packets();
};

}
}
#endif
