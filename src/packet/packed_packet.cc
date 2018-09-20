#include "packet/packed_packet.h"

std::shared_ptr<quicpp::ackhandler::packet>
quicpp::packet::packed_packet::to_ack_handler_packet() {
    auto result = std::make_shared<quicpp::ackhandler::packet>();

    result->packet_number = this->header->packet_number();
    result->packet_type = this->header->type();
    result->frames = this->frames;
    result->len = this->raw.size();
    result->encryption_level = this->encryption_level;
    result->send_time = std::chrono::system_clock::now();

    return result;
}
