#include "stream/crypto_stream.h"

quicpp::stream::crypto_stream::crypto_stream(quicpp::stream::stream_sender &sender,
                                             quicpp::flowcontrol::stream &flowcontrol)
    : quicpp::stream::stream(quicpp::base::stream_id_t(0),
                             sender,
                             flowcontrol) {}

void quicpp::stream::crypto_stream::set_read_offset(uint64_t offset) {
    this->receive_stream->read_offset = offset;
    this->receive_stream->frame_queue.read_position() = offset;
}
