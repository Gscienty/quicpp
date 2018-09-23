#ifndef _QUICPP_STREAM_STREAMS_MAP_
#define _QUICPP_STREAM_STREAMS_MAP_

#include "stream/incoming_uni_streamsmap.h"
#include "stream/incoming_bidi_streamsmap.h"
#include "stream/outgoing_uni_streamsmap.h"
#include "stream/outgoing_bidi_streamsmap.h"
#include "stream/stream_sender.h"
#include "stream/stream_getter.h"
#include "handshake/transport_parameters.h"
#include "base/stream_id_t.h"
#include "flowcontrol/stream.h"
#include <cstdint>
#include <functional>
#include <memory>

namespace quicpp {
namespace stream {

const uint8_t stream_type_outgoing_bidi = 0;
const uint8_t stream_type_incoming_bidi = 1;
const uint8_t stream_type_outgoing_uni = 2;
const uint8_t stream_type_incoming_uni = 3;
const uint8_t stream_type_error = 4;

class streamsmap : public quicpp::stream::stream_getter {
private:
    bool is_client;

    quicpp::stream::stream_sender &sender;
    std::function<std::shared_ptr<quicpp::flowcontrol::stream> (quicpp::base::stream_id_t)> new_flowcontroller;

    std::unique_ptr<quicpp::stream::incoming_uni_streamsmap> incoming_uni_streams;
    std::unique_ptr<quicpp::stream::incoming_bidi_streamsmap> incoming_bidi_streams;
    std::unique_ptr<quicpp::stream::outgoing_uni_streamsmap> outgoing_uni_streams;
    std::unique_ptr<quicpp::stream::outgoing_bidi_streamsmap> outgoing_bidi_streams;
public:
    streamsmap(quicpp::stream::stream_sender &sender,
               std::function<std::shared_ptr<quicpp::flowcontrol::stream> (quicpp::base::stream_id_t)> new_flowcontroller,
               int max_incoming_streams,
               int max_incoming_uni_streams,
               bool is_client);

    virtual ~streamsmap();

    uint8_t type(quicpp::base::stream_id_t id);
    virtual
    std::pair<std::shared_ptr<quicpp::stream::receive_stream>, quicpp::base::error_t>
    get_or_open_receive_stream(quicpp::base::stream_id_t stream_id) override;
    virtual 
    std::pair<std::shared_ptr<quicpp::stream::send_stream>, quicpp::base::error_t>
    get_or_open_send_stream(quicpp::base::stream_id_t stream_id) override;
    quicpp::base::error_t handle_max_stream_id_frame(std::shared_ptr<quicpp::frame::max_stream_id> &frame);
    void update_limits(quicpp::handshake::treansport_parameters &param);
    void close_with_error(quicpp::base::error_t err);
};

}
}

#endif
