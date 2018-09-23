#include "stream/incoming_uni_streamsmap.h"


quicpp::stream::incoming_uni_streamsmap::
incoming_uni_streamsmap(quicpp::base::stream_id_t next_stream,
                        quicpp::base::stream_id_t initial_max_stream_id,
                        int max_num_streams,
                        std::function<void (std::shared_ptr<quicpp::frame::frame>)> queue_control_frame,
                        std::function<std::shared_ptr<quicpp::stream::receive_stream> (quicpp::base::stream_id_t)> new_stream)
    : next_stream(next_stream)
    , max_stream(initial_max_stream_id)
    , max_num_streams(max_num_streams)
    , new_stream(new_stream)
    , _queue_max_stream_id(queue_control_frame)
    , queue_max_stream_id([this] (std::shared_ptr<quicpp::frame::max_stream_id> &frame) -> void {
                            this->_queue_max_stream_id(std::dynamic_pointer_cast<quicpp::frame::frame>(frame));
                          }) {}


std::pair<std::shared_ptr<quicpp::stream::receive_stream>, quicpp::base::error_t>
quicpp::stream::incoming_uni_streamsmap::accept_stream() {
    std::unique_lock<std::mutex> locker(this->rw_mutex.mutex());

    std::shared_ptr<quicpp::stream::receive_stream> ret_str = nullptr;
    while (true) {
        if (this->close_err != quicpp::error::success) {
            return std::make_pair(nullptr, this->close_err);
        }

        auto str_itr = this->streams.find(this->next_stream);
        if (str_itr == this->streams.end()) {
            ret_str = str_itr->second;
            break;
        }
        this->cond.wait(locker);
    }
    this->next_stream += 4;
    return std::make_pair(ret_str, quicpp::error::success);
}

std::pair<std::shared_ptr<quicpp::stream::receive_stream>, quicpp::base::error_t>
quicpp::stream::incoming_uni_streamsmap::
get_or_open_stream(quicpp::base::stream_id_t id) {
    {
        quicpp::reader_lock_guard locker(this->rw_mutex);

        if (id > this->max_stream) {
            return std::make_pair(nullptr,
                                  quicpp::error::peer_tried_to_open_stream);
        }

        if (id <= this->highest_stream) {
            return std::make_pair(this->streams[id], quicpp::error::success);
        }
    }

    {
        quicpp::writer_lock_guard locker(this->rw_mutex);
        quicpp::base::stream_id_t start;
        if (this->highest_stream == 0) {
            start = this->next_stream;
        }
        else {
            start = this->highest_stream + 4;
        }

        for (auto new_id = start; new_id <= id; new_id += 4) {
            this->streams[new_id] = this->new_stream(new_id);
            this->cond.notify_one();
        }
        this->highest_stream = id;
        return std::make_pair(this->streams[id], quicpp::error::success);
    }
}

quicpp::base::error_t
quicpp::stream::incoming_uni_streamsmap::
delete_stream(quicpp::base::stream_id_t id) {
    quicpp::writer_lock_guard locker(this->rw_mutex);

    if (this->streams.find(id) == this->streams.end()) {
        return quicpp::error::tried_to_delete_unknow_stream;
    }
    this->streams.erase(this->streams.find(id));

    int num_new_streams = this->max_num_streams - this->streams.size();
    if (num_new_streams > 0) {
        this->max_stream = this->highest_stream + (num_new_streams * 4);
        std::shared_ptr<quicpp::frame::max_stream_id> frame = std::make_shared<quicpp::frame::max_stream_id>();
        frame->maximum_stream_id() = this->max_stream;
        this->queue_max_stream_id(frame);
    }
    return quicpp::error::success;
}
void quicpp::stream::incoming_uni_streamsmap::
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
