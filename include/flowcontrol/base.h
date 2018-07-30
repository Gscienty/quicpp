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
    quicpp::congestion::rtt rtt;
public:
    virtual uint64_t send_window() const = 0;
    uint64_t _send_window() const;
    void send_window(const uint64_t &);
    void sent(const uint64_t &);
    void read(const uint64_t &);
    virtual uint64_t update() = 0;
    uint64_t _update();
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
