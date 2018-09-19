#ifndef _QUICPP_HANDSHAKE_TRANSPORT_PARAMETERS_
#define _QUICPP_HANDSHAKE_TRANSPORT_PARAMETERS_

#include <cstdint>
#include <chrono>

namespace quicpp {
namespace handshake {

class treansport_parameters {
private:
    uint64_t _streamflow_control_wnd;
    uint64_t _connectionflow_control_wnd;

    uint64_t _max_packet_size;

    uint16_t _max_uni_streams;
    uint16_t _max_bidi_streams;
    std::chrono::microseconds _idle_timeout;
    bool _disable_migration;
public:
    uint64_t &streamflow_control_window() {
        return this->_streamflow_control_wnd;
    }
    uint64_t &connectionflow_control_window() {
        return this->_connectionflow_control_wnd;
    }
    uint64_t &max_packet_size() {
        return this->_max_packet_size;
    }
    uint16_t &max_uni_streams() {
        return this->_max_uni_streams;
    }
    uint16_t &max_bidi_streams() {
        return this->_max_bidi_streams;
    }
    std::chrono::microseconds &idle_timeout() {
        return this->_idle_timeout;
    }
    bool &disable_migration() {
        return this->_disable_migration;
    }
};

}
}

#endif
