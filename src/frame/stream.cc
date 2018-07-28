#include "frame/stream.h"

quicpp::frame::stream::stream(std::basic_istream<uint8_t> &in) {
    uint8_t first_byte = in.get();
    this->offset_flag = (first_byte & 0x04) != 0;
    this->len_flag = (first_byte & 0x02) != 0;
    this->final_flag = (first_byte & 0x01) != 0;

    this->stream_id = quicpp::base::varint(in);
    if (this->offset_flag) {
        this->offset = quicpp::base::varint(in);
    }
    if (this->len_flag) {
        this->len = quicpp::base::varint(in);
    }
    else {
        this->len = in.rdbuf()->in_avail();
    }

    this->data.resize(this->len);
    in.read(const_cast<uint8_t *>(this->data.data()), this->len);
}

uint8_t quicpp::frame::stream::type() const {
    return quicpp::frame::frame_type_stream;
}

size_t quicpp::frame::stream::size() const {
    return 1 + this->stream_id.size() +
        (this->offset_flag ? this->offset.size() : 0) +
        (this->len_flag ? this->len.size() : 0) +
        this->data.size();
}

void quicpp::frame::stream::encode(std::basic_ostream<uint8_t> &out) const {
    uint8_t first_byte = this->type();

    if (this->offset_flag) {
        first_byte |= 0x04;
    }
    if (this->len_flag) {
        first_byte |= 0x02;
    }
    if (this->final_flag) {
        first_byte |= 0x01;
    }

    out.put(first_byte);
    this->stream_id.encode(out);
    if (this->offset_flag) {
        this->len.encode(out);
    }
    if (this->len_flag) {
        this->len.encode(out);
    }
    out.write(this->data.data(), this->data.size());
}
