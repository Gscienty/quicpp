#include "base/conn_id.h"

quicpp::base::conn_id::conn_id() { }

quicpp::base::conn_id::conn_id(std::basic_string<uint8_t> &&id)
    : id(id) { }

quicpp::base::conn_id::conn_id(std::basic_istream<uint8_t> &in, const size_t len) {
    this->id.resize(len);
    in.get(const_cast<uint8_t *>(this->id.data()), len);
}

size_t quicpp::base::conn_id::size() const {
    return this->id.size();
}

void quicpp::base::conn_id::encode(std::basic_ostream<uint8_t> &out) const {
    out.write(this->id.data(), this->size());
}

bool quicpp::base::conn_id::operator==(const quicpp::base::conn_id &other_id) const {
    if (this->size() != other_id.size()) {
        return false;
    }

    size_t size = this->size();
    for (size_t off = 0; off < size; off++) {
        if (this->id[off] != other_id.id[off]) {
            return false;
        }
    }
    return true;
}
