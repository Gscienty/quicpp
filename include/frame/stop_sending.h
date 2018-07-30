#ifndef _QUICPP_FRAME_STOP_SENDING_
#define _QUICPP_FRAME_STOP_SENDING_

#include "frame/type.h"
#include "base/stream_id_t.h"

namespace quicpp {
namespace frame {
class stop_sending : public quicpp::frame::frame {
private:
    quicpp::base::stream_id_t _stream_id;
    uint16_t _application_error_code;
public:
    stop_sending();
    stop_sending(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    quicpp::base::stream_id_t &stream_id();
    uint16_t &application_error_code();
};
}
}

#endif
