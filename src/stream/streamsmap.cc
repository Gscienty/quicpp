#include "stream/streamsmap.h"

quicpp::stream::streamsmap::
streamsmap(quicpp::stream::stream_sender &sender,
           std::function<std::shared_ptr<quicpp::flowcontrol::stream> (quicpp::base::stream_id_t)> new_flowcontroller,
           int max_incoming_streams,
           int max_incoming_uni_streams,
           bool is_client)
    : is_client(is_client)
    , sender(sender)
    , new_flowcontroller(new_flowcontroller) {
    quicpp::base::stream_id_t
        first_outgoing_bidi_stream,
        first_outgoing_uni_stream,
        first_incoming_bidi_stream,
        first_incoming_uni_stream;

    if (is_client) {
        first_outgoing_bidi_stream = 4;
        first_incoming_bidi_stream = 1;
        first_outgoing_uni_stream = 2;
        first_incoming_uni_stream = 3;
    }
    else {
        first_outgoing_bidi_stream = 1;
        first_incoming_bidi_stream = 4;
        first_outgoing_uni_stream = 3;
        first_incoming_uni_stream = 2;
    }

    this->outgoing_bidi_streams.reset(new quicpp::stream::
        outgoing_bidi_streamsmap(first_outgoing_bidi_stream,
                                 [this] (quicpp::base::stream_id_t id) -> std::shared_ptr<quicpp::stream::stream> {
                                    return std::make_shared<quicpp::stream::stream>(id,
                                                                  this->sender,
                                                                  this->new_flowcontroller(id));
                                 },
                                 [this] (std::shared_ptr<quicpp::frame::frame> frame) -> void {
                                    this->sender.queue_control_frame(frame);
                                 }));

    this->incoming_bidi_streams.reset(new quicpp::stream::
        incoming_bidi_streamsmap(first_outgoing_bidi_stream,
                                 quicpp::base::stream_id_t::max_bidi_stream_id(max_incoming_streams, is_client),
                                 max_incoming_streams,
                                 [this] (quicpp::base::stream_id_t id) -> std::shared_ptr<quicpp::stream::stream> {
                                    return std::make_shared<quicpp::stream::stream>(id,
                                                                      this->sender,
                                                                      this->new_flowcontroller(id));
                                 },
                                 [this] (std::shared_ptr<quicpp::frame::frame> frame) -> void {
                                    this->sender.queue_control_frame(frame);
                                 }));
    this->outgoing_uni_streams.reset(new quicpp::stream::
        outgoing_uni_streamsmap(first_outgoing_uni_stream,
                                [this] (std::shared_ptr<quicpp::frame::frame> frame) -> void {
                                    this->sender.queue_control_frame(frame);
                                },
                                [this] (quicpp::base::stream_id_t id) -> std::shared_ptr<quicpp::stream::send_stream> {
                                    return std::make_shared<quicpp::stream::send_stream>(id,
                                                                           this->sender,
                                                                           this->new_flowcontroller(id));
                                }));
    this->incoming_uni_streams.reset(new quicpp::stream::
        incoming_uni_streamsmap(first_incoming_bidi_stream,
                                 quicpp::base::stream_id_t::max_uni_stream_id(max_incoming_uni_streams, is_client),
                                 max_incoming_uni_streams,
                                 [this] (std::shared_ptr<quicpp::frame::frame> frame) -> void {
                                    this->sender.queue_control_frame(frame);
                                 },
                                 [this] (quicpp::base::stream_id_t id) -> std::shared_ptr<quicpp::stream::receive_stream> {
                                    return std::make_shared<quicpp::stream::receive_stream>(id,
                                                                             this->sender,
                                                                             this->new_flowcontroller(id));
                                 }));
}

quicpp::stream::streamsmap::~streamsmap() {}


uint8_t quicpp::stream::streamsmap::type(quicpp::base::stream_id_t id) {
    if (this->is_client) {
        switch (id % 4) {
        case 0:
            return quicpp::stream::stream_type_outgoing_bidi;
        case 1:
            return quicpp::stream::stream_type_incoming_bidi;
        case 2:
            return quicpp::stream::stream_type_outgoing_uni;
        case 3:
            return quicpp::stream::stream_type_incoming_uni;
        }
    }
    else {
        switch (id % 4) {
        case 0:
            return quicpp::stream::stream_type_incoming_bidi;
        case 1:
            return quicpp::stream::stream_type_outgoing_bidi;
        case 2:
            return quicpp::stream::stream_type_incoming_uni;
        case 3:
            return quicpp::stream::stream_type_outgoing_uni;
        }
    }
    return quicpp::stream::stream_type_error;
}

std::pair<std::shared_ptr<quicpp::stream::receive_stream>, quicpp::base::error_t>
quicpp::stream::streamsmap::
get_or_open_receive_stream(quicpp::base::stream_id_t stream_id) {
    switch (this->type(stream_id)) {
    case quicpp::stream::stream_type_outgoing_bidi:
        return this->outgoing_bidi_streams->get_stream(stream_id);
    case quicpp::stream::stream_type_incoming_bidi:
        return this->incoming_bidi_streams->get_or_open_stream(stream_id);
    case quicpp::stream::stream_type_incoming_uni:
        return this->incoming_uni_streams->get_or_open_stream(stream_id);
    case quicpp::stream::stream_type_outgoing_uni:
        return std::make_pair(nullptr, quicpp::error::peer_attemped_to_open_recieve_stream);
    }
    return std::make_pair(nullptr, quicpp::error::invalid_stream_type);
}

std::pair<std::shared_ptr<quicpp::stream::send_stream>, quicpp::base::error_t>
quicpp::stream::streamsmap::
get_or_open_send_stream(quicpp::base::stream_id_t stream_id) {
    switch (this->type(stream_id)) {
    case quicpp::stream::stream_type_outgoing_bidi:
        return this->outgoing_bidi_streams->get_stream(stream_id);
    case quicpp::stream::stream_type_incoming_bidi:
        return this->incoming_bidi_streams->get_or_open_stream(stream_id);
    case quicpp::stream::stream_type_outgoing_uni:
        return this->outgoing_uni_streams->get_stream(stream_id);
    case quicpp::stream::stream_type_incoming_uni:
        return std::make_pair(nullptr, quicpp::error::peer_attemped_to_open_send_stream);
    }
    return std::make_pair(nullptr, quicpp::error::invalid_stream_type);
}

quicpp::base::error_t
quicpp::stream::streamsmap::
handle_max_stream_id_frame(std::shared_ptr<quicpp::frame::max_stream_id> &frame) {
    quicpp::base::stream_id_t id = frame->maximum_stream_id();
    switch (this->type(id)) {
    case quicpp::stream::stream_type_outgoing_bidi:
        this->outgoing_bidi_streams->set_max_stream(id);
        return quicpp::error::success;
    case quicpp::stream::stream_type_outgoing_uni:
        this->outgoing_uni_streams->set_max_stream(id);
        return quicpp::error::success;
    default:
        return quicpp::error::received_max_stream_data_frame_for_incoming_stream;
    }
}

void quicpp::stream::streamsmap::close_with_error(quicpp::base::error_t err) {
    this->outgoing_bidi_streams->close_with_error(err);
    this->outgoing_uni_streams->close_with_error(err);
    this->incoming_bidi_streams->close_with_error(err);
    this->incoming_uni_streams->close_with_error(err);
}

void quicpp::stream::streamsmap::
update_limits(quicpp::handshake::treansport_parameters &param) {
    bool peer_is_client = false;
    if (this->is_client == false) {
        peer_is_client = true;
    }

    this->outgoing_bidi_streams
        ->set_max_stream(quicpp::base::stream_id_t::
                         max_bidi_stream_id(param.max_bidi_streams(),
                                            peer_is_client));
    this->outgoing_uni_streams
        ->set_max_stream(quicpp::base::stream_id_t::
                         max_uni_stream_id(param.max_uni_streams(),
                                           peer_is_client));
}
