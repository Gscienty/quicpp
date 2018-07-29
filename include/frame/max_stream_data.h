#ifndef _QUICPP_FRAME_MAX_SRTEAM_DATA_
#define _QUICPP_FRAME_MAX_SRTEAM_DATA_

#include "frame/type.h"
#include "base/varint.h"

namespace quicpp {
namespace frame {
class max_stream_data : public quicpp::frame::frame {
private:
    quicpp::base::varint _stream_id;
    quicpp::base::varint _maximum_stream_data;
public:
    max_stream_data();
    max_stream_data(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    quicpp::base::varint &stream_id();
    quicpp::base::varint &maximum_stream_data();
};
}
}

#endif

