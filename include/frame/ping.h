#ifndef _QUICPP_FRAME_PING_
#define _QUICPP_FRAME_PING_

#include "frame/type.h"

namespace quicpp {
namespace frame {

class ping : public quicpp::frame::frame {
public:
    ping();
    ping(std::basic_istream<uint8_t> &in);
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    virtual uint8_t type() const override;
};

}
}

#endif
