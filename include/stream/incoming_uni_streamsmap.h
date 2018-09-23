#ifndef _QUICPP_STREAM_INCOMING_UNI_STREAMS_MAP_
#define _QUICPP_STREAM_INCOMING_UNI_STREAMS_MAP_

#include "stream/receive_stream.h"
#include "frame/max_stream_id.h"
#include "base/stream_id_t.h"
#include "rw_mutex.h"
#include <condition_variable>
#include <map>
#include <functional>
#include <memory>

namespace quicpp {
namespace stream {

class incoming_uni_streamsmap {
private:
    quicpp::rw_mutex rw_mutex;
    std::condition_variable cond;

    std::map<quicpp::base::stream_id_t, std::shared_ptr<quicpp::stream::receive_stream>> streams;

    quicpp::base::stream_id_t next_stream;
    quicpp::base::stream_id_t highest_stream;
    quicpp::base::stream_id_t max_stream;
    int max_num_streams;

    std::function<std::shared_ptr<quicpp::stream::receive_stream> (quicpp::base::stream_id_t)> new_stream;
    std::function<void (std::shared_ptr<quicpp::frame::frame>)> _queue_max_stream_id;
    std::function<void (std::shared_ptr<quicpp::frame::max_stream_id> &)> queue_max_stream_id;

    quicpp::base::error_t close_err;

public:
    incoming_uni_streamsmap(quicpp::base::stream_id_t next_stream,
                            quicpp::base::stream_id_t initial_max_stream_id,
                            int max_num_streams,
                            std::function<void (std::shared_ptr<quicpp::frame::frame>)> queue_control_frame,
                            std::function<std::shared_ptr<quicpp::stream::receive_stream> (quicpp::base::stream_id_t)> new_stream);

    std::pair<std::shared_ptr<quicpp::stream::receive_stream>, quicpp::base::error_t>
    accept_stream();
    std::pair<std::shared_ptr<quicpp::stream::receive_stream>, quicpp::base::error_t>
    get_or_open_stream(quicpp::base::stream_id_t id);
    quicpp::base::error_t delete_stream(quicpp::base::stream_id_t id);
    void close_with_error(quicpp::base::error_t err);
};

}
}

#endif
