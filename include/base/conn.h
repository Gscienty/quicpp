#ifndef _QUICPP_BASE_CONN_
#define _QUICPP_BASE_CONN_

#include "base/error.h"
#include "base/addr.h"
#include "rw_mutex.h"
#include <string>
#include <tuple>

namespace quicpp {
namespace base {

class conn {
private:
    quicpp::rw_mutex rw_mutex;

    int fd;
    quicpp::base::addr current_addr;
public:
    quicpp::base::error_t write(uint8_t *buffer, size_t size);
    std::tuple<size_t, quicpp::base::addr, quicpp::base::error_t>
    read(uint8_t *buffer, size_t size);
    quicpp::base::error_t close();
    quicpp::base::addr local_addr();
    quicpp::base::addr remote_addr();
    void set_current_remote_addr(quicpp::base::addr addr);
};

}
}

#endif
