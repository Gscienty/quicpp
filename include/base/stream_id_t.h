#ifndef _QUICPP_BASE_STREAM_ID_T_
#define _QUICPP_BASE_STREAM_ID_T_

#include "base/varint.h"

namespace quicpp {
namespace base {

class stream_id_t : public quicpp::base::varint {
public:
    stream_id_t();
    stream_id_t(std::basic_istream<uint8_t> &buf);
    stream_id_t(uint64_t &&);
    stream_id_t(const uint64_t &);
    stream_id_t(const quicpp::base::varint &);

    bool bidirectional() const;
    bool client_initiated() const;

    stream_id_t & operator+= (const uint64_t val);
};

}
}

#endif
