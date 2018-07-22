#include "frame/connection_close.h"
#include "base/varint.h"

quicpp::frame::connection_close::connection_close(std::basic_istream<uint8_t> &in) {
    in.seekg(1, std::ios_base::cur);
    this->error_code = quicpp::bigendian_decode<uint16_t>(in);
    quicpp::base::varint reason_phrase_length(in);
    this->reason_phrase.resize(reason_phrase_length.get_value());
    in.get(const_cast<uint8_t *>(this->reason_phrase.data()), reason_phrase_length.get_value());
}

uint8_t quicpp::frame::connection_close::get_type() const {
    return quicpp::frame::frame_type_connection_close;
}

size_t quicpp::frame::connection_close::size() const {
    size_t reason_phrase_length = this->reason_phrase.size();
    return 1 + 2 + quicpp::base::varint(uint64_t(reason_phrase_length)).size() + reason_phrase_length;
}

void quicpp::frame::connection_close::encode(std::basic_ostream<uint8_t> &out) const {
    out.put(this->get_type());
    quicpp::bigendian_encode(out, this->error_code);
    quicpp::base::varint(this->reason_phrase.size()).encode(out);
    out.write(this->reason_phrase.data(), this->reason_phrase.size());
}
