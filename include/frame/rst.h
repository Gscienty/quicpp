#ifndef _QUICPP_FRAME_RST_
#define _QUICPP_FRAME_RST_

#include "base/varint.h"
#include "encodable.h"

namespace quicpp {
namespace frame {
    class rst : public quicpp::encodable {
    private:
        quicpp::base::varint stream_id;
        uint16_t application_error_code;
        quicpp::base::varint final_offset;
    public:
        rst(std::basic_istream<uint8_t> &in);
        virtual size_t size() const override;
        virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    };
}
}

#endif

