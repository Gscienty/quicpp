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

namespace quicpp {
namespace stream {

class stream
    : public quicpp::base::readable
    , public quicpp::base::writable
    , public quicpp::base::closable {
protected:
    quicpp::stream::send_stream *send_stream;
    quicpp::stream::receive_stream *receive_stream;
    quicpp::stream::stream_sender *sender_for_sendstream;
    quicpp::stream::stream_sender *sender_for_receivestream;
    std::mutex completed_mutex;
    quicpp::stream::stream_sender &sender;
    bool receive_stream_completed;
    bool send_stream_completed;
public:
    stream(quicpp::base::stream_id_t stream_id,
           quicpp::stream::stream_sender &sender,
           quicpp::flowcontrol::stream &flowcontrol);
    virtual ~stream();

    quicpp::base::stream_id_t stream_id();
    quicpp::base::error_t close();
    virtual quicpp::base::error_t cancel_write(quicpp::base::error_t err) = 0;
    virtual quicpp::base::error_t cancel_read(quicpp::base::error_t err) = 0;
    virtual quicpp::base::error_t 
    set_read_deadline(std::chrono::system_clock::time_point t) = 0;
    virtual quicpp::base::error_t
    set_write_deadline(std::chrono::system_clock::time_point t) = 0;
    quicpp::base::error_t set_deadline(std::chrono::system_clock::time_point t);
    void close_for_shutdown(quicpp::base::error_t err);
    quicpp::base::error_t handle_rst_stream_frame(quicpp::frame::rst *frame);
    void check_if_completed();
};

}
}

#endif
