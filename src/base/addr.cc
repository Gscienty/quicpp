#include "base/addr.h"
#include <arpa/inet.h>

quicpp::base::addr::addr() {}

quicpp::base::addr::addr(std::string ip, uint16_t port)
    : _ip(ip)
    , _port(port) {
    
    this->_sockaddr.sin_family = AF_INET;
    this->_sockaddr.sin_port = htons(port);
    this->_sockaddr.sin_addr.s_addr = inet_addr(ip.c_str());
}

quicpp::base::addr::addr(sockaddr_in addr)
    : _ip(inet_ntoa(addr.sin_addr))
    , _port(htons(addr.sin_port))
    , _sockaddr(addr) {}

const sockaddr_in &quicpp::base::addr::get() {
    return this->_sockaddr;
}
