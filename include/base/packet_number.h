#ifndef _QUICPP_BASE_PACKET_NUMBER_
#define _QUICPP_BASE_PACKET_NUMBER_

#include "base/error.h"
#include <cstdlib>
#include <cstdint>
#include <utility>

namespace quicpp {
namespace base {

size_t get_packet_number_length_for_header(uint64_t packet_number,
                                           uint64_t least_unacked);

size_t get_packet_number_length(uint64_t packet_number);

class packet_number_generator {
private:
    uint64_t average_period;
    uint64_t _next;
    uint64_t next_to_skip;
public:
    packet_number_generator(uint64_t initial, uint64_t average_period);
    uint64_t peek() const;
    uint64_t pop();
    quicpp::base::error_t generate_new_skip();
    uint16_t get_random_number();
};

}
}

#endif
