#ifndef _QUICPP_STREAM_CRYPTO_STREAM_
#define _QUICPP_STREAM_CRYPTO_STREAM_

#include "stream/stream.h"
#include "frame/stream.h"
#include "frame/max_stream_data.h"
#include "base/stream_id_t.h"
#include "base/error.h"
#include "base/readable.h"
#include "base/writable.h"
#include <cstdint>
#include <utility>

namespace quicpp {
namespace stream {

class crypto_stream : public quicpp::stream::stream {
public:
    crypto_stream(quicpp::stream::stream_sender &sender,
                  quicpp::flowcontrol::stream &flowcontrol);

    void set_read_offset(uint64_t offset);
};

}
}

#endif
