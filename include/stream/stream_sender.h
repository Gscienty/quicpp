#ifndef _QUICPP_STREAM_STREAM_SENDER_
#define _QUICPP_STREAM_STREAM_SENDER_

#include "frame/type.h"
#include "base/stream_id_t.h"
#include <functional>

namespace quicpp {
namespace stream {

class stream_sender {
public:
    virtual ~stream_sender() {}
    virtual void queue_control_frame(quicpp::frame::frame *frame) = 0;
    virtual void on_has_stream_data(quicpp::base::stream_id_t stream_id) = 0;
    virtual void on_stream_completed(quicpp::base::stream_id_t stream_id) = 0;
};

class uni_stream_sender : public quicpp::stream::stream_sender {
private:
    quicpp::stream::stream_sender &sender;
    std::function<void ()> on_stream_completed_implement;
public:
    uni_stream_sender(quicpp::stream::stream_sender &sender,
                      std::function<void ()> func)
        : sender(sender)
        , on_stream_completed_implement(func) {}

    virtual void queue_control_frame(quicpp::frame::frame *frame) override {
        this->sender.queue_control_frame(frame);
    }

    virtual void on_has_stream_data(quicpp::base::stream_id_t stream_id) override {
        this->sender.on_has_stream_data(stream_id);
    }

    virtual void on_stream_completed(quicpp::base::stream_id_t) override {
        this->on_stream_completed_implement();
    }
};

}
}

#endif
