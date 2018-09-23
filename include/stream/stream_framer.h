#ifndef _QUICPP_STREAM_STREAM_FRAMER_
#define _QUICPP_STREAM_STREAM_FRAMER_

#include "stream/stream.h"
#include "stream/stream_getter.h"
#include "stream/crypto_stream.h"
#include "base/stream_id_t.h"
#include <mutex>
#include <set>
#include <queue>
#include <vector>

namespace quicpp {
namespace stream {

class stream_framer {
private:
    quicpp::stream::stream_getter &stream_getter;
    quicpp::stream::crypto_stream &crypto_stream;

    std::mutex stream_queue_mutex;
    std::set<quicpp::base::stream_id_t> active_streams;
    std::queue<quicpp::base::stream_id_t> stream_queue;
    bool _has_crypto_stream_data;
public:
    stream_framer(quicpp::stream::crypto_stream &crypto_stream,
                  quicpp::stream::stream_getter &stream_getter);

    void add_active_stream(quicpp::base::stream_id_t stream_id);
    bool has_crypto_stream_data();
    std::shared_ptr<quicpp::frame::stream> pop_crypto_stream_frame(uint64_t maxlen);
    std::vector<std::shared_ptr<quicpp::frame::stream>> pop_stream_frames(uint64_t max_total_len);

};

}
}

#endif
