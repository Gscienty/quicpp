#ifndef _QUICPP_STREAM_CRYPT_STREAM_
#define _QUICPP_STREAM_CRYPT_STREAM_

#include "stream/stream.h"
#include <cstdint>
#include <utility>

namespace quicpp {
namespace stream {

class crypto_stream : public quicpp::stream::stream {
public:
    crypto_stream(quicpp::stream::stream_sender &sender,
                  quicpp::flowcontrol::stream &flowcontrol);

    void set_read_offset(uint64_t offset);
    virtual std::pair<quicpp::frame::stream *, bool> pop_stream_frame(uint64_t len) = 0;
    virtual uint64_t update() = 0;
};

}
}

#endif
