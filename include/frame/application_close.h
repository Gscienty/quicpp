#ifndef _QUICPP_FRAME_APPLICATION_CLOSE_
#define _QUICPP_FRAME_APPLICATION_CLOSE_

#include "frame/type.h"
#include <string>

namespace quicpp {
namespace frame {
class application_close : public quicpp::frame::frame {
private:
    uint16_t _error_code;
    std::basic_string<uint8_t> _reason_phrase;
public:
    application_close();
    application_close(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    uint16_t &error();
    std::basic_string<uint8_t> &reason_phrase();
};
}
}

#endif

