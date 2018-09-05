#ifndef _QUICPP_ACKHANDLER_SEND_MODE_
#define _QUICPP_ACKHANDLER_SEND_MODE_

#include <cstdint>

namespace quicpp {
namespace ackhandler {

class send_mode_t {
private:
    uint8_t _send_mode;
public:
    send_mode_t() : _send_mode(0) {}
    send_mode_t(uint8_t val) : _send_mode(val) {}

    bool operator== (const quicpp::ackhandler::send_mode_t &send_mode) {
        return this->_send_mode == send_mode._send_mode;
    }

    bool operator!= (const quicpp::ackhandler::send_mode_t &send_mode) {
        return this->_send_mode != send_mode._send_mode;
    }
};
}

namespace send_mode {

const quicpp::ackhandler::send_mode_t send_none(0);
const quicpp::ackhandler::send_mode_t send_ack(1);
const quicpp::ackhandler::send_mode_t send_retransmission(2);
const quicpp::ackhandler::send_mode_t send_rto(3);
const quicpp::ackhandler::send_mode_t send_tlp(4);
const quicpp::ackhandler::send_mode_t send_any(5);

}
}

#endif
