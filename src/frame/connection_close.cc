#include "frame/connection_close.h"
#include "base/varint.h"

quicpp::frame::connection_close::connection_close()
    : _error_code(0) {}

quicpp::frame::connection_close::connection_close(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->_error_code = quicpp::bigendian_decode<uint16_t>(in);
    quicpp::base::varint reason_phrase_length(in);
    this->_reason_phrase.resize(reason_phrase_length.value());
    in.read(reinterpret_cast<uint8_t *>(const_cast<char *>(this->_reason_phrase.data())),
           reason_phrase_length.value());
}

uint8_t quicpp::frame::connection_close::type() const {
    return quicpp::frame::frame_type_connection_close;
}

size_t quicpp::frame::connection_close::size() const {
    size_t reason_phrase_length = this->_reason_phrase.size();
    return 1 + 2 + quicpp::base::varint(uint64_t(reason_phrase_length)).size() + reason_phrase_length;
}

void quicpp::frame::connection_close::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->type());
    quicpp::bigendian_encode(out, this->_error_code);
    quicpp::base::varint(this->_reason_phrase.size()).encode(out);
    out.write(reinterpret_cast<const uint8_t *>(this->_reason_phrase.data()),
              this->_reason_phrase.size());
}

uint16_t &quicpp::frame::connection_close::error() {
    return this->_error_code;
}

std::string &quicpp::frame::connection_close::reason_phrase() {
    return this->_reason_phrase;
}

bool quicpp::frame::connection_close::
operator== (const quicpp::frame::connection_close &frame) const {
    return this->_error_code == frame._error_code &&
        this->_reason_phrase == frame._reason_phrase;
}
