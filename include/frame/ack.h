#ifndef _QUICPP_FRAME_ACK_
#define _QUICPP_FRAME_ACK_

#include "frame/type.h"
#include <chrono>
#include <utility>
#include <list>

namespace quicpp {
namespace frame {

const int ack_range_largest = 1;
const int ack_range_smallest = 0;

class ack : public quicpp::frame::frame {
private:
    std::chrono::duration<uint64_t, std::chrono::nanoseconds> delay;
    std::list<std::pair<uint64_t, uint64_t>> ranges;
public:
    ack(std::basic_istream<uint8_t> &in);

    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    uint64_t largest() const;
    uint64_t smallest() const;
};

}
}

#endif
