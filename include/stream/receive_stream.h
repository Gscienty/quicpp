#ifndef _QUICPP_STREAM_RECEIVE_STREAM_
#define _QUICPP_STREAM_RECEIVE_STREAM_

#include "stream/stream_sender.h"
#include "stream/stream_frame_sorter.h"
#include "base/stream_id_t.h"
#include "base/readable.h"
#include "base/error.h"
#include "frame/stream.h"
#include "frame/rst.h"
#include "flowcontrol/stream.h"
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <tuple>

namespace quicpp {
namespace stream {

class receive_stream : public quicpp::base::readable {
public:
    std::mutex mutex;
    quicpp::base::stream_id_t _stream_id;

    quicpp::stream::stream_sender &sender;

    quicpp::stream::stream_frame_sorter frame_queue;
    ssize_t readpos_in_frame;
    uint64_t read_offset;

    quicpp::base::error_t _close_for_shutdown_err;
    quicpp::base::error_t _cancel_read_err;
    quicpp::base::error_t _reset_remotely_err;

    bool closed_for_shutdown;
    bool fin_read;
    bool canceled_read;
    bool reset_remotely;

    std::condition_variable read_cond;
    std::chrono::system_clock::time_point read_deadline;

    quicpp::flowcontrol::stream &flowcontrol;
public:
    receive_stream(quicpp::base::stream_id_t stream_id,
                   quicpp::stream::stream_sender &sender,
                   quicpp::flowcontrol::stream &flowcontrol);
    quicpp::base::stream_id_t &stream_id();
    quicpp::base::error_t cancel_read(quicpp::base::error_t err);
    quicpp::base::error_t 
    set_read_deadline(std::chrono::system_clock::time_point t);
    std::tuple<bool, ssize_t, quicpp::base::error_t>
    read_implement(uint8_t *buffer_ptr, size_t size);
    virtual ssize_t read(uint8_t *buffer_ptr, size_t size) override;
    quicpp::base::error_t handle_stream_frame(quicpp::frame::stream *frame);
    std::pair<bool, quicpp::base::error_t>
    handle_rst_frame_implement(quicpp::frame::rst *frame);
    quicpp::base::error_t handle_rst_stream_frame(quicpp::frame::rst *frame);
    void close_remote(uint64_t offset);
    void on_close(uint64_t offset);
    void close_for_shutdown(quicpp::base::error_t err);
    uint64_t get_window_update();
    void signal_read();
};

}
}

#endif
