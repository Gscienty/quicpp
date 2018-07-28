#include "base/reset_token.h"

quicpp::base::reset_token::reset_token() { }

quicpp::base::reset_token::reset_token(std::basic_istream<uint8_t> &in) {
    in.read(this->token, 16);
}

size_t quicpp::base::reset_token::size() const {
    return 16;
}

void quicpp::base::reset_token::encode(std::basic_ostream<uint8_t> &out) const {
    out.write(this->token, 16);
}
