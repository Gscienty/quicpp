#include "stream/window_update_queue.h"
#include "frame/max_data.h"
#include "frame/max_stream_data.h"

quicpp::stream::window_update_queue::
window_update_queue(quicpp::stream::stream_getter &stream_getter,
                    quicpp::stream::crypto_stream &crypto_stream,
                    quicpp::flowcontrol::connection &conn_fc,
                    std::function<void (std::shared_ptr<quicpp::frame::frame>)> cb)
    : queue_conn(false)
    , crypto_stream(crypto_stream)
    , stream_getter(stream_getter)
    , conn_flowcontroller(conn_fc)
    , callback(cb) {}

void quicpp::stream::window_update_queue::
add_stream(quicpp::base::stream_id_t id) {
    std::lock_guard<std::mutex> locker(this->mutex);
    this->queue[id] = true;
}

void quicpp::stream::window_update_queue::add_connection() {
    std::lock_guard<std::mutex> locker(this->mutex);
    this->queue_conn = true;
}

void quicpp::stream::window_update_queue::queue_all() {
    std::lock_guard<std::mutex> locker(this->mutex);
    if (this->queue_conn) {
        auto frame = std::make_shared<quicpp::frame::max_data>();
        frame->maximum_data() = this->conn_flowcontroller.update();
        this->callback(frame);
        this->queue_conn = false;
    }

    uint64_t offset = 0;
    for (auto id_itr = this->queue.begin();
         id_itr != this->queue.end();
         id_itr++) {
        if (id_itr->first == this->crypto_stream.stream_id()) {
            offset = this->crypto_stream.update();
        }
        else {
            quicpp::stream::receive_stream *rstr;
            quicpp::base::error_t err;
            std::tie(rstr, err) =
                this->stream_getter.get_or_open_receive_stream(id_itr->first);
            if (err != quicpp::error::success || rstr == nullptr) {
                continue;
            }
            offset = rstr->update();
        }
        if (offset == 0) {
            continue;
        }

        auto frame = std::make_shared<quicpp::frame::max_stream_data>();
        frame->stream_id() = id_itr->first;
        frame->maximum_stream_data() = offset;
        this->callback(frame);
        id_itr = this->queue.erase(id_itr);
        id_itr--;
    }
}
