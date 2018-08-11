#ifndef _QUICPP_FLOWCONTROL_STREAM_
#define _QUICPP_FLOWCONTROL_STREAM_

#include "flowcontrol/base.h"
#include "flowcontrol/connection.h"
#include "base/stream_id_t.h"
#include "base/error.h"
#include <functional>

namespace quicpp {
namespace flowcontrol {

class stream : public quicpp::flowcontrol::base {
protected:
    quicpp::base::stream_id_t stream_id;
    std::function<void ()> update_func;
    std::function<void (const quicpp::base::stream_id_t &)> _update_func;
    quicpp::flowcontrol::connection &connection;
    bool contributes_to_connection;
    bool received_final_offset;
public:
    stream(quicpp::base::stream_id_t stream_id,
           bool contributes_to_connection,
           quicpp::flowcontrol::connection &connection,
           uint64_t rwnd,
           uint64_t max_rwnd,
           uint64_t swnd,
           std::function<void (const quicpp::base::stream_id_t &)> update_func,
           quicpp::congestion::rtt &rtt);

    quicpp::base::error_t update_highest_received(uint64_t offset, bool final);
    virtual void sent(const uint64_t &) override;
    virtual void read(const uint64_t &) override;
    virtual uint64_t send_window() const override;
    virtual void maybe_update() override;
    virtual uint64_t update() override;
};

}
}

#endif
