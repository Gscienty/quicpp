#ifndef _QUICPP_FRAME_RST_
#define _QUICPP_FRAME_RST_

#include "frame/type.h"
#include "base/stream_id_t.h"
#include "base/varint.h"

namespace quicpp {
namespace frame {
class rst : public quicpp::frame::frame {
private:
    quicpp::base::stream_id_t _stream_id;
    uint16_t _application_error_code;
    quicpp::base::varint _final_offset;
public:
    rst();
    rst(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    quicpp::base::stream_id_t &stream_id();
    uint16_t &application_error_code();
    quicpp::base::varint &final_offset();

    bool operator==(const quicpp::frame::rst &frame) const;
};
}
}

#endif

