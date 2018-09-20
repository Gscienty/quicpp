#ifndef _QUICPP_PACKET_STREAM_FRAME_SOURCE_
#define _QUICPP_PACKET_STREAM_FRAME_SOURCE_

#include "frame/stream.h"
#include <memory>
#include <cstdint>
#include <vector>

namespace quicpp {
namespace packet {

class stream_frame_source {
public:
    virtual bool has_crypto_stream_data() = 0;
    virtual
    std::shared_ptr<quicpp::frame::stream>
    pop_crypto_stream_frame(uint64_t bytes_count) = 0;
    virtual
    std::vector<std::shared_ptr<quicpp::frame::stream>>
    pop_stream_frames(uint64_t bytes_count) = 0;
};

}
}

#endif
