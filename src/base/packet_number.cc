#include "base/packet_number.h"
#include <random>
#include <limits>

size_t
quicpp::base::get_packet_number_length_for_header(uint64_t packet_number,
                                                  uint64_t least_unacked) {
    uint64_t diff = packet_number - least_unacked;

    if (diff < (1<<(14 - 1))) {
        return 2;
    }

    return 4;
}

size_t quicpp::base::get_packet_number_length(uint64_t packet_number) {
    if (packet_number < (uint64_t(1) << (1 * 8))) {
        return 1;
    }
    if (packet_number < (uint64_t(1) << (2 * 8))) {
        return 2;
    }
    if (packet_number < (uint64_t(1) << (4 * 8))) {
        return 4;
    }
    return 6;
}

quicpp::base::packet_number_generator::
packet_number_generator(uint64_t initial,
                        uint64_t average_period)
    : average_period(average_period)
    , _next(initial)
    , next_to_skip(0) {}

uint64_t quicpp::base::packet_number_generator::peek() const {
    return this->_next;
}

uint64_t quicpp::base::packet_number_generator::pop() {
    uint64_t ret = this->_next;
    
    if (this->_next == this->next_to_skip) {
        this->_next++;
        this->generate_new_skip();
    }

    return ret;
}

quicpp::base::error_t quicpp::base::packet_number_generator::generate_new_skip() {
    uint64_t skip = uint64_t(this->get_random_number()) *
        (this->average_period - 1) /
        (std::numeric_limits<uint16_t>::max() / 2);

    this->next_to_skip = this->next_to_skip + 2 + skip;

    return quicpp::error::success;
}

uint16_t
quicpp::base::packet_number_generator::get_random_number() {
    return std::rand() % 0x10000;
}
