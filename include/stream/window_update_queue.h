#ifndef _QUICPP_STREAM_WINDO_UPDATE_QUEUE_
#define _QUICPP_STREAM_WINDO_UPDATE_QUEUE_

#include "flowcontrol/connection.h"
#include "stream/crypto_stream.h"
#include "stream/stream_getter.h"
#include "base/stream_id_t.h"
#include <mutex>
#include <map>
#include <functional>
#include <memory>

namespace quicpp {
namespace stream {

class window_update_queue {
private:
    std::mutex mutex;
    std::map<quicpp::base::stream_id_t, bool> queue;
    bool queue_conn;

    quicpp::stream::crypto_stream &crypto_stream;
    quicpp::stream::stream_getter &stream_getter;
    quicpp::flowcontrol::connection &conn_flowcontroller;
    std::function<void (std::shared_ptr<quicpp::frame::frame>)> callback;
public:
    window_update_queue(quicpp::stream::stream_getter &stream_getter,
                        quicpp::stream::crypto_stream &crypto_stream,
                        quicpp::flowcontrol::connection &conn_fc,
                        std::function<void (std::shared_ptr<quicpp::frame::frame>)> cb);
    void add_stream(quicpp::base::stream_id_t id);
    void add_connection();
    void queue_all();
};

}
}

#endif
