#ifndef _QUICPP_FRAME_STREAM_
#define _QUICPP_FRAME_STREAM_

#include "frame/type.h"
#include "base/stream_id_t.h"
#include "base/varint.h"
#include "base/error.h"
#include <string>
#include <memory>

namespace quicpp {
namespace frame {

class stream : public quicpp::frame::frame {
private:
    bool _offset_flag;
    bool _len_flag;
    bool _final_flag;
    quicpp::base::stream_id_t _stream_id;
    quicpp::base::varint _offset;
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
    std::basic_string<uint8_t> &data();
    uint64_t maxdata_len(uint64_t max_size);
    std::pair<std::shared_ptr<quicpp::frame::stream>, quicpp::base::error_t>
    maybe_split(uint64_t maxsize);
};

}
}

#endif
