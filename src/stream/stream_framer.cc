#include "stream/stream_framer.h"
#include "params.h"

quicpp::stream::stream_framer::
stream_framer(quicpp::stream::crypto_stream &crypto_stream,
              quicpp::stream::stream_getter &stream_getter)
    : stream_getter(stream_getter)
    , crypto_stream(crypto_stream)
    , _has_crypto_stream_data(false) {}

void quicpp::stream::stream_framer::
add_active_stream(quicpp::base::stream_id_t stream_id) {
    std::lock_guard<std::mutex> locker(this->stream_queue_mutex);
    if (stream_id == 0) {
        this->_has_crypto_stream_data = true;
        return;
    }

    if (this->active_streams.find(stream_id) == this->active_streams.end()) {
        this->stream_queue.push(stream_id);
        this->active_streams.insert(stream_id);
    }
}

bool quicpp::stream::stream_framer::has_crypto_stream_data() {
    std::lock_guard<std::mutex> locker(this->stream_queue_mutex);
    return this->_has_crypto_stream_data;
}

quicpp::frame::stream *
quicpp::stream::stream_framer::pop_crypto_stream_frame(uint64_t maxlen) {
    std::lock_guard<std::mutex> locker(this->stream_queue_mutex);
    quicpp::frame::stream *frame;
    bool has_more_data;
    std::tie(frame, has_more_data) = 
        this->crypto_stream.pop_stream_frame(maxlen);
    this->_has_crypto_stream_data = has_more_data;
    return frame;
}

std::vector<quicpp::frame::stream *>
quicpp::stream::stream_framer::pop_stream_frames(uint64_t max_total_len) {
    uint64_t current_len = 0;
    std::vector<quicpp::frame::stream *> frames;

    std::lock_guard<std::mutex> locker(this->stream_queue_mutex);

    int num_actice_streams = this->stream_queue.size();
    for (int i = 0; i < num_actice_streams; i++) {
        if (max_total_len - current_len < quicpp::min_stream_frame_size) {
            break;
        }
        quicpp::base::stream_id_t id = this->stream_queue.front();
        this->stream_queue.pop();

        quicpp::stream::send_stream *sstr;
        quicpp::base::error_t err;
        std::tie(sstr, err) = this->stream_getter.get_or_open_send_stream(id);

        if (sstr == nullptr || err != quicpp::error::success) {
            this->active_streams.erase(this->active_streams.find(id));
            continue;
        }
        quicpp::frame::stream *frame;
        bool has_more_data;
        std::tie(frame, has_more_data) =
            sstr->pop_stream_frame(max_total_len - current_len);
        if (has_more_data) {
            this->stream_queue.push(id);
        }
        else {
            this->active_streams.erase(this->active_streams.find(id));
        }
        if (frame == nullptr) {
            continue;
        }
        frames.push_back(frame);
        current_len += frame->size();
    }
    return frames;
}
