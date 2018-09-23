#ifndef _QUICPP_STREAM_STREAM_GETTER_
#define _QUICPP_STREAM_STREAM_GETTER_

#include "base/error.h"
#include "base/stream_id_t.h"
#include "stream/receive_stream.h"
#include "stream/send_stream.h"
#include <utility>

namespace quicpp {
namespace stream {

class stream_getter {
public:
    virtual
    std::pair<std::shared_ptr<quicpp::stream::receive_stream>,
        quicpp::base::error_t>
    get_or_open_receive_stream(quicpp::base::stream_id_t stream_id) = 0;
    virtual 
    std::pair<std::shared_ptr<quicpp::stream::send_stream>,
        quicpp::base::error_t>
    get_or_open_send_stream(quicpp::base::stream_id_t stream_id) = 0;
};

}
}

#endif
