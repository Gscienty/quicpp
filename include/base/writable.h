#ifndef _QUICPP_BASE_WRITABLE_
#define _QUICPP_BASE_WRITABLE_

#include "base/error.h"
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace quicpp {
namespace base {

class writable {
public:
    virtual
    std::pair<ssize_t, quicpp::base::error_t>
    write(uint8_t *buffer_ptr, size_t size) = 0;
};

}
}

#endif
