#include "stream/send_stream.h"
#include "frame/stream_blocked.h"
#include "frame/rst.h"
#include <algorithm>

quicpp::stream::send_stream::
send_stream(quicpp::base::stream_id_t stream_id,
            quicpp::stream::stream_sender &sender,
            quicpp::flowcontrol::stream &flowcontrol)
    : _stream_id(stream_id)
    , sender(sender)
    , write_offset(0)
    , cancel_write_err(quicpp::error::success)
    , closed_for_shutdown_err(quicpp::error::success)
    , _close_for_shutdown(false)
    , finished_writing(false)
    , canceled_write(false)
    , fin_sent(false)
    , write_deadline(std::chrono::system_clock::time_point::min())
    , flowcontrol(flowcontrol) {}

quicpp::base::stream_id_t &quicpp::stream::send_stream::stream_id() {
    return this->_stream_id;
}

std::tuple<bool, quicpp::frame::stream *, bool>
quicpp::stream::send_stream::pop_stream_frame_implement(uint64_t max_bytes) {
    std::lock_guard<std::mutex> locker(this->mutex);

    if (this->closed_for_shutdown_err != quicpp::error::success) {
        return std::make_tuple(false, nullptr, false);
    }

    quicpp::frame::stream *frame = new quicpp::frame::stream();
    frame->stream_id() = this->_stream_id;
    frame->offset() = this->write_offset;
    frame->len_flag() = true;

    uint64_t maxdata_len = frame->maxdata_len(max_bytes);
    if (maxdata_len == 0) {
        return std::make_tuple(false, nullptr, this->data_for_writing.empty() == false);
    }
    std::tie(frame->data(), frame->final_flag()) = this->get_data_for_writing(maxdata_len);
    if (frame->data().empty() && frame->final_flag() == false) {
        if (this->data_for_writing.empty()) {
            return std::make_tuple(false, nullptr, false);
        }
        bool is_blocked;
        uint64_t offset;
        std::tie(is_blocked, offset) = this->flowcontrol.is_newly_blocked();
        if (is_blocked) {
            quicpp::frame::stream_blocked *frame = new quicpp::frame::stream_blocked();
            frame->stream_id() = this->_stream_id;
            frame->offset() = offset;
            this->sender.queue_control_frame(frame);
            return std::make_tuple(false, nullptr, false);
        }
        return std::make_tuple(false, nullptr, true);
    }
    if (frame->final_flag()) {
        this->fin_sent = true;
    }
    return std::make_tuple(frame->final_flag(), frame, this->data_for_writing.empty() == false);
}

std::pair<quicpp::frame::stream *, bool>
quicpp::stream::send_stream::pop_stream_frame(uint64_t max_bytes) {
    bool completed;
    quicpp::frame::stream *frame = nullptr;
    bool has_more_data;

    std::tie(completed, frame, has_more_data) =
        this->pop_stream_frame_implement(max_bytes);
    if (completed) {
        this->sender.on_stream_completed(this->_stream_id);
    }
    return std::make_pair(frame, has_more_data);
}

std::pair<std::basic_string<uint8_t>, bool> 
quicpp::stream::send_stream::get_data_for_writing(uint64_t max_bytes) {
    if (this->data_for_writing.empty()) {
        return std::make_pair(std::basic_string<uint8_t>(),
                              this->finished_writing && this->fin_sent == false);
    }
    if (this->_stream_id != 0) {
        max_bytes = std::min(max_bytes, this->flowcontrol.send_window());
    }
    if (max_bytes == 0) {
        return std::make_pair(std::basic_string<uint8_t>(), false);
    }
    std::basic_string<uint8_t> ret;
    if (this->data_for_writing.size() > max_bytes) {
        ret.assign(this->data_for_writing.begin(),
                   this->data_for_writing.begin() + max_bytes);
        this->data_for_writing.erase(0, max_bytes);
    }
    else {
        ret = this->data_for_writing;
        this->data_for_writing.clear();
        this->signal_write();
    }
    this->write_offset += ret.size();
    this->flowcontrol.sent(ret.size());
    return std::make_pair(ret,
                          this->finished_writing &&
                          this->data_for_writing.empty() &&
                          this->fin_sent == false);
}

quicpp::base::error_t quicpp::stream::send_stream::close() {
    std::lock_guard<std::mutex> locker(this->mutex);

    if (this->canceled_write) {
        return quicpp::error::close_canceled_stream;
    }
    this->finished_writing = true;
    this->sender.on_has_stream_data(this->_stream_id);
    return quicpp::error::success;
}

std::pair<bool, quicpp::base::error_t>
quicpp::stream::send_stream::
cancel_write_implement(uint16_t errorcode, quicpp::base::error_t write_err) {
    if (this->canceled_write) {
        return std::make_pair(false, quicpp::error::success);
    }
    if (this->finished_writing) {
        return std::make_pair(false, quicpp::error::close_canceled_stream);
    }
    this->canceled_write = true;
    this->cancel_write_err = write_err;
    this->signal_write();
    quicpp::frame::rst *frame = new quicpp::frame::rst();
    frame->stream_id() = this->_stream_id;
    frame->final_offset() = this->write_offset;
    frame->application_error_code() = errorcode;

    this->sender.queue_control_frame(frame);

    return std::make_pair(true, quicpp::error::success);
}

quicpp::base::error_t quicpp::stream::send_stream::cancel_write(uint16_t errorcode) {
    this->mutex.lock();
    bool completed;
    quicpp::base::error_t err;
    std::tie(completed, err) =
        this->cancel_write_implement(errorcode,
                                     quicpp::error::write_on_canceled_stream);
    this->mutex.unlock();

    if (completed) {
        this->sender.on_stream_completed(this->_stream_id);
    }
    return err;
}

bool quicpp::stream::send_stream::
handle_stop_sending_frame_implement(quicpp::frame::stop_sending *frame) {
    std::lock_guard<std::mutex> locker(this->mutex);

    quicpp::base::error_t write_err(frame->application_error_code());
    uint16_t errorcode = 7;
    bool completed;
    std::tie(completed, std::ignore) = this->cancel_write_implement(errorcode, write_err);
    return completed;
}

void quicpp::stream::send_stream::
handle_stop_sending_frame(quicpp::frame::stop_sending *frame) {
    if (this->handle_stop_sending_frame_implement(frame)) {
        this->sender.on_stream_completed(this->_stream_id);
    }
}

void quicpp::stream::send_stream::
handle_max_stream_data_frame(quicpp::frame::max_stream_data *frame) {
    this->flowcontrol.base::send_window(frame->maximum_stream_data());
    std::lock_guard<std::mutex> locker(this->mutex);
    if (this->data_for_writing.empty() == false) {
        this->sender.on_has_stream_data(this->_stream_id);
    }
}

quicpp::base::error_t quicpp::stream::send_stream::
set_write_deadline(std::chrono::system_clock::time_point t) {
    this->mutex.lock();
    auto old_deadline = this->write_deadline;
    this->write_deadline = t;
    this->mutex.unlock();
    if (t < old_deadline) {
        this->signal_write();
    }
    return quicpp::error::success;
}

void quicpp::stream::send_stream::close_for_shutdown(quicpp::base::error_t err) {
    this->mutex.lock();
    this->_close_for_shutdown = true;
    this->closed_for_shutdown_err = err;
    this->mutex.unlock();
    this->signal_write();
}

uint64_t quicpp::stream::send_stream::get_write_offset() {
    return this->write_offset;
}

void quicpp::stream::send_stream::signal_write() {
    this->write_cond.notify_one();
}

std::pair<int, quicpp::base::error_t>
quicpp::stream::send_stream::write(std::basic_string<uint8_t> p) {
    std::unique_lock<std::mutex> locker(this->mutex);
    if (this->finished_writing) {
        return std::make_pair(0, quicpp::error::write_on_closed_stream);
    }
    if (this->canceled_write) {
        return std::make_pair(0, this->closed_for_shutdown_err);
    }
    if (this->write_deadline != std::chrono::system_clock::time_point::min() &&
        std::chrono::system_clock::now() >= this->write_deadline) {
        return std::make_pair(0, quicpp::error::deadline_error);
    }

    if (p.empty()) {
        return std::make_pair(0, quicpp::error::success);
    }

    this->data_for_writing.assign(p.begin(), p.end());
    this->sender.on_has_stream_data(this->_stream_id);

    int bytes_written = 0;
    quicpp::base::error_t err;

    while (true) {
        bytes_written = p.size() - this->data_for_writing.size();
        auto deadline = this->write_deadline;
        if (deadline != std::chrono::system_clock::time_point::min() &&
            std::chrono::system_clock::now() >= deadline) {
            this->data_for_writing.clear();
            err = quicpp::error::deadline_error;
            break;
        }
        if (this->data_for_writing.empty() ||
            this->canceled_write ||
            this->_close_for_shutdown) {
            break;
        }

        locker.unlock();
        if (deadline == std::chrono::system_clock::time_point::min()) {
            this->write_cond.wait(locker);
        }
        else {
            this->write_cond.wait_until(locker, deadline);
        }
        locker.lock();
    }

    if (this->closed_for_shutdown_err != quicpp::error::success) {
        err = this->closed_for_shutdown_err;
    }
    else if (this->cancel_write_err != quicpp::error::success) {
        err = this->cancel_write_err;
    }
    return std::make_pair(bytes_written, err);
}
