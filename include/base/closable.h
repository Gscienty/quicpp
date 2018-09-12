#ifndef _QUICPP_BASE_CLOSABLE_
#define _QUICPP_BASE_CLOSABLE_

#include "base/error.h"

namespace quicpp {
namespace base {

class closable {
public:
    virtual quicpp::base::error_t close() = 0;
};

}
}

#endif
