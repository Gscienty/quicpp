#ifndef _QUICPP_STREAM_INCOMING_BIDI_STREAMS_MAP_
#define _QUICPP_STREAM_INCOMING_BIDI_STREAMS_MAP_

#include "stream/stream.h"
#include "base/stream_id_t.h"
#include "base/error.h"
#include "frame/max_stream_id.h"
#include "rw_mutex.h"
#include <map>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace quicpp {
namespace stream {

class incoming_bidi_streamsmap {
private:
    quicpp::rw_mutex rw_mutex;
    std::condition_variable cond;
    std::map<quicpp::base::stream_id_t, std::shared_ptr<quicpp::stream::stream>> streams;
    quicpp::base::stream_id_t next_stream;
    quicpp::base::stream_id_t highest_stream;
    quicpp::base::stream_id_t max_stream;
    int max_num_streams;
    std::function<std::shared_ptr<quicpp::stream::stream> (quicpp::base::stream_id_t)> new_stream;
    std::function<void (std::shared_ptr<quicpp::frame::frame>)> _queue_max_stream_id;
    std::function<void (std::shared_ptr<quicpp::frame::max_stream_id> &)> queue_max_stream_id;
    quicpp::base::error_t close_err;
public:
    incoming_bidi_streamsmap(quicpp::base::stream_id_t next_stream,
                             quicpp::base::stream_id_t initial_max_stream_id,
                             int max_num_streams,
                             std::function<std::shared_ptr<quicpp::stream::stream> (quicpp::base::stream_id_t)> new_stream,
                             std::function<void (std::shared_ptr<quicpp::frame::frame>)> queue_max_stream_id);
    std::pair<std::shared_ptr<quicpp::stream::stream>, quicpp::base::error_t> accept_stream();
    std::pair<std::shared_ptr<quicpp::stream::stream>, quicpp::base::error_t>
    get_or_open_stream(quicpp::base::stream_id_t id);
    quicpp::base::error_t delete_stream(quicpp::base::stream_id_t id);
    void close_with_error(quicpp::base::error_t err);
};

}
}

#endif
