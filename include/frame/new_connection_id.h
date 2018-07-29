#ifndef _QUICPP_FRAME_NEW_CONNECTION_ID_
#define _QUICPP_FRAME_NEW_CONNECTION_ID_

#include "frame/type.h"
#include "base/varint.h"
#include "base/conn_id.h"
#include "base/reset_token.h"

namespace quicpp {
namespace frame {
class new_connection_id : public quicpp::frame::frame {
private:
    quicpp::base::varint _sequence;
    quicpp::base::conn_id _conn_id;
    quicpp::base::reset_token _reset_token;
public:
    new_connection_id(std::basic_istream<uint8_t> &in);
    virtual uint8_t type() const override;
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;

    quicpp::base::varint &sequence();
    quicpp::base::conn_id &conn_id();
    quicpp::base::reset_token &reset_token();
};
}
}

#endif

