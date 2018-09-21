#include "packet/packer.h"
#include "frame/stream.h"
#include "params.h"
#include <sstream>
#include <cstdint>

quicpp::packet::packer::
packer(quicpp::base::conn_id dest_connid,
       quicpp::base::conn_id src_connid,
       uint64_t initial_packet_number,
       std::function<size_t (uint64_t packet_number)> get_packet_number_length,
       size_t max_packet_size,
       std::basic_string<uint8_t> div_nonce,
       quicpp::packet::sealing_manager &crypto_setup,
       quicpp::packet::stream_frame_source &stream_framer,
       bool is_client)
    : dest_connid(dest_connid)
    , src_connid(src_connid)
    , is_client(is_client)
    , version(0)
    , div_nonce(div_nonce)
    , crypto_setup(crypto_setup)
    , packet_number_generator(initial_packet_number, quicpp::skip_packet_average_period_length)
    , get_packet_number_length(get_packet_number_length)
    , streams(stream_framer)
    , omit_connid(false)
    , max_packet_size(max_packet_size)
    , has_sent_packet(false)
    , num_non_retransmittable_acks(0) {}

std::pair<std::basic_string<uint8_t>, quicpp::base::error_t>
quicpp::packet::packer::
write_and_seal_packet(quicpp::packet::header &header,
                      std::vector<std::shared_ptr<quicpp::frame::frame>> &payload_frames,
                      quicpp::handshake::sealer &sealer) {

    if (header.type() == quicpp::packet::header_type_initial) {
        auto &last_frame = payload_frames.back();
        if (last_frame->type() == quicpp::frame::frame_type_stream) {
            dynamic_cast<quicpp::frame::stream *>(last_frame.get())
                ->len_flag() = true;
        }
    }

    if (header.header_form() == quicpp::packet::header_form_long_header) {
        if (header.type() == quicpp::packet::header_type_initial) {
            uint64_t header_length = header.size();
            header.payload_length() = quicpp::min_initial_packet_size - header_length;
        }
        else {
            uint64_t payload_length = sealer.overhead();
            for (auto &frame : payload_frames) {
                payload_length += frame->size();
            }
            header.payload_length() = payload_length;
        }
    }

    std::basic_string<uint8_t> header_buffer;
    header_buffer.resize(header.size());

    std::basic_ostringstream<uint8_t> out;
    out.rdbuf()->pubsetbuf(const_cast<uint8_t *>(header_buffer.data()),
                           header_buffer.size());
    header.encode(out);

    std::basic_string<uint8_t> payload_buffer;
    payload_buffer.resize(header.payload_length());
    out.rdbuf()->pubsetbuf(const_cast<uint8_t *>(header_buffer.data()),
                           payload_buffer.size());
    for (auto &frame : payload_frames) {
        frame->encode(out);
    }

    if (header.type() == quicpp::packet::header_type_initial) {
        size_t padding_length = quicpp::min_initial_packet_size -
            sealer.overhead() -
            header_buffer.size() -
            payload_buffer.size();
        if (padding_length > 0) {
            for (size_t i = 0; i < padding_length; i++) {
                out.put(uint8_t(0x00));
            }
        }
    }

    size_t size = header_buffer.size() +
        payload_buffer.size() +
        sealer.overhead();
    if (size > this->max_packet_size) {
        return std::make_pair(std::basic_string<uint8_t>(),
                              quicpp::error::bug);
    }

    std::basic_string<uint8_t> raw = sealer.seal(payload_buffer,
                                                 header.packet_number(),
                                                 header_buffer);

    uint64_t num = this->packet_number_generator.pop();
    if (num != header.packet_number()) {
        return std::make_pair(std::basic_string<uint8_t>(),
                              quicpp::error::bug);
    }
    this->has_sent_packet = true;
    return std::make_pair(raw, quicpp::error::success);
}
