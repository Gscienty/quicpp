#ifndef _QUICPP_STREAM_SEND_STREAM_
#define _QUICPP_STREAM_SEND_STREAM_

#include "stream/stream_sender.h"
#include "base/stream_id_t.h"
#include "base/error.h"
#include "base/writable.h"
#include "base/closable.h"
#include "flowcontrol/stream.h"
#include "frame/stream.h"
#include "frame/stop_sending.h"
#include "frame/max_stream_data.h"
#include <cstdint>
#include <mutex>
#include <string>
#include <condition_variable>
#include <chrono>
#include <utility>
#include <memory>

namespace quicpp {
namespace stream {

class send_stream
    : public quicpp::base::writable
    , public quicpp::base::closable {
private:
    std::mutex mutex;

    quicpp::base::stream_id_t _stream_id;
    quicpp::stream::stream_sender &sender;
    
    uint64_t write_offset;
    quicpp::base::error_t cancel_write_err;
    quicpp::base::error_t closed_for_shutdown_err;

    bool _close_for_shutdown;
    bool finished_writing;
    bool canceled_write;
    bool fin_sent;
    
    std::basic_string<uint8_t> data_for_writing;
    std::condition_variable write_cond;
    std::chrono::system_clock::time_point write_deadline;
    
    std::shared_ptr<quicpp::flowcontrol::stream> flowcontrol;

public:
    send_stream(quicpp::base::stream_id_t stream_id,
                quicpp::stream::stream_sender &sender,
                std::shared_ptr<quicpp::flowcontrol::stream> flowcontrol);

    quicpp::base::stream_id_t &stream_id();
    std::tuple<bool, std::shared_ptr<quicpp::frame::stream>, bool>
    pop_stream_frame_implement(uint64_t max_bytes);
    std::pair<std::shared_ptr<quicpp::frame::stream>, bool>
    pop_stream_frame(uint64_t max_bytes);
    std::pair<std::basic_string<uint8_t>, bool> get_data_for_writing(uint64_t max_bytes);
    virtual quicpp::base::error_t close() override;
    std::pair<bool, quicpp::base::error_t>
    cancel_write_implement(uint16_t errorcode, quicpp::base::error_t write_err);
    quicpp::base::error_t cancel_write(uint16_t errorcode);
    bool handle_stop_sending_frame_implement(std::shared_ptr<quicpp::frame::stop_sending> &frame);
    void handle_stop_sending_frame(std::shared_ptr<quicpp::frame::stop_sending> &frame);
    void handle_max_stream_data_frame(std::shared_ptr<quicpp::frame::max_stream_data> &frame);
    quicpp::base::error_t set_write_deadline(std::chrono::system_clock::time_point t);
    void close_for_shutdown(quicpp::base::error_t err);
    uint64_t get_write_offset();
    void signal_write();
    virtual
    std::pair<ssize_t, quicpp::base::error_t>
    write(uint8_t *buffer_ptr, size_t size) override;
};

}
}

#endif
