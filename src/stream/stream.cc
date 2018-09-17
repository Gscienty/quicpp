#include "stream/stream.h"

quicpp::stream::stream::stream(quicpp::base::stream_id_t stream_id,
           quicpp::stream::stream_sender &sender,
           quicpp::flowcontrol::stream &flowcontrol)
    : sender(sender)
    , receive_stream_completed(false)
    , send_stream_completed(false) {

    this->sender_for_sendstream =
        new quicpp::stream::
        uni_stream_sender(sender,
                          [this] () -> void {
                          std::lock_guard<std::mutex>
                            locker(this->completed_mutex);
                          this->send_stream_completed = true;
                          this->check_if_completed();
                          });

    this->send_stream = 
        new quicpp::stream::send_stream(stream_id,
                                        *this->sender_for_sendstream,
                                        flowcontrol);

    this->sender_for_receivestream =
        new quicpp::stream::
        uni_stream_sender(sender,
                          [this] () -> void {
                          std::lock_guard<std::mutex>
                                locker(this->completed_mutex);
                          this->receive_stream_completed = true;
                          this->check_if_completed();
                          });

    this->receive_stream = 
        new quicpp::stream::receive_stream(stream_id,
                                           *this->sender_for_receivestream,
                                           flowcontrol);
}

quicpp::stream::stream::~stream() {
    delete this->send_stream;
    delete this->receive_stream;
    delete this->sender_for_receivestream;
    delete this->sender_for_receivestream;
}

quicpp::base::stream_id_t
quicpp::stream::stream::stream_id() {
    return this->send_stream->stream_id();
}

quicpp::base::error_t quicpp::stream::stream::close() {
    quicpp::base::error_t err = this->send_stream->close();
    if (err != quicpp::error::success) {
        return err;
    }

    this->receive_stream->on_close(this->send_stream->get_write_offset());
    return quicpp::error::success;
}

quicpp::base::error_t
quicpp::stream::stream::set_deadline(std::chrono::system_clock::time_point t) {
    this->set_read_deadline(t);
    this->set_write_deadline(t);
    return quicpp::error::success;
}

void quicpp::stream::stream::close_for_shutdown(quicpp::base::error_t err) {
    this->send_stream->close_for_shutdown(err);
    this->receive_stream->close_for_shutdown(err);
}

quicpp::base::error_t 
quicpp::stream::stream::handle_rst_stream_frame(quicpp::frame::rst *frame) {
    quicpp::base::error_t err =
        this->receive_stream->handle_rst_stream_frame(frame);
    if (err != quicpp::error::success) {
        return err;
    }
    return quicpp::error::success;
}

void quicpp::stream::stream::check_if_completed() {
    if (this->send_stream_completed && this->receive_stream_completed) {
        this->sender.on_stream_completed(this->stream_id());
    }
}
