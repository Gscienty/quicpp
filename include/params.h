#ifndef _QUICPP_PARAMS_
#define _QUICPP_PARAMS_

#include <cstdint>

namespace quicpp {

const double window_update_threshole = 0.25;
const double connection_flowcontrol_multiplier = 1.5;
const uint64_t default_tcp_mss = 1460;

}

#endif
