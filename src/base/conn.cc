#include <errno.h>
#include "base/conn.h"
#include <unistd.h>

quicpp::base::error_t 
quicpp::base::conn::write(uint8_t *buffer, size_t size) {
    ssize_t ret = sendto(this->fd,
                         buffer,
                         size,
                         0,
                         reinterpret_cast<sockaddr *>(&this->current_addr),
                         sizeof(sockaddr_in));

    if (ret == -1) {
        // error
    }

    return quicpp::error::success;
}

std::tuple<size_t, quicpp::base::addr, quicpp::base::error_t>
quicpp::base::conn::read(uint8_t *buffer, size_t size) {
    sockaddr_in remote_addr;
    socklen_t len = sizeof(sockaddr_in);
    ssize_t ret = recvfrom(this->fd,
                           buffer,
                           size,
                           0,
                           reinterpret_cast<sockaddr *>(&remote_addr),
                           &len);
    if (ret == -1) {
        // error
    }
    return std::make_tuple(ret,
                           quicpp::base::addr(remote_addr),
                           quicpp::error::success);
}

quicpp::base::error_t quicpp::base::conn::close() {
    ::close(this->fd);

    return quicpp::error::success;
}

quicpp::base::addr quicpp::base::conn::local_addr() {
    return quicpp::base::addr();
}

quicpp::base::addr quicpp::base::conn::remote_addr() {
    quicpp::reader_lock_guard read_locker(this->rw_mutex);

    return this->current_addr;
}

void quicpp::base::conn::set_current_remote_addr(quicpp::base::addr addr) {
    quicpp::writer_lock_guard write_locker(this->rw_mutex);

    this->current_addr = addr;
}
