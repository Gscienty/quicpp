#ifndef _QUICPP_CRYPT_ENCRYPTION_LEVEL_
#define _QUICPP_CRYPT_ENCRYPTION_LEVEL_

#include <cstdint>

namespace quicpp {
namespace crypt {

const uint8_t encryption_unspecified = 0;
const uint8_t encryption_unencrypted = 1;
const uint8_t encryption_secure = 2;
const uint8_t encryption_forward_secure = 3;

}
}

#endif
