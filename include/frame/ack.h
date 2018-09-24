#ifndef _QUICPP_FRAME_ACK_
#define _QUICPP_FRAME_ACK_

#include "frame/type.h"
#include <chrono>
#include <utility>
#include <vector>

namespace quicpp {
namespace frame {

const int ack_range_largest = 1;
const int ack_range_smallest = 0;

class ack : public quicpp::frame::frame {
private:
    std::chrono::nanoseconds _delay;
    std::vector<std::pair<uint64_t, uint64_t>> _ranges;
public:
    ack();
    ack(std::basic_istream<uint8_t> &in);

    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    uint64_t largest() const;
    uint64_t smallest() const;

    std::chrono::nanoseconds &delay();
    std::vector<std::pair<uint64_t, uint64_t>> &ranges();

    bool has_missing_ranges() const;

    bool acks_packet(uint64_t p);
    
    bool operator==(const quicpp::frame::ack &frame) const;
};

}
}

#endif
