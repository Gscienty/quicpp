#include "base/conn_id.h"

quicpp::base::conn_id::conn_id() {}

quicpp::base::conn_id::conn_id(std::basic_string<uint8_t> &id)
    : _id(id) {}

quicpp::base::conn_id::conn_id(std::basic_string<uint8_t> &&id)
    : _id(id) {}

quicpp::base::conn_id::conn_id(std::basic_istream<uint8_t> &in, const size_t len) {
    this->_id.resize(len);
    in.read(const_cast<uint8_t *>(this->_id.data()), len);
}

size_t quicpp::base::conn_id::size() const {
    return this->_id.size();
}

void quicpp::base::conn_id::encode(std::basic_ostream<uint8_t> &out) const {
    out.write(this->_id.data(), this->size());
}

bool quicpp::base::conn_id::operator==(const quicpp::base::conn_id &other_id) const {
    if (this->size() != other_id.size()) {
        return false;
    }

    size_t size = this->size();
    for (size_t off = 0; off < size; off++) {
        if (this->_id[off] != other_id._id[off]) {
            return false;
        }
    }
    return true;
}

std::basic_string<uint8_t> &quicpp::base::conn_id::id() {
    return this->_id;
}
