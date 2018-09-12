#ifndef _QUICPP_BASE_WRITABLE_
#define _QUICPP_BASE_WRITABLE_

#include <cstdint>
#include <cstdlib>

namespace quicpp {
namespace base {

class writable {
public:
    virtual int write(uint8_t *buffer_ptr, size_t size) = 0;
};

}
}

#endif
