#ifndef _QUICPP_FRAME_MAXIMUM_STREAM_ID_
#define _QUICPP_FRAME_MAXIMUM_STREAM_ID_

#include "frame/type.h"
#include "base/varint.h"

namespace quicpp {
namespace frame {
    class max_stream_id : public quicpp::frame::frame {
    private:
        quicpp::base::varint maximum_stream_id;
    public:
        max_stream_id(std::basic_istream<uint8_t> &in);
        virtual uint8_t get_type() const override;
        virtual size_t size() const override;
        virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    };
}
}

#endif

