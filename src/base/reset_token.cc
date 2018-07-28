#include "base/reset_token.h"

quicpp::base::reset_token::reset_token() { }

quicpp::base::reset_token::reset_token(std::basic_istream<uint8_t> &in) {
    this->_token.resize(16, 0x00);
    in.read(const_cast<uint8_t *>(this->_token.data()), 16);
}

size_t quicpp::base::reset_token::size() const {
    return 16;
}

void quicpp::base::reset_token::encode(std::basic_ostream<uint8_t> &out) const {
    out.write(this->_token.data(), 16);
}

std::basic_string<uint8_t> &quicpp::base::reset_token::token() {
    return this->_token;
}
