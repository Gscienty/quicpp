#ifndef _QUICPP_FRAME_CONNECTION_CLOSE_
#define _QUICPP_FRAME_CONNECTION_CLOSE_

#include "frame/type.h"
#include <string>

namespace quicpp {
namespace frame {
class connection_close : public quicpp::frame::frame {
private:
    uint16_t _error_code;
    std::string _reason_phrase;
public:
    connection_close();
    connection_close(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    uint16_t &error();
    std::string &reason_phrase();

    bool operator== (const quicpp::frame::connection_close &) const;
};
}
}

#endif

