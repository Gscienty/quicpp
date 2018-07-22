#ifndef _QUICPP_BASE_RESET_TOKEN_
#define _QUICPP_BASE_RESET_TOKEN_

#include "encodable.h"
#include <stdint.h>

namespace quicpp {
namespace base {
    class reset_token : public encodable {
    private:
        uint8_t token[16];
    public:
        reset_token();
        reset_token(std::basic_istream<uint8_t> &in);
        virtual size_t size() const override;
        virtual void encode(std::basic_ostream<uint8_t> &out) const override;
    };
}
}

#endif

