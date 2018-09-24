#include "frame/ack.h"
#include "base/varint.h"
#include <algorithm>

quicpp::frame::ack::ack()
    : _delay(0) {}

quicpp::frame::ack::ack(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);

    quicpp::base::varint largest(in);
    this->_delay = std::chrono::microseconds(quicpp::base::varint(in));
    int len = uint64_t(quicpp::base::varint(in));

    quicpp::base::varint first(in);
    uint64_t top = uint64_t(largest) - uint64_t(first);
    this->_ranges.push_back(std::make_pair(top, uint64_t(largest)));

    while (len--) {
        quicpp::base::varint gap(in);
        quicpp::base::varint len(in);

        top -= uint64_t(gap);
        this->_ranges.push_back(std::make_pair(top - uint64_t(len), top));
        top -= uint64_t(len);
    }
}

uint8_t quicpp::frame::ack::type() const {
    return quicpp::frame::frame_type_ack;
}

size_t quicpp::frame::ack::size() const {
    size_t result = 1 +
        quicpp::base::varint(this->largest()).size() + 
        quicpp::base::varint(this->_delay.count()).size() +
        quicpp::base::varint(this->_ranges.size() - 1).size();
    
    auto begin = this->_ranges.begin();
    uint64_t largest = std::get<quicpp::frame::ack_range_largest>(*begin);
    uint64_t smallest = std::get<quicpp::frame::ack_range_smallest>(*begin);
    result += quicpp::base::varint(largest - smallest).size();

    begin++;
    std::for_each(begin, this->_ranges.end(),
                  [&] (const std::pair<uint64_t, uint64_t> &range) -> void {

                    uint64_t len = std::get<quicpp::frame::ack_range_largest>(range) - 
                        std::get<quicpp::frame::ack_range_smallest>(range);
                    uint64_t gap = smallest - std::get<quicpp::frame::ack_range_largest>(range);

                    largest = std::get<quicpp::frame::ack_range_largest>(range);
                    smallest = std::get<quicpp::frame::ack_range_smallest>(range);

                    result += quicpp::base::varint(gap).size();
                    result += quicpp::base::varint(len).size();

                  });

    return result;
}

void quicpp::frame::ack::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    quicpp::base::varint(this->largest()).encode(out);
    quicpp::base::varint(this->_delay.count()).encode(out);
    quicpp::base::varint(this->_ranges.size() - 1).encode(out);

    auto begin = this->_ranges.begin();
    uint64_t smallest = std::get<quicpp::frame::ack_range_smallest>(*begin);
    quicpp::base::varint(std::get<quicpp::frame::ack_range_largest>(*begin) - smallest).encode(out);

    begin++;
    std::for_each(begin, this->_ranges.end(),
                  [&] (const std::pair<uint64_t, uint64_t> &range) -> void {
                    
                    uint64_t len = std::get<quicpp::frame::ack_range_largest>(range) - 
                        std::get<quicpp::frame::ack_range_smallest>(range);
                    uint64_t gap = smallest - std::get<quicpp::frame::ack_range_largest>(range);

                    smallest = std::get<quicpp::frame::ack_range_smallest>(range);

                    quicpp::base::varint(gap).encode(out);
                    quicpp::base::varint(len).encode(out);

                  });
}

uint64_t quicpp::frame::ack::largest() const {
    return std::get<quicpp::frame::ack_range_largest>(*this->_ranges.begin());
}

uint64_t quicpp::frame::ack::smallest() const {
    return std::get<quicpp::frame::ack_range_smallest>(*this->_ranges.rbegin());
}

std::chrono::microseconds &quicpp::frame::ack::delay() {
    return this->_delay;
}

std::vector<std::pair<uint64_t, uint64_t>> &quicpp::frame::ack::ranges() {
    return this->_ranges;
}

bool quicpp::frame::ack::has_missing_ranges() const {
    return this->_ranges.size() > 1;
}

bool quicpp::frame::ack::acks_packet(uint64_t p) {
    if (p < this->smallest() || p > this->largest()) {
        return false;
    }

    int i = 0;
    int j = this->_ranges.size();

    std::function<bool (int)> search_func = [&] (int i) -> bool {
        return p >= std::get<quicpp::frame::ack_range_smallest>(this->_ranges[i]);
    };
    while (i < j) {
        int h = i + (j - i) / 2;
        if (!search_func(h)) {
            i = h + 1;
        }
        else {
            j = h;
        }
    }

    return p <= std::get<quicpp::frame::ack_range_largest>(this->_ranges[i]);
}

bool quicpp::frame::ack::operator==(const quicpp::frame::ack &frame) const {
    if (this->_delay != frame._delay) {
        return false;
    }
    if (this->_ranges.size() != frame._ranges.size()) {
        return false;
    }
    
    auto this_itr = this->_ranges.begin();
    auto other_itr = frame._ranges.begin();
    size_t size = this->_ranges.size();
    while (size--) {
        if (*this_itr != *other_itr) {
            return false;
        }
        this_itr++;
        other_itr++;
    }

    return true;
}
