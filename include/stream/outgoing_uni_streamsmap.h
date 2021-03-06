#ifndef _QUICPP_STREAM_OUTGOING_UNI_STREAMS_MAP_
#define _QUICPP_STREAM_OUTGOING_UNI_STREAMS_MAP_

#include "stream/send_stream.h"
#include "frame/stream_id_blocked.h"
#include "base/stream_id_t.h"
#include "base/error.h"
#include "rw_mutex.h"
#include <condition_variable>
#include <map>
#include <functional>
#include <memory>

namespace quicpp {
namespace stream  {

class outgoing_uni_streamsmap {
private:
    quicpp::rw_mutex rw_mutex;
    std::condition_variable cond;

    std::map<quicpp::base::stream_id_t, std::shared_ptr<quicpp::stream::send_stream>> streams;

    quicpp::base::stream_id_t next_stream;
    quicpp::base::stream_id_t max_stream;
    quicpp::base::stream_id_t highest_stream;

    std::function<std::shared_ptr<quicpp::stream::send_stream> (quicpp::base::stream_id_t)> new_stream;
    std::function<void (std::shared_ptr<quicpp::frame::frame>)> _queue_stream_id_blocked;
    std::function<void (std::shared_ptr<quicpp::frame::stream_id_blocked>)> queue_stream_id_blocked;

    quicpp::base::error_t close_err;

public:
    outgoing_uni_streamsmap(quicpp::base::stream_id_t next_stream,
                            std::function<void (std::shared_ptr<quicpp::frame::frame>)> queue_control_frame,
                            std::function<std::shared_ptr<quicpp::stream::send_stream> (quicpp::base::stream_id_t)> new_stream);

    std::pair<std::shared_ptr<quicpp::stream::send_stream>, quicpp::base::error_t>
    open_stream_implement();
    std::pair<std::shared_ptr<quicpp::stream::send_stream>, quicpp::base::error_t>
    open_stream();
    std::pair<std::shared_ptr<quicpp::stream::send_stream>, quicpp::base::error_t>
    open_stream_sync();
    std::pair<std::shared_ptr<quicpp::stream::send_stream>, quicpp::base::error_t>
    get_stream(quicpp::base::stream_id_t id);
    quicpp::base::error_t delete_stream(quicpp::base::stream_id_t id);
    void set_max_stream(quicpp::base::stream_id_t id);
    void close_with_error(quicpp::base::error_t err);

};

}
}

#endif
