#ifndef _QUICPP_STREAM_STREAM_
#define _QUICPP_STREAM_STREAM_

#include "stream/send_stream.h"
#include "stream/receive_stream.h"
#include "stream/stream_sender.h"
#include "flowcontrol/stream.h"
#include "base/stream_id_t.h"
#include "base/readable.h"
#include "base/writable.h"
#include "base/closable.h"
#include <chrono>
#include <mutex>
#include <memory>

namespace quicpp {
namespace stream {

class stream
    : public quicpp::stream::send_stream
    , public quicpp::stream::receive_stream {
private:
    std::mutex completed_mutex;
    quicpp::stream::stream_sender &sender;
    bool receive_stream_completed;
    bool send_stream_completed;

    quicpp::stream::uni_stream_sender sender_for_sendstream;
    quicpp::stream::uni_stream_sender sender_for_receivestream;
public:
    stream(quicpp::base::stream_id_t stream_id,
           quicpp::stream::stream_sender &sender,
           std::shared_ptr<quicpp::flowcontrol::stream> flowcontrol);
    quicpp::base::stream_id_t stream_id();
    virtual quicpp::base::error_t close() override;
    quicpp::base::error_t set_deadline(std::chrono::system_clock::time_point t);
    void close_for_shutdown(quicpp::base::error_t err);
    quicpp::base::error_t handle_rst_stream_frame(std::shared_ptr<quicpp::frame::rst> &frame);
    void check_if_completed();
};

}
}

#endif
