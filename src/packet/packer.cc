#include "packet/packer.h"
#include "frame/stream.h"
#include "params.h"
#include <sstream>
#include <cstdint>
#include <utility>
#include <algorithm>

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
            dynamic_cast<quicpp::frame::stream *>(last_frame.get())->len_flag() = true;
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

std::shared_ptr<quicpp::packet::header>
quicpp::packet::packer::get_header(uint8_t encryption_level) {
    uint64_t pnum = this->packet_number_generator.peek();
    size_t pnum_size = quicpp::base::get_packet_number_length(pnum);

    std::shared_ptr<quicpp::packet::header> header(new quicpp::packet::header());
    header->packet_number() = pnum;
    header->packet_number() = pnum_size;


    // use tls
    if (encryption_level != quicpp::crypt::encryption_forward_secure) {
        header->header_form() = quicpp::packet::header_form_long_header;
        header->src_connid() = this->src_connid;
        header->payload_length() = this->max_packet_size;
        if (!this->has_sent_packet && this->is_client) {
            header->type() = quicpp::packet::header_type_initial;
        }
        else {
            header->type() = quicpp::packet::header_type_handshake;
        }
    }

    if (!this->omit_connid || 
        encryption_level != quicpp::crypt::encryption_forward_secure) {
        header->dest_connid() = this->dest_connid;
    }

    if (encryption_level != quicpp::crypt::encryption_forward_secure) {
        header->version() = this->version;
    }

    return header;
}

std::pair<std::unique_ptr<quicpp::packet::packed_packet>,
    quicpp::base::error_t>
quicpp::packet::packer::
pack_connection_close(std::shared_ptr<quicpp::frame::connection_close> &frame) {
    std::vector<std::shared_ptr<quicpp::frame::frame>> frames = { frame };
    auto sealer = this->crypto_setup.get_sealer();
    auto header = this->get_header(std::get<0>(sealer));
    auto raw = this->write_and_seal_packet(*header, frames, std::get<1>(sealer));

    std::unique_ptr<quicpp::packet::packed_packet> packet(new quicpp::packet::packed_packet());
    packet->header() = header;
    packet->raw() = std::get<0>(raw);
    packet->frames() = frames;
    packet->encryption_level() = std::get<0>(sealer);

    return std::make_pair(packet, std::get<1>(raw));
}
std::pair<std::unique_ptr<quicpp::packet::packed_packet>,
    quicpp::base::error_t>
quicpp::packet::packer::pack_ack_packet() {
    if (bool(this->ack_frame) == false) {
        return std::make_pair(nullptr, quicpp::error::bug);
    }

    auto sealer = this->crypto_setup.get_sealer();
    auto header = this->get_header(std::get<0>(sealer));
    std::vector<std::shared_ptr<quicpp::frame::frame>> frames = { this->ack_frame };
    this->ack_frame.reset();
    auto raw = this->write_and_seal_packet(*header, frames, std::get<1>(sealer));

    std::unique_ptr<quicpp::packet::packed_packet> packet(new quicpp::packet::packed_packet());
    packet->header() = header;
    packet->raw() = std::get<0>(raw);
    packet->frames() = frames;
    packet->encryption_level() = std::get<0>(sealer);

    return std::make_pair(packet, std::get<1>(raw));
}

std::pair<std::vector<std::shared_ptr<quicpp::packet::packed_packet>>,
    quicpp::base::error_t>
quicpp::packet::packer::
pack_retransmission(std::shared_ptr<quicpp::ackhandler::packet> &packet) {
    if (packet->encryption_level != quicpp::crypt::encryption_forward_secure) {
        auto p = this->pack_handshake_retransmission(packet);
        std::vector<std::shared_ptr<quicpp::packet::packed_packet>> frames =
        { std::get<0>(p) };
        return std::make_pair(frames, std::get<1>(p));
    }

    std::list<std::shared_ptr<quicpp::frame::frame>> control_frames;
    std::list<std::shared_ptr<quicpp::frame::frame>> stream_frames;

    for (auto frame : packet->frames) {
        if (frame->type() == quicpp::frame::frame_type_stream) {
            dynamic_cast<quicpp::frame::stream *>(frame.get())
                ->len_flag() = true;
            stream_frames.push_back(frame);
        }
        else {
            control_frames.push_back(frame);
        }
    }

    std::vector<std::shared_ptr<quicpp::packet::packed_packet>> packets;
    auto sealer = this->crypto_setup.get_sealer();

    while (!control_frames.empty() || !stream_frames.empty()) {
        std::vector<std::shared_ptr<quicpp::frame::frame>> frames;
        uint64_t payload_length = 0;

        auto header = this->get_header(std::get<0>(sealer));
        uint64_t header_length = header->size();

        uint64_t _max_size = this->max_packet_size -
            std::get<1>(sealer).overhead() -
            header_length;

        while (!control_frames.empty()) {
            auto frame = control_frames.front();
            uint64_t frame_size = frame->size();
            if (payload_length + frame_size > _max_size) {
                break;
            }
            payload_length += frame_size;
            frames.push_back(frame);
            control_frames.pop_front();
        }

        _max_size++;

        while(!stream_frames.empty() &&
              payload_length + quicpp::min_stream_frame_size < _max_size) {
            auto frame = stream_frames.front();
            std::shared_ptr<quicpp::frame::frame> will_add = frame;

            auto splited_frame = dynamic_cast<quicpp::frame::stream *>(frame.get())
                ->maybe_split(_max_size - payload_length);
            if (std::get<1>(splited_frame) != quicpp::error::success) {
                return std::make_pair(std::vector<std::shared_ptr<quicpp::packet::packed_packet>>(),
                                      std::get<1>(splited_frame));
            }
            if (bool(std::get<0>(splited_frame))) {
                will_add = std::get<0>(splited_frame);
            }
            else {
                stream_frames.pop_front();
            }
            payload_length += will_add->size();
            frames.push_back(will_add);
        }

        if (frames.back()->type() == quicpp::frame::frame_type_stream) {
            dynamic_cast<quicpp::frame::stream *>(frames.back().get())
                ->len_flag() = false;
        }

        auto raw = this->write_and_seal_packet(*header,
                                               frames,
                                               std::get<1>(sealer));
        if (std::get<1>(raw) != quicpp::error::success) {
            return std::make_pair(std::vector<std::shared_ptr<quicpp::packet::packed_packet>>(),
                                  std::get<1>(raw));
        }

        std::shared_ptr<quicpp::packet::packed_packet> packet = 
            std::make_shared<quicpp::packet::packed_packet>();

        packet->header() = header;
        packet->raw() = std::get<0>(raw);
        packet->frames() = frames;
        packet->encryption_level() = std::get<0>(sealer);

        packets.push_back(packet);
    }

    return std::make_pair(packets, quicpp::error::success);
}

std::pair<std::shared_ptr<quicpp::packet::packed_packet>,
    quicpp::base::error_t>
quicpp::packet::packer::
pack_handshake_retransmission(std::shared_ptr<quicpp::ackhandler::packet> &packet) {
    auto sealer = this->crypto_setup
        .get_sealer_with_encryption_level(packet->encryption_level);
    if (std::get<1>(sealer) != quicpp::error::success) {
        return std::make_pair(nullptr, std::get<1>(sealer));
    }

    if (packet->packet_type == quicpp::packet::header_type_initial) {
        this->has_sent_packet = false;
    }
    auto header = this->get_header(packet->encryption_level);
    header->type() = packet->packet_type;
    auto raw = this->write_and_seal_packet(*header,
                                           packet->frames,
                                           std::get<0>(sealer));

    std::shared_ptr<quicpp::packet::packed_packet> ret =
        std::make_shared<quicpp::packet::packed_packet>();

    ret->header() = header;
    ret->raw() = std::get<0>(raw);
    ret->frames() = packet->frames;
    ret->encryption_level() = packet->encryption_level;

    return std::make_pair(ret, std::get<1>(raw));
}

std::pair<std::vector<std::shared_ptr<quicpp::frame::frame>>,
    quicpp::base::error_t>
quicpp::packet::packer::
compose_next_packet(uint64_t max_frame_size, bool can_send_stream_frames) {
    uint64_t payload_length = 0;
    std::vector<std::shared_ptr<quicpp::frame::frame>> payload_frames;

    if (bool(this->ack_frame)) {
        payload_frames.push_back(this->ack_frame);
        payload_length += this->ack_frame->size();
    }

    {
        std::lock_guard<std::mutex> locker(this->control_frame_mutex);
        while (!this->control_frames.empty()) {
            auto frame = this->control_frames.back();
            uint64_t frame_size = frame->size();
            if (payload_length + frame_size > max_frame_size) {
                break;
            }
            payload_frames.push_back(frame);
            payload_length += frame_size;
            this->control_frames.pop_back();
        }
    }

    if (payload_length > max_frame_size) {
        return std::make_pair(std::vector<std::shared_ptr<quicpp::frame::frame>>(),
                              quicpp::error::bug);
    }

    if (!can_send_stream_frames) {
        return std::make_pair(payload_frames, quicpp::error::success);
    }

    max_frame_size++;

    auto fs = this->streams.pop_stream_frames(max_frame_size - payload_length);

    if (!fs.empty()) {
        fs.back()->len_flag() = false;
    }

    for (auto f : fs) {
        payload_frames.push_back(f);
    }

    return std::make_pair(payload_frames, quicpp::error::success);
}

void 
quicpp::packet::packer::
queue_control_frame(std::shared_ptr<quicpp::frame::frame> &frame) {
    switch (frame->type()) {
    case quicpp::frame::frame_type_ack:
        this->ack_frame = frame;
        break;
    default:
        {
            std::lock_guard<std::mutex> locker(this->control_frame_mutex);
            this->control_frames.push_back(frame);
        }
    }
}

bool quicpp::packet::packer::can_send_data(uint8_t encryption_level) {
    if (this->is_client) {
        return encryption_level >= quicpp::crypt::encryption_secure;
    }
    return encryption_level == quicpp::crypt::encryption_forward_secure;
}

void quicpp::packet::packer::set_omit_connection_id() {
    this->omit_connid = true;
}

void quicpp::packet::packer::change_dest_connid(quicpp::base::conn_id &conn_id) {
    this->dest_connid = conn_id;
}

void quicpp::packet::packer::set_max_packet_size(uint64_t size) {
    this->max_packet_size = std::min(this->max_packet_size, size);
}
