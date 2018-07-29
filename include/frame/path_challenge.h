#ifndef _QUICPP_FRAME_PATH_CHALLENGE_
#define _QUICPP_FRAME_PATH_CHALLENGE_

#include "frame/type.h"

namespace quicpp {
namespace frame {
class path_challenge : public quicpp::frame::frame {
private:
    uint64_t _data;
public:
    path_challenge();
    path_challenge(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    uint64_t &data();
};
}
}

#endif
