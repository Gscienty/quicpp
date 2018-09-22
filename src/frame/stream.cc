#include "frame/stream.h"

quicpp::frame::stream::stream()
    : _offset_flag(false)
    , _len_flag(false)
    , _final_flag(false)
    , _stream_id(0)
    , _offset(0) {}

quicpp::frame::stream::stream(std::basic_istream<uint8_t> &in) {
    uint8_t first_byte = in.get();
    this->_offset_flag = (first_byte & 0x04) != 0;
    this->_len_flag = (first_byte & 0x02) != 0;
    this->_final_flag = (first_byte & 0x01) != 0;

    this->_stream_id = quicpp::base::stream_id_t(in);
    if (this->_offset_flag) {
        this->_offset = quicpp::base::varint(in);
    }
    size_t len;
    if (this->_len_flag) {
        len = quicpp::base::varint(in);
    }
    else {
        len = in.rdbuf()->in_avail();
    }

    this->_data.resize(len);
    in.read(const_cast<uint8_t *>(this->_data.data()), len);
}

uint8_t quicpp::frame::stream::type() const {
    return quicpp::frame::frame_type_stream;
}

size_t quicpp::frame::stream::size() const {
    return 1 + this->_stream_id.size() +
        (this->_offset_flag ? this->_offset.size() : 0) +
        (this->_len_flag ? quicpp::base::varint(this->_data.size()).size() : 0) +
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
        quicpp::base::varint(this->_data.size()).encode(out);
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

std::basic_string<uint8_t> &quicpp::frame::stream::data() {
    return this->_data;
}

uint64_t quicpp::frame::stream::maxdata_len(uint64_t maxsize) {
    uint64_t header_len = 1 + this->_stream_id.size();
    if (this->offset() != 0) {
        header_len += this->_offset.size();
    }
    if (this->_len_flag) {
        header_len++;
    }
    if (header_len > maxsize) {
        return 0;
    }
    
    uint64_t maxdata_len = maxsize - header_len;
    if (this->_len_flag && quicpp::base::varint(maxdata_len).size() != 1) {
        maxdata_len--;
    }
    return maxdata_len;
}

std::pair<std::shared_ptr<quicpp::frame::stream>, quicpp::base::error_t>
quicpp::frame::stream::maybe_split(uint64_t maxsize) {
    if (maxsize >= this->size()) {
        return std::make_pair(nullptr, quicpp::error::success);
    }

    uint64_t n = this->maxdata_len(maxsize);
    if (n == 0) {
        return std::make_pair(nullptr, quicpp::error::too_small);
    }

    std::shared_ptr<quicpp::frame::stream> frame = 
        std::make_shared<quicpp::frame::stream>();
    frame->_final_flag = false;
    frame->_stream_id = this->_stream_id;
    frame->_offset = this->_offset;
    frame->_data.assign(this->_data.begin(), this->_data.begin() + n);
    frame->_len_flag = this->_len_flag;

    this->_data.erase(0, n);
    this->_offset.value() += n;

    return std::make_pair(frame, quicpp::error::success);
}
