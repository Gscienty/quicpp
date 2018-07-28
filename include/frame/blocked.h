#ifndef _QUICPP_FRAME_BLOCKED_
#define _QUICPP_FRAME_BLOCKED_

#include "frame/type.h"
#include "base/varint.h"

namespace quicpp {
namespace frame {
class blocked : public quicpp::frame::frame {
private:
    quicpp::base::varint offset;
public:
    blocked(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
};
}
}

#endif

