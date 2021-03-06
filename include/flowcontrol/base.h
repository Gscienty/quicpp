#ifndef _QUICPP_FLOWCONTROL_BASE_
#define _QUICPP_FLOWCONTROL_BASE_

#include "congestion/rtt.h"
#include <cstdint>
#include <utility>
#include <mutex>
#include <chrono>

namespace quicpp {
namespace flowcontrol {

class base {
protected:
    uint64_t sent_bytes;
    uint64_t swnd;
    uint64_t last_blocked_at;

    std::mutex mtx;
    uint64_t read_bytes;
    uint64_t highest_received;
    uint64_t rwnd;
    uint64_t rwnd_size;
    uint64_t max_rwnd_size;

    std::chrono::system_clock::time_point epoch_start_time;
    uint64_t epoch_start_offset;
    quicpp::congestion::rtt& rtt;

    uint64_t _send_window() const;
    uint64_t _update();
public:
    base(uint64_t rwnd, uint64_t max_rwnd, quicpp::congestion::rtt &);
    base(uint64_t rwnd, uint64_t max_rwnd, uint64_t swnd, quicpp::congestion::rtt &);
    virtual ~base() {}
    virtual uint64_t send_window() const = 0;
    void send_window(const uint64_t &);
    virtual void sent(const uint64_t &);
    virtual void read(const uint64_t &);
    virtual uint64_t update() = 0;
    bool has_update() const;
    virtual void maybe_update() = 0;
    void maybe_adjust_window();
    std::pair<bool, uint64_t> is_newly_blocked();

    void start_new_auto_tuning_epoch();
    bool check_flowcontrol_violation() const;
};

}
}

#endif
