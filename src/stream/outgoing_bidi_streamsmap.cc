#include "stream/outgoing_bidi_streamsmap.h"

quicpp::stream::outgoing_bidi_streamsmap::
outgoing_bidi_streamsmap(quicpp::base::stream_id_t next_stream,
                         std::function<std::shared_ptr<quicpp::stream::stream> (quicpp::base::stream_id_t)> new_stream,
                         std::function<void (std::shared_ptr<quicpp::frame::frame>)> queue_control_frame)
    : next_stream(next_stream)
    , new_stream(new_stream)
    , _queue_stream_id_blocked(queue_control_frame)
    , queue_stream_id_blocked([this] (std::shared_ptr<quicpp::frame::stream_id_blocked> frame) -> void {
                                this->_queue_stream_id_blocked(frame);
                              }) {}


std::pair<std::shared_ptr<quicpp::stream::stream>, quicpp::base::error_t>
quicpp::stream::outgoing_bidi_streamsmap::open_stream_implement() {
    if (this->close_err != quicpp::error::success) {
        return std::make_pair(nullptr, this->close_err);
    }

    if (this->next_stream > this->max_stream) {
        if (this->max_stream == 0 || this->highest_stream < this->max_stream) {
            std::shared_ptr<quicpp::frame::stream_id_blocked> frame = std::make_shared<quicpp::frame::stream_id_blocked>();
            frame->stream_id() = this->max_stream;
            this->queue_stream_id_blocked(frame);
            this->highest_stream = this->max_stream;
        }
        return std::make_pair(nullptr, quicpp::error::too_many_open_streams);
    }

    std::shared_ptr<quicpp::stream::stream> s = this->new_stream(this->next_stream);
    this->streams[this->next_stream] = s;
    this->next_stream += 4;
    return std::make_pair(s, quicpp::error::success);
}

std::pair<std::shared_ptr<quicpp::stream::stream>, quicpp::base::error_t>
quicpp::stream::outgoing_bidi_streamsmap::open_stream() {
    quicpp::writer_lock_guard locker(this->rw_mutex);

    return this->open_stream_implement();
}

std::pair<std::shared_ptr<quicpp::stream::stream>, quicpp::base::error_t>
quicpp::stream::outgoing_bidi_streamsmap::open_stream_sync() {
    std::unique_lock<std::mutex> locker(this->rw_mutex.mutex());

    while (true) {
        std::shared_ptr<quicpp::stream::stream> ret_str;
        quicpp::base::error_t err;
        std::tie(ret_str, err) = this->open_stream_implement();
        if (err == quicpp::error::success) {
            return std::make_pair(ret_str, err);
        }
        if (err != quicpp::error::success && err != quicpp::error::too_many_open_streams) {
            return std::make_pair(nullptr, err);
        }

        this->cond.wait(locker);
    }
}

std::pair<std::shared_ptr<quicpp::stream::stream>, quicpp::base::error_t>
quicpp::stream::outgoing_bidi_streamsmap::
get_stream(quicpp::base::stream_id_t id) {
    quicpp::reader_lock_guard locker(this->rw_mutex);
    if (id >= this->next_stream) {
        return std::make_pair(nullptr, quicpp::error::invalid_stream_id);
    }

    return std::make_pair(this->streams[id], quicpp::error::success);
}

quicpp::base::error_t 
quicpp::stream::outgoing_bidi_streamsmap::
delete_stream(quicpp::base::stream_id_t id) {
    quicpp::writer_lock_guard locker(this->rw_mutex);

    if (this->streams.find(id) == this->streams.end()) {
        return quicpp::error::tried_to_delete_unknow_stream;
    }
    this->streams.erase(this->streams.find(id));
    return quicpp::error::success;
}

void
quicpp::stream::outgoing_bidi_streamsmap::
set_max_stream(quicpp::base::stream_id_t id) {
    quicpp::writer_lock_guard locker(this->rw_mutex);
    if (id > this->max_stream) {
        this->max_stream = id;
        this->cond.notify_all();
    }
}

void
quicpp::stream::outgoing_bidi_streamsmap::
close_with_error(quicpp::base::error_t err) {
    quicpp::writer_lock_guard locker(this->rw_mutex);
    this->close_err = err;
    for (auto str_itr = this->streams.begin();
         str_itr != this->streams.end();
         str_itr++) {
        str_itr->second->close_for_shutdown(err);
    }
    this->cond.notify_all();
}
