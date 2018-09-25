#ifndef _QUICPP_BASE_ADDR_
#define _QUICPP_BASE_ADDR_

#include <string>
#include <netinet/in.h>

namespace quicpp {
namespace base {

class addr {
private:
    std::string _ip;
    uint16_t _port;

    sockaddr_in _sockaddr;
public:
    addr();
    addr(sockaddr_in addr);
    addr(std::string ip, uint16_t port);
    const sockaddr_in &get();
};

}
}

#endif
