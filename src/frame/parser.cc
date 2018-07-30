#include "frame/parser.h"
#include "frame/stream.h"
#include "frame/rst.h"
#include "frame/connection_close.h"
#include "frame/max_data.h"
#include "frame/max_stream_data.h"
#include "frame/max_stream_id.h"
#include "frame/ping.h"
#include "frame/blocked.h"
#include "frame/stream_blocked.h"
#include "frame/stream_id_blocked.h"
#include "frame/stop_sending.h"
#include "frame/ack.h"
#include "frame/path_challenge.h"
#include "frame/path_response.h"

quicpp::frame::frame *__parse_frame(uint8_t frame_type, std::basic_istream<uint8_t> &reader) {
    if ((frame_type & 0xF8) == 0x10) {
        return new quicpp::frame::stream(reader);
    }

    switch (frame_type) {
    case 0x01:
        return new quicpp::frame::rst(reader);
    case 0x02:
        return new quicpp::frame::connection_close(reader);
    case 0x04:
        return new quicpp::frame::max_data(reader);
    case 0x05:
        return new quicpp::frame::max_stream_data(reader);
    case 0x06:
        return new quicpp::frame::max_stream_id(reader);
    case 0x07:
        return new quicpp::frame::ping(reader);
    case 0x08:
        return new quicpp::frame::blocked(reader);
    case 0x09:
        return new quicpp::frame::stream_blocked(reader);
    case 0x0A:
        return new quicpp::frame::stream_id_blocked(reader);
    case 0x0C:
        return new quicpp::frame::stop_sending(reader);
    case 0x0D:
        return new quicpp::frame::ack(reader);
    case 0x0E:
        return new quicpp::frame::path_challenge(reader);
    case 0x0F:
        return new quicpp::frame::path_response(reader);
    }
    return nullptr;
}

quicpp::frame::frame *quicpp::frame::parse_next_frame(std::basic_istream<uint8_t> &reader) {
    while (reader.rdbuf()->in_avail() != 0) {
        uint8_t type_byte = reader.get();
        if (type_byte == 0x00) {
            continue;
        }

        reader.unget();

        return __parse_frame(type_byte, reader);
    }

    return nullptr;
}
