#ifndef _QUICPP_FRAME_MAX_DATA_
#define _QUICPP_FRAME_MAX_DATA_

#include "frame/type.h"
#include "base/varint.h"

namespace quicpp {
namespace frame {
    class max_data : public quicpp::frame::frame {
    private:
        quicpp::base::varint maximum_data;
    public:
        max_data(std::basic_istream<uint8_t> &in);
        virtual uint8_t get_type() const override;
        virtual size_t size() const override;
        virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    };
}
}

#endif

