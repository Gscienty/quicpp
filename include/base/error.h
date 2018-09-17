#ifndef _QUICPP_BASE_ERROR_
#define _QUICPP_BASE_ERROR_

#include <cstdint>

namespace quicpp {
namespace base {

class error_t {
private:
    uint64_t sign;
public:
    error_t() : sign(0) {}
    error_t(uint64_t sign) : sign(sign) {}

    bool operator== (const quicpp::base::error_t &err) {
        return this->sign == err.sign;
    }

    bool operator!= (const quicpp::base::error_t &err) {
        return this->sign != err.sign;
    }

    uint64_t code() { return this->sign; }
};
}

namespace error {

const quicpp::base::error_t success(0);
const quicpp::base::error_t bug(1);
const quicpp::base::error_t eof(2);
const quicpp::base::error_t flowcontrol_recv_too_much_data(1000);
const quicpp::base::error_t stream_data_after_termination(1001);
const quicpp::base::error_t packet_not_found(2000);
const quicpp::base::error_t num_outstanding_handshake_packets_negative(2001);
const quicpp::base::error_t invalid_ack_data(3000);
const quicpp::base::error_t encryption_level_not_equal(3001);
const quicpp::base::error_t too_many_outstanding_received_ack_ranges(3002);
const quicpp::base::error_t too_small(3003);
const quicpp::base::error_t duplicate_stream_data(4000);
const quicpp::base::error_t too_many_gaps_in_received_stream_data(4001);
const quicpp::base::error_t deadline_error(4002);
const quicpp::base::error_t stream_was_reset(4003);
const quicpp::base::error_t close_canceled_stream(4004);
const quicpp::base::error_t write_on_canceled_stream(4005);
const quicpp::base::error_t write_on_closed_stream(4006);

}
}

#endif
