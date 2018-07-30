#ifndef _QUICPP_BASE_ERROR_
#define _QUICPP_BASE_ERROR_

#include <cstdint>

namespace quicpp {
namespace base {

class error_t {
private:
    uint64_t sign;
public:
    error(uint64_t sign) : sign(sign) {}

    bool operator== (const quicpp::base::error_t &err) {
        return this->sign == err.sign;
    }

    bool operator!= (const quicpp::base::error_t &err) {
        return this->sign != err.sign;
    }
};

}
}

#endif
