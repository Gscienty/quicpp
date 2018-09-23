#ifndef _QUICPP_STREAM_STREAM_FRAME_SORTER_
#define _QUICPP_STREAM_STREAM_FRAME_SORTER_

#include "frame/stream.h"
#include "base/error.h"
#include <cstdint>
#include <map>
#include <list>
#include <utility>
#include <memory>

namespace quicpp {
namespace stream {

class stream_frame_sorter {
private:
    std::map<uint64_t, std::shared_ptr<quicpp::frame::stream>> queued_frames;
    uint64_t _read_position;
    std::list<std::pair<uint64_t, uint64_t>> gaps;
public:
    stream_frame_sorter();
    quicpp::base::error_t push(std::shared_ptr<quicpp::frame::stream> &frame);
    std::shared_ptr<quicpp::frame::stream> pop();
    std::shared_ptr<quicpp::frame::stream> head();
    uint64_t &read_position();
};

}
}

#endif
