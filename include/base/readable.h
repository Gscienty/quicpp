#ifndef _QUICPP_BASE_READABLE_
#define _QUICPP_BASE_READABLE_

#include <cstdint>
#include <cstdlib>

namespace quicpp {
namespace base {

class readable {
public:
    virtual ssize_t read(uint8_t *buffer_ptr, size_t size) = 0;
};

}
}

#endif
