#ifndef _QUICPP_BASE_PACKET_NUMBER_
#define _QUICPP_BASE_PACKET_NUMBER_

#include <cstdlib>
#include <cstdint>

namespace quicpp {
namespace base {

size_t get_packet_number_length_for_header(uint64_t packet_number,
                                           uint64_t least_unacked);

size_t get_packet_number_length(uint64_t packet_number);

}
}

#endif
