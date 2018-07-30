#ifndef _QUICPP_FRAME_STREAM_
#define _QUICPP_FRAME_STREAM_

#include "frame/type.h"
#include "base/stream_id_t.h"
#include "base/varint.h"
#include <string>

namespace quicpp {
namespace frame {

class stream : public quicpp::frame::frame {
private:
    bool _offset_flag;
    bool _len_flag;
    bool _final_flag;
    quicpp::base::stream_id_t _stream_id;
    quicpp::base::varint _offset;
    quicpp::base::varint _len;
    std::basic_string<uint8_t> _data;
public:
    stream();
    stream(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    bool &offset_flag();
    bool &len_flag();
    bool &final_flag();
    quicpp::base::stream_id_t &stream_id();
    quicpp::base::varint &offset();
    quicpp::base::varint &len();
    std::basic_string<uint8_t> &data();
};

}
}

#endif
