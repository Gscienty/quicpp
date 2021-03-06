#ifndef _QUICPP_FRAME_STREAM_ID_BLOCKED_
#define _QUICPP_FRAME_STREAM_ID_BLOCKED_

#include "frame/type.h"
#include "base/stream_id_t.h"

namespace quicpp {
namespace frame {
class stream_id_blocked : public quicpp::frame::frame {
private:
    quicpp::base::stream_id_t _stream_id; 
public:
    stream_id_blocked();
    stream_id_blocked(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    quicpp::base::stream_id_t &stream_id();
};
}
}

#endif

