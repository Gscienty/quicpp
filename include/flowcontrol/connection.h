#ifndef _QUICPP_FLOWCONTROL_CONNECTION_
#define _QUICPP_FLOWCONTROL_CONNECTION_

#include "flowcontrol/base.h"
#include "base/error.h"
#include <functional>

namespace quicpp {
namespace flowcontrol {

class connection : public quicpp::flowcontrol::base {
private:
    std::function<void ()> update_func;
public:
    connection(uint64_t rwnd,
               uint64_t max_rwnd,
               std::function<void ()> update_func,
               quicpp::congestion::rtt &rtt);
    
    virtual uint64_t send_window() const override;
    virtual uint64_t update() override;
    virtual void maybe_update() override;

    quicpp::base::error_t increment_highest_received(uint64_t increment);
    void ensure_min_wnd(uint64_t inc);
};

}
}

#endif
