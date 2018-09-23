#include "stream/receive_stream.h"
#include "frame/stop_sending.h"
#include <algorithm>

quicpp::stream::receive_stream::receive_stream(quicpp::base::stream_id_t stream_id,
                                               quicpp::stream::stream_sender &sender,
                                               std::shared_ptr<quicpp::flowcontrol::stream> flowcontrol)
    : _stream_id(stream_id) 
    , sender(sender)
    , readpos_in_frame(0)
    , read_offset(0)
    , _close_for_shutdown_err(quicpp::error::success)
    , _cancel_read_err(quicpp::error::success)
    , _reset_remotely_err(quicpp::error::success)
    , closed_for_shutdown(false)
    , fin_read(false)
    , canceled_read(false)
    , reset_remotely(false)
    , read_deadline(std::chrono::system_clock::time_point::min())
    , flowcontrol(flowcontrol) {}

quicpp::base::stream_id_t &
quicpp::stream::receive_stream::stream_id() {
    return this->_stream_id;
}

std::tuple<bool, ssize_t, quicpp::base::error_t>
quicpp::stream::receive_stream::read_implement(uint8_t *buffer_ptr, size_t size) {
    std::unique_lock<std::mutex> locker(this->mutex);

    if (this->fin_read) {
        return std::make_tuple(false, 0, quicpp::error::eof);
    }
    if (this->canceled_read) {
        return std::make_tuple(false, 0, this->_cancel_read_err);
    }
    if (this->reset_remotely) {
        return std::make_tuple(false, 0, this->_reset_remotely_err);
    }
    if (this->closed_for_shutdown) {
        return std::make_tuple(false, 0, this->_close_for_shutdown_err);
    }

    size_t bytes_read = 0;
    while (bytes_read < size) {
        std::shared_ptr<quicpp::frame::stream> frame = this->frame_queue.head();
        if (frame == nullptr && bytes_read > 0) {
            return std::make_tuple(false, bytes_read, this->_close_for_shutdown_err);
        }

        while (true) {
            if (this->closed_for_shutdown) {
                return std::make_tuple(false, bytes_read, this->_close_for_shutdown_err);
            }
            if (this->canceled_read) {
                return std::make_tuple(false, bytes_read, this->_cancel_read_err);
            }
            if (this->reset_remotely) {
                return std::make_tuple(false, bytes_read, this->_reset_remotely_err);
            }

            std::chrono::system_clock::time_point deadline = this->read_deadline;
            if (deadline != std::chrono::system_clock::time_point::min() &&
                std::chrono::system_clock::now() >= deadline) {
                return std::make_tuple(false, bytes_read, quicpp::error::deadline_error);
            }

            if (frame != nullptr) {
                this->readpos_in_frame = this->read_offset - frame->offset();
                break;
            }

            locker.unlock();
            if (deadline == std::chrono::system_clock::time_point::min()) {
                this->read_cond.wait(locker);
            }
            else {
                this->read_cond.wait_until(locker, deadline);
            }
            locker.lock();
            frame = this->frame_queue.head();
        }

        if (bytes_read > size) {
            return std::make_tuple(false, bytes_read, quicpp::error::bug);
        }
        if (this->readpos_in_frame > static_cast<ssize_t>(frame->data().size())) {
            return std::make_tuple(false, bytes_read, quicpp::error::bug);
        }

        locker.unlock();
        ssize_t copy_size = std::min(size - bytes_read,
                                     frame->data().size() - this->readpos_in_frame);
        std::copy_n(frame->data().begin() + this->readpos_in_frame,
                    copy_size,
                    buffer_ptr + bytes_read);

        this->readpos_in_frame += copy_size;
        bytes_read += copy_size;
        this->read_offset += copy_size;
        locker.lock();

        if (!this->reset_remotely) {
            this->flowcontrol->read(copy_size);
        }
        this->flowcontrol->maybe_update();

        if (this->readpos_in_frame >= static_cast<ssize_t>(frame->data().size())) {
            this->frame_queue.pop();
            this->fin_read = frame->final_flag();
            if (frame->final_flag()) {
                return std::make_tuple(true, bytes_read, quicpp::error::eof);
            }
        }
    }
    return std::make_tuple(false, bytes_read, quicpp::error::success);
}

std::pair<ssize_t, quicpp::base::error_t>
quicpp::stream::receive_stream::read(uint8_t *buffer_ptr, size_t size) {
    std::tuple<bool, int, quicpp::base::error_t> result =
        this->read_implement(buffer_ptr, size);

    if (std::get<0>(result)) {
        this->sender.on_stream_completed(this->_stream_id);
    }

    return std::make_pair(std::get<1>(result), std::get<2>(result));
}


quicpp::base::error_t 
quicpp::stream::receive_stream::cancel_read(quicpp::base::error_t err) {
    std::lock_guard<std::mutex> locker(this->mutex);

    if (this->fin_read) {
        return quicpp::error::success;
    }
    if (this->canceled_read) {
        return quicpp::error::success;
    }

    this->canceled_read = true;
    this->_cancel_read_err = err;

    this->signal_read();
    quicpp::frame::stop_sending *frame = new quicpp::frame::stop_sending();
    frame->stream_id() = this->stream_id();
    frame->application_error_code() = uint16_t(err.code());
    this->sender.queue_control_frame(frame);

    return quicpp::error::success;
}

quicpp::base::error_t 
quicpp::stream::receive_stream::
handle_rst_stream_frame(std::shared_ptr<quicpp::frame::rst> &frame) {
    bool completed;
    quicpp::base::error_t err;
    std::tie(completed, err) = this->handle_rst_frame_implement(frame);

    if (completed) {
        this->sender.on_stream_completed(this->stream_id());
    }

    return err;
}

std::pair<bool, quicpp::base::error_t>
quicpp::stream::receive_stream::
handle_rst_frame_implement(std::shared_ptr<quicpp::frame::rst> &frame) {
    std::lock_guard<std::mutex> locker(this->mutex);

    if (this->closed_for_shutdown) {
        return std::make_pair(false, quicpp::error::success);
    }
    
    quicpp::base::error_t err = 
        this->flowcontrol->update_highest_received(frame->final_offset(), true);
    if (err != quicpp::error::success) {
        return std::make_pair(false, err);
    }

    if (this->reset_remotely) {
        return std::make_pair(false, quicpp::error::success);
    }
    this->reset_remotely = true;
    this->_reset_remotely_err = quicpp::error::stream_was_reset;

    this->signal_read();
    return std::make_pair(true, quicpp::error::success);
}

quicpp::base::error_t
quicpp::stream::receive_stream::
handle_stream_frame(std::shared_ptr<quicpp::frame::stream> &frame) {
    uint64_t max_offset = frame->offset() + frame->data().size();
    quicpp::base::error_t err =
        this->flowcontrol->update_highest_received(max_offset,
                                                  frame->final_flag());
    if (err != quicpp::error::success) {
        return err;
    }

    std::lock_guard<std::mutex> locker(this->mutex);
    err = this->frame_queue.push(frame);
    if (err != quicpp::error::success &&
        err != quicpp::error::duplicate_stream_data) {
        return err;
    }

    this->signal_read();
    return quicpp::error::success;
}

void quicpp::stream::receive_stream::close_remote(uint64_t offset) {
    std::shared_ptr<quicpp::frame::stream> frame = std::make_shared<quicpp::frame::stream>();
    frame->final_flag() = true;
    frame->offset() = offset;
    this->handle_stream_frame(frame);
}

void quicpp::stream::receive_stream::on_close(uint64_t) {}

quicpp::base::error_t 
quicpp::stream::receive_stream::
set_read_deadline(std::chrono::system_clock::time_point t) {
    this->mutex.lock();
    auto old_deadline = this->read_deadline;
    this->read_deadline = t;
    this->mutex.unlock();

    if (t < old_deadline) {
        this->signal_read();
    }
    return quicpp::error::success;
}

void quicpp::stream::receive_stream::
close_for_shutdown(quicpp::base::error_t err) {
    this->mutex.lock();
    this->closed_for_shutdown = true;
    this->_close_for_shutdown_err = err;
    this->mutex.unlock();

    this->signal_read();
}

uint64_t quicpp::stream::receive_stream::update() {
    return this->flowcontrol->update();
}

void quicpp::stream::receive_stream::signal_read() {
    this->read_cond.notify_one();
}
