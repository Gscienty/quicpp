#include "frame/max_data.h"

quicpp::frame::max_data::max_data()
    : _maximum_data(0) {}

quicpp::frame::max_data::max_data(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_maximum_data = quicpp::base::varint(in);
}

uint8_t quicpp::frame::max_data::type() const {
    return quicpp::frame::frame_type_max_data;
}

size_t quicpp::frame::max_data::size() const {
    return 1 + this->_maximum_data.size();
}

void quicpp::frame::max_data::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    this->_maximum_data.encode(out);
}

quicpp::base::varint &quicpp::frame::max_data::maximum_data() {
    return this->_maximum_data;
}

bool quicpp::frame::max_data::operator==(const quicpp::frame::max_data &frame) const {
    return this->_maximum_data == frame._maximum_data;
}
