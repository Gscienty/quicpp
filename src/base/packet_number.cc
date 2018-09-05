#include "base/packet_number.h"

namespace quicpp {
namespace base {

size_t get_packet_number_length_for_header(uint64_t packet_number,
                                           uint64_t least_unacked) {
    uint64_t diff = packet_number - least_unacked;

    if (diff < (1<<(14 - 1))) {
        return 2;
    }

    return 4;
}

size_t get_packet_number_length(uint64_t packet_number) {
    if (packet_number < (uint64_t(1) << (1 * 8))) {
        return 1;
    }
    if (packet_number < (uint64_t(1) << (2 * 8))) {
        return 2;
    }
    if (packet_number < (uint64_t(1) << (4 * 8))) {
        return 4;
    }
    return 6;
}

}
}
