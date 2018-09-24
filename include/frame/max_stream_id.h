#ifndef _QUICPP_FRAME_MAXIMUM_STREAM_ID_
#define _QUICPP_FRAME_MAXIMUM_STREAM_ID_

#include "frame/type.h"
#include "base/stream_id_t.h"

namespace quicpp {
namespace frame {
class max_stream_id : public quicpp::frame::frame {
private:
    quicpp::base::stream_id_t _maximum_stream_id;
public:
    max_stream_id();
    max_stream_id(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    quicpp::base::stream_id_t &maximum_stream_id();

    bool operator==(const quicpp::frame::max_stream_id &frame) const;
};
}
}

#endif

