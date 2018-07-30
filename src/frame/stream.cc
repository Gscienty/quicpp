#include "frame/stream.h"

quicpp::frame::stream::stream()
    : _offset_flag(false)
    , _len_flag(false)
    , _final_flag(false)
    , _stream_id(0)
    , _offset(0)
    , _len(0) {}

quicpp::frame::stream::stream(std::basic_istream<uint8_t> &in) {
    uint8_t first_byte = in.get();
    this->_offset_flag = (first_byte & 0x04) != 0;
    this->_len_flag = (first_byte & 0x02) != 0;
    this->_final_flag = (first_byte & 0x01) != 0;

    this->_stream_id = quicpp::base::stream_id_t(in);
    if (this->_offset_flag) {
        this->_offset = quicpp::base::varint(in);
    }
    if (this->_len_flag) {
        this->_len = quicpp::base::varint(in);
    }
    else {
        this->_len = in.rdbuf()->in_avail();
    }

    this->_data.resize(this->_len);
    in.read(const_cast<uint8_t *>(this->_data.data()), this->_len);
}

uint8_t quicpp::frame::stream::type() const {
    return quicpp::frame::frame_type_stream;
}

size_t quicpp::frame::stream::size() const {
    return 1 + this->_stream_id.size() +
        (this->_offset_flag ? this->_offset.size() : 0) +
        (this->_len_flag ? this->_len.size() : 0) +
        this->_data.size();
}

void quicpp::frame::stream::encode(std::basic_ostream<uint8_t> &out) const {
    uint8_t first_byte = this->type();

    if (this->_offset_flag) {
        first_byte |= 0x04;
    }
    if (this->_len_flag) {
        first_byte |= 0x02;
    }
    if (this->_final_flag) {
        first_byte |= 0x01;
    }

    out.put(first_byte);
    this->_stream_id.encode(out);
    if (this->_offset_flag) {
        this->_offset.encode(out);
    }
    if (this->_len_flag) {
        this->_len.encode(out);
    }
    out.write(this->_data.data(), this->_data.size());
}

bool &quicpp::frame::stream::offset_flag() {
    return this->_offset_flag;
}

bool &quicpp::frame::stream::len_flag() {
    return this->_len_flag;
}

bool &quicpp::frame::stream::final_flag() {
    return this->_final_flag;
}

quicpp::base::stream_id_t &quicpp::frame::stream::stream_id() {
    return this->_stream_id;
}

quicpp::base::varint &quicpp::frame::stream::offset() {
    return this->_offset;
}

quicpp::base::varint &quicpp::frame::stream::len() {
    return this->_len;
}

std::basic_string<uint8_t> &quicpp::frame::stream::data() {
    return this->_data;
}
