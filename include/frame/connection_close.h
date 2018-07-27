#ifndef _QUICPP_FRAME_CONNECTION_CLOSE_
#define _QUICPP_FRAME_CONNECTION_CLOSE_

#include "frame/type.h"
#include <string>

namespace quicpp {
namespace frame {
class connection_close : public quicpp::frame::frame {
private:
    uint16_t error_code;
    std::basic_string<uint8_t> reason_phrase;
public:
    connection_close(std::basic_istream<uint8_t> &in);
    virtual uint8_t get_type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
};
}
}

#endif

