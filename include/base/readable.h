#ifndef _QUICPP_BASE_READABLE_
#define _QUICPP_BASE_READABLE_

#include "base/error.h"
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace quicpp {
namespace base {

class readable {
public:
    virtual
    std::pair<ssize_t, quicpp::base::error_t>
    read(uint8_t *buffer_ptr, size_t size) = 0;
};

}
}

#endif
