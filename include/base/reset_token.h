#ifndef _QUICPP_BASE_RESET_TOKEN_
#define _QUICPP_BASE_RESET_TOKEN_

#include "encodable.h"
#include <stdint.h>
#include <algorithm>

namespace quicpp {
namespace base {
class reset_token : public encodable {
private:
    uint8_t token[16];
public:
    reset_token();
    reset_token(std::basic_istream<uint8_t> &in);
    template <typename InputIterator>
    reset_token(InputIterator begin, InputIterator end) {
        int byte_count = 0;
        for (auto iter = begin; iter != end; iter++, byte_count++) {
            if (byte_count == 16) {
                break;
            }

            token[byte_count] = *iter;
        }
        std::fill(token + byte_count, token + 16, 0x00);
    }
    virtual size_t size() const override;
    virtual void encode(std::basic_ostream<uint8_t> &out) const override;
};
}
}

#endif

