#ifndef _QUICPP_FRAME_STOP_SENDING_
#define _QUICPP_FRAME_STOP_SENDING_

#include "frame/type.h"
#include "base/varint.h"

namespace quicpp {
namespace frame {
class stop_sending : public quicpp::frame::frame {
private:
    quicpp::base::varint stream_id;
    uint16_t application_error_code;
public:
    stop_sending(std::basic_istream<uint8_t> &in);
    virtual uint8_t get_type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
};
}
}

#endif
