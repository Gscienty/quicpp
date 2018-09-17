#ifndef _QUICPP_STREAM_STREAM_SENDER_
#define _QUICPP_STREAM_STREAM_SENDER_

#include "frame/type.h"
#include "base/stream_id_t.h"

namespace quicpp {
namespace stream {

class stream_sender {
public:
    virtual void queue_control_frame(quicpp::frame::frame *frame) = 0;
    virtual void on_has_stream_data(quicpp::base::stream_id_t stream_id) = 0;
    virtual void on_stream_completed(quicpp::base::stream_id_t stream_id) = 0;
};

}
}

#endif
