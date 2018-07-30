#ifndef _QUICPP_FRAME_STREAM_BLOCKED_
#define _QUICPP_FRAME_STREAM_BLOCKED_

#include "frame/type.h"
#include "base/stream_id_t.h"
#include "base/varint.h"

namespace quicpp {
namespace frame {
class stream_blocked : public quicpp::frame::frame {
private:
    quicpp::base::stream_id_t _stream_id;
    quicpp::base::varint _offset;
public:
    stream_blocked();
    stream_blocked(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    quicpp::base::stream_id_t &stream_id();
    quicpp::base::varint &offset();
};
}
}

#endif

