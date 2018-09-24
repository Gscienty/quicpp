#include "ackhandler/received_packet_history.h"
#include "params.h"
#include <iostream>
#include <algorithm>

quicpp::ackhandler::received_packet_history::received_packet_history()
    : _lowest_in_received_packet_numbers(0) {}

quicpp::base::error_t
quicpp::ackhandler::received_packet_history::
received_packet(uint64_t p) {
    if (this->_ranges.size() > quicpp::max_tracked_received_ack_ranges) {
        return quicpp::error::too_many_outstanding_received_ack_ranges;
    }
    
    if (this->_ranges.empty()) {
        this->_ranges.push_back(std::make_pair(p, p));
        return quicpp::error::success;
    }

    for (auto el = this->_ranges.rbegin(); el != this->_ranges.rend(); el++) {
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
            auto prev_itr = el;
            prev_itr++;
            if (prev_itr != this->_ranges.rend() &&
                std::get<quicpp::ackhandler::packet_interval_end>(*prev_itr) + 1 ==
                std::get<quicpp::ackhandler::packet_interval_start>(*el)) {
                std::get<quicpp::ackhandler::packet_interval_end>(*prev_itr) =
                    std::get<quicpp::ackhandler::packet_interval_end>(*el);
                this->_ranges.erase((++el).base());
                return quicpp::error::success;
            }
            return quicpp::error::success;
        }

        if (p > std::get<quicpp::ackhandler::packet_interval_end>(*el)) {
            this->_ranges.insert(el.base(), std::make_pair(p, p));
            return quicpp::error::success;
        }
    }

    this->_ranges.push_front(std::make_pair(p, p));
    return quicpp::error::success;
}

void quicpp::ackhandler::received_packet_history::delete_below(uint64_t p) {
    if (p <= this->_lowest_in_received_packet_numbers) {
        return;
    }
    this->_lowest_in_received_packet_numbers = p;

    auto next_elem = this->_ranges.begin();
    for (auto el = this->_ranges.begin();
         el != this->_ranges.end();
         el = next_elem) {
        next_elem = el;
        next_elem++;

        if (p > std::get<quicpp::ackhandler::packet_interval_start>(*el) &&
            p <= std::get<quicpp::ackhandler::packet_interval_end>(*el)) {
            std::get<quicpp::ackhandler::packet_interval_start>(*el) = p;
        }
        else if (std::get<quicpp::ackhandler::packet_interval_end>(*el) < p) {
            this->_ranges.erase(el);
        }
        else {
            return;
        }
    }
}

std::vector<std::pair<uint64_t, uint64_t>>
quicpp::ackhandler::received_packet_history::get_ack_ranges() {
    if (this->_ranges.empty()) {
        return std::vector<std::pair<uint64_t, uint64_t>>();
    }

    std::vector<std::pair<uint64_t, uint64_t>> result;
    for (auto el = this->_ranges.rbegin();
         el != this->_ranges.rend();
         el++) {
        result.push_back(*el);
    }

    return result;
}

std::pair<uint64_t, uint64_t>
quicpp::ackhandler::received_packet_history::get_highest_ack_range() {
    if (!this->_ranges.empty()) {
        return this->_ranges.back();
    }
    return std::make_pair(0UL, 0UL);
}


std::list<std::pair<uint64_t, uint64_t>> &
quicpp::ackhandler::received_packet_history::ranges() {
    return this->_ranges;
}

uint64_t &
quicpp::ackhandler::received_packet_history::lowest_in_received_packet_numbers() {
    return this->_lowest_in_received_packet_numbers;
}
