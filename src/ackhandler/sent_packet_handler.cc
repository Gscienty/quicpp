#include "ackhandler/sent_packet_handler.h"
#include "ackhandler/retransmittable.h"
#include "congestion/cubic_sender.h"
#include "crypt/encryption_level.h"
#include "frame/ack.h"
#include "base/packet_number.h"
#include "params.h"
#include <cmath>
#include <algorithm>

quicpp::ackhandler::sent_packet_handler::
sent_packet_handler(quicpp::congestion::rtt &rtt)
    : ls_pn(0)
    , ls_retransmittable_pt(std::chrono::system_clock::time_point::min())
    , ls_handshake_pt(std::chrono::system_clock::time_point::min())
    , next_packet_st(std::chrono::system_clock::time_point::min())
    , largest_acked(0)
    , largest_received_packet_with_ack(0)
    , _lowest_packet_not_confirmed_acked(0)
    , largest_sent_before_rto(0)
    , bytes_inflight(0)
    , congestion(new quicpp::congestion::cubic_sender(rtt,
                                                      quicpp::initial_cwnd,
                                                      quicpp::default_max_cwnd))
    , rtt(rtt)
    , handshake_complete(false)
    , handshake_count(0)
    , tlp_count(0)
    , allow_tlp(false)
    , rto_count(0)
    , num_rtos(0)
    , loss_time(std::chrono::system_clock::time_point::min())
    , alarm(std::chrono::system_clock::time_point::min()) {}

quicpp::ackhandler::sent_packet_handler::~sent_packet_handler() {
    delete this->congestion;
}

uint64_t quicpp::ackhandler::sent_packet_handler::lowest_unacked() const {
    auto p = this->packet_history.first_outstanding();
    if (p != nullptr) {
        return p->packet_number;
    }
    return this->largest_acked + 1;
}

void quicpp::ackhandler::sent_packet_handler::set_handshake_complete() {
    std::list<quicpp::ackhandler::packet *> queue;

    std::for_each(this->retransmission_queue.begin(),
                  this->retransmission_queue.end(),
                  [&] (quicpp::ackhandler::packet *p) -> void {
                    if (p->encryption_level == quicpp::crypt::encryption_forward_secure) {
                        queue.push_back(p);
                    }
                  });
    std::list<quicpp::ackhandler::packet *> handshake_packets;
    this->packet_history.iterate([&] (quicpp::ackhandler::packet *p)
                                 -> std::pair<bool, quicpp::base::error_t> {
                                     if (p->encryption_level != quicpp::crypt::encryption_forward_secure) {
                                        handshake_packets.push_back(p);
                                     }
                                     return std::make_pair(true, quicpp::error::success);
                                 });
    std::for_each(handshake_packets.begin(),
                  handshake_packets.end(),
                  [this] (quicpp::ackhandler::packet *p) -> void {
                      this->packet_history.remove(p->packet_number);
                  });

    this->retransmission_queue = queue;
    this->handshake_complete = true;
}

void
quicpp::ackhandler::sent_packet_handler::
sent_packet(quicpp::ackhandler::packet *p) {
    if (this->sent_packet_implement(p)) {
        this->packet_history.send_packet(p);
        this->update_loss_detection_alarm();
    }
}

void
quicpp::ackhandler::sent_packet_handler::
sent_packets_retransmission(std::vector<quicpp::ackhandler::packet *> packets,
                            uint64_t retransmission_of) {
    std::vector<quicpp::ackhandler::packet *> p;
    std::for_each(packets.begin(),
                  packets.end(),
                  [&, this] (quicpp::ackhandler::packet *packet) -> void {
                      if (this->sent_packet_implement(packet)) {
                        p.push_back(packet);
                      }
                  });
    this->packet_history
        .send_packets_as_retransmission(p.begin(),
                                        p.end(),
                                        retransmission_of);
    this->update_loss_detection_alarm();
}

bool
quicpp::ackhandler::sent_packet_handler::
sent_packet_implement(quicpp::ackhandler::packet *p) {
    for (uint64_t pn = this->ls_pn + 1; pn < p->packet_number; pn++) {
        this->skipped_packets.push_back(pn);
        if (this->skipped_packets.size() > quicpp::max_tracked_skipped_packets) {
            this->skipped_packets.erase(this->skipped_packets.begin());
        }
    }

    this->ls_pn = p->packet_number;

    if (!p->frames.empty()) {
        if (p->frames.front()->type() == quicpp::frame::frame_type_ack) {
            p->largest_acked = 
                dynamic_cast<quicpp::frame::ack *>(p->frames.front())->largest();
        }
    }

    p->frames = quicpp::ackhandler::strip_non_retransmittable_frames(p->frames);
    bool is_retransmittable = !p->frames.empty();

    if (is_retransmittable) {
        if (p->encryption_level < quicpp::crypt::encryption_forward_secure) {
            this->ls_handshake_pt = p->send_time;
        }
        this->ls_retransmittable_pt = p->send_time;
        p->included_in_bytes_inflight = true;
        this->bytes_inflight += p->len;
        p->can_be_retransmitted = true;
        if (this->num_rtos > 0) {
            this->num_rtos--;
        }
        this->allow_tlp = false;
    }
    this->congestion->on_packet_sent(p->packet_number,
                                     p->len,
                                     is_retransmittable);

    this->next_packet_st = std::max(this->next_packet_st, p->send_time) +
        this->congestion->time_until_send(this->bytes_inflight);

    return is_retransmittable;
}

quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::
received_ack(quicpp::frame::ack *ack,
             uint64_t with_packet_number,
             uint8_t encryption_level,
             std::chrono::system_clock::time_point rcv_time) {
    uint64_t largest_acked = ack->largest();
    if (largest_acked > this->ls_pn) {
        return quicpp::error::invalid_ack_data;
    }

    if (with_packet_number != 0 && with_packet_number <= this->largest_received_packet_with_ack) {
        return quicpp::error::success;
    }
    this->largest_received_packet_with_ack = with_packet_number;
    this->largest_acked = std::max(this->largest_acked, largest_acked);

    if (this->skipped_packets_acked(ack)) {
        return quicpp::error::invalid_ack_data;
    }

    if (this->maybe_update_rtt(largest_acked,
                               std::chrono::duration<std::chrono::milliseconds>(ack->delay()),
                               rcv_time)) {
        this->congestion->maybe_exit_slowstart();
    }

    std::vector<quicpp::ackhandler::packet *> acked_packets;
    quicpp::base::error_t err;
    std::tie(acked_packets, err) = this->determine_newly_acked_packets(ack);
    if (err != quicpp::error::success) {
        return err;
    }

    uint64_t prior_inflight = this->bytes_inflight;
    for (auto p = acked_packets.begin(); p != acked_packets.end(); p++) {
        if (encryption_level < (*p)->encryption_level) {
            return quicpp::error::encryption_level_not_equal;
        }
        if ((*p)->largest_acked != 0) {
            this->_lowest_packet_not_confirmed_acked = 
                std::max(this->_lowest_packet_not_confirmed_acked,
                (*p)->largest_acked + 1);
        }
        err = this->on_packet_acked((*p));
        if (err != quicpp::error::success) {
            return err;
        }
        if ((*p)->included_in_bytes_inflight) {
            this->congestion->on_packet_acked((*p)->packet_number,
                                              (*p)->len,
                                              prior_inflight,
                                              rcv_time);
        }
    }

    err = this->detect_lost_packets(rcv_time, prior_inflight);
    if (err != quicpp::error::success) {
        return err;
    }

    this->update_loss_detection_alarm();

    this->gc_skipped_packets();

    return quicpp::error::success;
}

uint64_t &
quicpp::ackhandler::sent_packet_handler::
lowest_packet_not_confirmed_acked() {
    return this->_lowest_packet_not_confirmed_acked;
}

std::pair<std::vector<quicpp::ackhandler::packet *>, quicpp::base::error_t>
quicpp::ackhandler::sent_packet_handler::
determine_newly_acked_packets(quicpp::frame::ack *ack) {
    std::vector<quicpp::ackhandler::packet *> acked_packets;
    int ack_range_index = 0;
    uint64_t lowest_acked = ack->smallest();
    uint64_t largest_acked = ack->largest();

    quicpp::base::error_t err = this->packet_history
        .iterate([&] (quicpp::ackhandler::packet *p) -> std::pair<bool, quicpp::base::error_t> {
                     if (p->packet_number < lowest_acked) {
                        return std::make_pair(true, quicpp::error::success);
                     }
                     if (p->packet_number > largest_acked) {
                        return std::make_pair(false, quicpp::error::success);
                     }

                     if (ack->has_missing_ranges()) {
                        auto range = *(ack->ranges().rbegin() + ack_range_index);

                        while (p->packet_number > std::get<quicpp::frame::ack_range_largest>(range) &&
                               ack_range_index < int(ack->ranges().size() - 1)) {
                            ack_range_index++;
                            range = *(ack->ranges().rbegin() + ack_range_index);
                        }

                        if (p->packet_number >= std::get<quicpp::frame::ack_range_smallest>(range)) {
                            return std::make_pair(false, quicpp::error::bug);
                        }
                        acked_packets.push_back(p);
                     }
                     else {
                         acked_packets.push_back(p);
                     }

                     return std::make_pair(true, quicpp::error::success);
                 });
    return std::make_pair(acked_packets, err);
}

bool 
quicpp::ackhandler::sent_packet_handler::
maybe_update_rtt(uint64_t largest_acked,
                 std::chrono::microseconds ack_delay,
                 std::chrono::system_clock::time_point rcv_time) {
    auto p = this->packet_history.get_packet(largest_acked);
    if (p != nullptr) {
        using namespace std::chrono;
        this->rtt.update(duration_cast<microseconds>(rcv_time - p->send_time),
                         ack_delay,
                         rcv_time);
        return true;
    }
    return false;
}

void 
quicpp::ackhandler::sent_packet_handler::
update_loss_detection_alarm() {
    if (!this->packet_history.has_outstanding_packets()) {
        this->alarm = std::chrono::system_clock::time_point::min();
        return;
    }

    if (this->packet_history.has_outstanding_handshake_packets()) {
        this->alarm = this->ls_handshake_pt + this->compute_handshake_timeout();
    }
    else if (this->loss_time != std::chrono::system_clock::time_point::min()) {
        this->alarm = this->loss_time;
    }
    else {
        auto alarm_duration = this->compute_rto_timeout();
        if (this->tlp_count < quicpp::ackhandler::max_tlps) {
            auto tlp_alarm = this->compute_tlp_timeout();
            alarm_duration = std::max(alarm_duration, tlp_alarm);
        }
        this->alarm = this->ls_retransmittable_pt + alarm_duration;
    }
}

quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::
detect_lost_packets(std::chrono::system_clock::time_point now,
                    uint64_t prior_inflight) {
    this->loss_time = std::chrono::system_clock::time_point::min();

    using namespace std::chrono;
    double max_rtt = 
        duration_cast<nanoseconds>(std::max(this->rtt.latest(),
                                            this->rtt.smoothed()))
        .count();

    nanoseconds delay_until_lost(uint64_t((1.0 + quicpp::ackhandler::time_reordering_fraction) * max_rtt));

    std::vector<quicpp::ackhandler::packet *> lost_packets;
    this->packet_history
        .iterate([&, this] (quicpp::ackhandler::packet *p) -> std::pair<bool, quicpp::base::error_t> {
                     if (p->packet_number > this->largest_acked) {
                        return std::make_pair(false, quicpp::error::success);
                     }

                     auto time_since_sent = now - p->send_time;
                     if (time_since_sent > delay_until_lost) {
                        this->loss_time = now + (delay_until_lost - time_since_sent);
                     }
                     return std::make_pair(true, quicpp::error::success);
                 });

    for (auto p = lost_packets.begin(); p != lost_packets.end(); p++) {
        if ((*p)->included_in_bytes_inflight) {
            this->bytes_inflight -= (*p)->len;
            this->congestion->on_packet_lost((*p)->packet_number,
                                             (*p)->len,
                                             prior_inflight);
        }
        if ((*p)->can_be_retransmitted) {
            quicpp::base::error_t err = 
                this->queue_packet_for_retransmission(*p);
            if (err != quicpp::error::success) {
                return err;
            }
        }
        this->packet_history.remove((*p)->packet_number);
    }
    return quicpp::error::success;
}

quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::on_alarm() {
    using namespace std::chrono;

    system_clock::time_point now = system_clock::now();

    quicpp::base::error_t err;
    if (this->packet_history.has_outstanding_handshake_packets()) {
        this->handshake_count++;
        err = this->queue_handshake_packets_for_retransmission();
    }
    else if (this->loss_time != system_clock::time_point::min()) {
        err = this->detect_lost_packets(now, this->bytes_inflight);
    }
    else if (this->tlp_count < quicpp::ackhandler::max_tlps) {
        this->allow_tlp = true;
        this->tlp_count++;
    }
    else {
        this->rto_count++;
        this->num_rtos += 2;
        err = this->queue_rtos();
    }

    if (err != quicpp::error::success) {
        return err;
    }

    this->update_loss_detection_alarm();
    return quicpp::error::success;
}

std::chrono::system_clock::time_point &
quicpp::ackhandler::sent_packet_handler::alarm_timeout() {
    return this->alarm;
}

quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::
on_packet_acked(quicpp::ackhandler::packet *p) {
    if (this->packet_history.get_packet(p->packet_number) == nullptr) {
        return quicpp::error::success;
    }

    if (p->is_retransmission) {
        auto parent = this->packet_history.get_packet(p->retransmitted_of);
        if (parent != nullptr) {
            if (parent->retransmitted_as.size() == 1) {
                parent->retransmitted_as.clear();
            }
            else {
                std::vector<uint64_t> retransmitted_as;
                for (auto pn = parent->retransmitted_as.begin();
                     pn != parent->retransmitted_as.end();
                     pn++) {
                    if (*pn != p->packet_number) {
                        retransmitted_as.push_back(*pn);
                    }
                }
                parent->retransmitted_as = retransmitted_as;
            }
        }
    }

    if (p->included_in_bytes_inflight) {
        this->bytes_inflight -= p->len;
    }
    if (this->rto_count > 0) {
        this->verify_rto(p->packet_number);
    }

    quicpp::base::error_t err = this->stop_retransmission_for(p);
    if (err != quicpp::error::success) {
        return err;
    }

    this->rto_count = 0;
    this->tlp_count = 0;
    this->handshake_count = 0;
    return this->packet_history.remove(p->packet_number);
}

quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::
stop_retransmission_for(quicpp::ackhandler::packet *p) {
    quicpp::base::error_t err = 
        this->packet_history.mark_cannot_be_retransmitted(p->packet_number);
    if (err != quicpp::error::success) {
        return err;
    }

    for (auto r = p->retransmitted_as.begin();
         r != p->retransmitted_as.end();
         r++) {
        auto packet = this->packet_history.get_packet(*r);
        if (packet == nullptr) {
            return quicpp::error::bug;
        }
        this->stop_retransmission_for(packet);
    }

    return quicpp::error::success;
}

void quicpp::ackhandler::sent_packet_handler::verify_rto(uint64_t pn) {
    if (pn <= this->largest_sent_before_rto) {
        this->rtt.expire_smoothed_metrics();
        return;
    }
    this->congestion->on_retransmission_timeout(true);
}

quicpp::ackhandler::packet *
quicpp::ackhandler::sent_packet_handler::dequeue_packet_for_retransmission() {
    if (this->retransmission_queue.empty()) {
        return nullptr;
    }

    auto packet = this->retransmission_queue.front();
    this->retransmission_queue.pop_front();
    return packet;
}


size_t
quicpp::ackhandler::sent_packet_handler::
packet_number_len(uint64_t p) const {
    return quicpp::base::
        get_packet_number_length_for_header(p,
                                            this->lowest_unacked());
}

quicpp::ackhandler::send_mode_t
quicpp::ackhandler::sent_packet_handler::
send_mode() {
    size_t num_tracked_packets =
        this->retransmission_queue.size() + this->packet_history.size();

    if (num_tracked_packets >= quicpp::max_tracked_skipped_packets) {
        return quicpp::send_mode::send_none;
    }
    if (this->allow_tlp) {
        return quicpp::send_mode::send_tlp;
    }
    if (this->num_rtos > 0) {
        return quicpp::send_mode::send_rto;
    }
    
    if (this->bytes_inflight > this->congestion->cwnd()) {
        return quicpp::send_mode::send_ack;
    }
    if (!this->retransmission_queue.empty()) {
        return quicpp::send_mode::send_retransmission;
    }
    if (num_tracked_packets >= quicpp::max_outstanding_sent_packets) {
        return quicpp::send_mode::send_ack;
    }
    return quicpp::send_mode::send_any;
}

std::chrono::system_clock::time_point
quicpp::ackhandler::sent_packet_handler::time_until_send() {
    return this->next_packet_st;
}

int quicpp::ackhandler::sent_packet_handler::should_send_num_packets() {
    if (this->num_rtos > 0) { return this->num_rtos; }
    std::chrono::microseconds delay =
        this->congestion->time_until_send(this->bytes_inflight);
    if (delay == std::chrono::microseconds::zero() ||
        delay > quicpp::min_pacing_delay) { return 1; }

    return int(std::ceil(double(quicpp::min_pacing_delay.count()) / 
                         double(delay.count())));
}



quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::queue_rtos() {
    this->largest_sent_before_rto = this->ls_pn;

    for (int i = 0; i < 2; i++) {
        auto p = this->packet_history.first_outstanding();
        if (p != nullptr) {
            quicpp::base::error_t err =
                this->queue_packet_for_retransmission(*p);
            if (err == quicpp::error::success) {
                return err;
            }
        }
    }
    return quicpp::error::success;
}

quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::
queue_handshake_packets_for_retransmission() {
    std::vector<quicpp::ackhandler::packet *> handshake_packets;
    this->packet_history
        .iterate([&] (quicpp::ackhandler::packet *p)
                 -> std::pair<bool, quicpp::base::error_t> {
                    if (p->can_be_retransmitted &&
                        p->encryption_level < quicpp::crypt::encryption_forward_secure) {
                        handshake_packets.push_back(p);
                    }
                    return std::make_pair(true, quicpp::error::success);
        });

    for (auto p_itr = handshake_packets.begin();
         p_itr != handshake_packets.end();
         p_itr++) {
        quicpp::base::error_t err =
            this->queue_packet_for_retransmission(*p_itr);

        if (err != quicpp::error::success) {
            return err;
        }
    }

    return quicpp::error::success;
}

quicpp::base::error_t
quicpp::ackhandler::sent_packet_handler::
queue_packet_for_retransmission(quicpp::ackhandler::packet *p) {
    if (!p->can_be_retransmitted) {
        return quicpp::error::bug;
    }
    quicpp::base::error_t err =
        this->packet_history.mark_cannot_be_retransmitted(p->packet_number);
    if (err != quicpp::error::success) {
        return err;
    }

    this->retransmission_queue.push_back(p);
    return quicpp::error::success;
}

std::chrono::microseconds
quicpp::ackhandler::sent_packet_handler::compute_handshake_timeout() {
    std::chrono::microseconds duration = 
        std::max(2 * this->rtt.smoothed_or_initial(), 
                 std::chrono::microseconds(quicpp::ackhandler::min_tlp_timeout));

    return std::chrono::microseconds(duration.count() << this->handshake_count);
}

std::chrono::microseconds 
quicpp::ackhandler::sent_packet_handler::compute_tlp_timeout() {
    return std::max(this->rtt.smoothed_or_initial() * 3 / 2,
                    std::chrono::microseconds(quicpp::ackhandler::min_tlp_timeout));
}

std::chrono::microseconds
quicpp::ackhandler::sent_packet_handler::compute_rto_timeout() {
    std::chrono::microseconds rto = std::chrono::microseconds::zero();

    auto rtt = this->rtt.smoothed();
    if (rtt == std::chrono::microseconds::zero()) {
        rto = quicpp::ackhandler::default_rto_timeout;
    }
    else {
        rto = rtt + 4 * this->rtt.mean_deviation();
    }
    rto = std::max(rto,
                   std::chrono::microseconds(quicpp::ackhandler::min_rto_timeout));
    rto = std::chrono::microseconds(rto.count() << this->rto_count);
    return std::min(rto,
                    std::chrono::microseconds(quicpp::ackhandler::max_rto_timeout));
}


bool
quicpp::ackhandler::sent_packet_handler::
skipped_packets_acked(quicpp::frame::ack *ack) {
    for (auto p_itr = this->skipped_packets.begin();
         p_itr != this->skipped_packets.end();
         p_itr++) {
        if (ack->acks_packet(*p_itr)) {
            return true;
        }
    }
    return false;
}

void
quicpp::ackhandler::sent_packet_handler::
gc_skipped_packets() {
    uint64_t lowest_unacked = this->lowest_unacked();
    uint64_t delete_index = 0;

    for (auto p_itr = this->skipped_packets.begin();
         p_itr != this->skipped_packets.end();
         p_itr++) {
        if (*p_itr < lowest_unacked) {
            delete_index = p_itr - this->skipped_packets.begin() + 1;
        }
    }

    this->skipped_packets.erase(this->skipped_packets.begin(),
                                 this->skipped_packets.begin() + delete_index);
}
