#ifndef _QUICPP_BASE_ERROR_
#define _QUICPP_BASE_ERROR_

#include <cstdint>

namespace quicpp {
namespace base {

class error_t {
private:
    uint64_t sign;
public:
    error_t(uint64_t sign) : sign(sign) {}

    bool operator== (const quicpp::base::error_t &err) {
        return this->sign == err.sign;
    }

    bool operator!= (const quicpp::base::error_t &err) {
        return this->sign != err.sign;
    }
};
}
}

namespace quicpp {
namespace error {

const quicpp::base::error_t success(0);
const quicpp::base::error_t flowcontrol_recv_too_much_data(1000);

}
}

#endif
