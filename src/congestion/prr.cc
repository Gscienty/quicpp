#include "congestion/prr.h"
#include "params.h"

void quicpp::congestion::prr::on_packet_send(const uint64_t sent_bytes) {
    this->sent_since_loss += sent_bytes;
}

void quicpp::congestion::prr::on_packet_lost(const uint64_t prior_inflight) {
    this->sent_since_loss = 0;
    this->inflight_since_loss = prior_inflight;
    this->delivered_since_loss = 0;
    this->ack_count_since_loss = 0;
}

void quicpp::congestion::prr::on_packet_acked(const uint64_t acked_bytes) {
    this->delivered_since_loss += acked_bytes;
    this->ack_count_since_loss++;
}

bool quicpp::congestion::prr::can_send(const uint64_t cwnd,
                                       const uint64_t inflight,
                                       const uint64_t slowstart_threhold) {
    if (this->sent_since_loss == 0 || inflight < quicpp::default_tcp_mss) {
        return true;
    }
    if (cwnd > inflight) {
        return this->delivered_since_loss +
            this->ack_count_since_loss *
            quicpp::default_tcp_mss > this->sent_since_loss;
    }

    return this->delivered_since_loss * slowstart_threhold > 
        this->sent_since_loss * this->inflight_since_loss;
}

void quicpp::congestion::prr::init() {
    this->sent_since_loss = 0;
    this->delivered_since_loss = 0;
    this->ack_count_since_loss = 0;
    this->inflight_since_loss = 0;
}
