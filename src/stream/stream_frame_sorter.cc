#include "stream/stream_frame_sorter.h"
#include "params.h"

quicpp::stream::stream_frame_sorter::stream_frame_sorter()
    : _read_position(0) {
    this->gaps.push_back(std::make_pair(0UL, quicpp::max_byte_count));
}

quicpp::base::error_t
quicpp::stream::stream_frame_sorter::push(quicpp::frame::stream *frame) {
    if (frame->data().empty()) {
        if (frame->final_flag()) {
            this->queued_frames[frame->offset()] = frame;
        }
        return quicpp::error::success;
    }

    bool was_cut = false;
    auto old_frame_itr = this->queued_frames.find(frame->offset());
    if (old_frame_itr != this->queued_frames.end()) {
        if (frame->data().size() <= old_frame_itr->second->data().size()) {
            return quicpp::error::duplicate_stream_data;
        }
        frame->data() =
            std::basic_string<uint8_t>(frame->data().begin() + old_frame_itr->second->data().size(),
                                       frame->data().end());
        frame->offset() = frame->offset() + old_frame_itr->second->data().size();
        was_cut = true;
    }

    uint64_t start = frame->offset();
    uint64_t end = frame->offset() + frame->data().size();
    
    auto gap_itr = this->gaps.begin();
    for (;
         gap_itr != this->gaps.end();
         gap_itr++) {
        if (end <= std::get<0>(*gap_itr)) {
            return quicpp::error::duplicate_stream_data;
        }
        if (end > std::get<0>(*gap_itr) && start <= std::get<1>(*gap_itr)) {
            break;
        }
    }

    if (gap_itr == this->gaps.end()) {
        return quicpp::error::bug;
    }

    if (start < std::get<0>(*gap_itr)) {
        uint64_t add = std::get<0>(*gap_itr) - start;
        frame->offset() = frame->offset() + add;
        frame->data() = std::basic_string<uint8_t>(frame->data().begin() + add,
                                                   frame->data().end());
        was_cut = true;
    }

    auto endgap_itr = gap_itr;
    while (end >= std::get<1>(*endgap_itr)) {
        auto next_endgap = endgap_itr;
        next_endgap++;

        if (next_endgap == this->gaps.end()) {
            return quicpp::error::bug;
        }
        if (endgap_itr != gap_itr) {
            this->gaps.erase(endgap_itr);
        }
        if (end <= std::get<0>(*next_endgap)) {
            break;
        }
        this->queued_frames
            .erase(this->queued_frames.find(std::get<1>(*endgap_itr)));
        endgap_itr = next_endgap;
    }

    if (end > std::get<1>(*endgap_itr)) {
        uint64_t cut_len = end - std::get<1>(*endgap_itr);
        uint64_t len = frame->data().size() - cut_len;
        end -= cut_len;
        frame->data() =
            std::basic_string<uint8_t>(frame->data().begin(),
                                       frame->data().begin() + len);
        was_cut = true;
    }

    if (start == std::get<0>(*gap_itr)) {
        if (end >= std::get<1>(*gap_itr)) {
            this->gaps.erase(gap_itr);
        }
        if (end < std::get<1>(*endgap_itr)) {
            std::get<0>(*endgap_itr) = end;
        }
    }
    else if (end == std::get<1>(*endgap_itr)) {
        std::get<1>(*gap_itr) = start;
    }
    else {
        if (gap_itr == endgap_itr) {
            this->gaps.insert(gap_itr,
                              std::make_pair(end, std::get<1>(*gap_itr)));
            std::get<1>(*gap_itr) = start;
        }
        else {
            std::get<1>(*gap_itr) = start;
            std::get<0>(*endgap_itr) = end;
        }
    }

    if (this->gaps.size() > quicpp::max_stream_frame_sorter_gaps) {
        return quicpp::error::too_many_gaps_in_received_stream_data;
    }

    if (was_cut) {
        frame->data() =
            std::basic_string<uint8_t>(frame->data().begin(),
                                       frame->data().begin() + frame->data().size());
    }

    this->queued_frames[frame->offset()] = frame;

    return quicpp::error::success;
}

quicpp::frame::stream *quicpp::stream::stream_frame_sorter::pop() {
    quicpp::frame::stream *frame = this->head();
    if (frame != nullptr) {
        this->_read_position += frame->data().size();
        this->queued_frames.erase(this->queued_frames.find(frame->offset()));
    }
    return frame;
}

quicpp::frame::stream *quicpp::stream::stream_frame_sorter::head() {
    auto frame_itr = this->queued_frames.find(this->_read_position);
    if (frame_itr != this->queued_frames.end()) {
        return std::get<1>(*frame_itr);
    }
    return nullptr;
}

uint64_t &quicpp::stream::stream_frame_sorter::read_position() {
    return this->_read_position;
}
