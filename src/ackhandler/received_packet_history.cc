#include "ackhandler/received_packet_history.h"
#include "params.h"

quicpp::base::error_t
quicpp::ackhandler::received_packet_history::
received_packet(uint64_t p) {
    if (this->ranges.size() > quicpp::max_tracked_received_ack_ranges) {
        return quicpp::error::too_many_outstanding_received_ack_ranges;
    }
    
    if (this->ranges.empty()) {
        this->ranges.push_back(std::make_pair(p, p));
        return quicpp::error::success;
    }

    for (auto el = this->ranges.rbegin(); el != this->ranges.rend(); el++) {
        if (p >= std::get<quicpp::ackhandler::packet_interval_start>(*el) &&
            p <= std::get<quicpp::ackhandler::packet_interval_end>(*el)) {
            return quicpp::error::success;
        }

        bool range_extended = false;
        if (std::get<quicpp::ackhandler::packet_interval_end>(*el) == p - 1) {
            range_extended = true;
            std::get<quicpp::ackhandler::packet_interval_end>(*el) = p;
        }
        else if (std::get<quicpp::ackhandler::packet_interval_start>(*el) == p + 1) {
            range_extended = true;
            std::get<quicpp::ackhandler::packet_interval_start>(*el) = p;
        }

        if (range_extended) {
            auto prev_itr = el + 1;
            if (prev_itr != this->ranges.rend() &&
                std::get<quicpp::ackhandler::packet_interval_end>(*prev_itr) + 1 ==
                std::get<quicpp::ackhandler::packet_interval_start>(*el)) {
                std::get<quicpp::ackhandler::packet_interval_end>(*prev_itr) =
                    std::get<quicpp::ackhandler::packet_interval_end>(*el);
                this->ranges.erase(el.base());
                return quicpp::error::success;
            }
            return quicpp::error::success;
        }

        if (p > std::get<quicpp::ackhandler::packet_interval_end>(*el)) {
            this->ranges.insert((el - 1).base(), std::make_pair(p, p));
            return quicpp::error::success;
        }
    }

    this->ranges.insert(this->ranges.begin(), std::make_pair(p, p));
    return quicpp::error::success;
}
