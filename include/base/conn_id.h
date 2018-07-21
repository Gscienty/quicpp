#ifndef _QUICPP_BASE_CONN_ID_
#define _QUICPP_BASE_CONN_ID_

#include "encodable.h"
#include <string>
#include <istream>

namespace quicpp {
namespace base {
    class conn_id : public quicpp::encodable {
    private:
        std::basic_string<uint8_t> id;
    public:
        conn_id();
        conn_id(std::basic_string<uint8_t> &&);
        conn_id(std::basic_istream<uint8_t> &in, const size_t len);
        virtual size_t size() const override;
        virtual void encode(std::basic_ostream<uint8_t> &out) const override;
        bool operator==(const conn_id &other_id) const;
    };
}
}

#endif

