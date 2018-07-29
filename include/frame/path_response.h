#ifndef _QUICPP_FRAME_PATH_RESPONSE_
#define _QUICPP_FRAME_PATH_RESPONSE_

#include "frame/type.h"

namespace quicpp {
namespace frame {
class path_response : public quicpp::frame::frame {
private:
    uint64_t _data;
public:
    path_response();
    path_response(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    uint64_t &data();
};
}
}

#endif
