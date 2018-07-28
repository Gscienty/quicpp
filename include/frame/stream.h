#ifndef _QUICPP_FRAME_STREAM_
#define _QUICPP_FRAME_STREAM_

#include "frame/type.h"
#include "base/varint.h"
#include <string>

namespace quicpp {
namespace frame {

class stream : public quicpp::frame::frame {
private:
    bool offset_flag;
    bool len_flag;
    bool final_flag;
    quicpp::base::varint stream_id;
    quicpp::base::varint offset;
    quicpp::base::varint len;
    std::basic_string<uint8_t> data;
public:
    stream(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
};

}
}

#endif
