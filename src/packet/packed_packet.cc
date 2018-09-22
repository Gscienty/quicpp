#include "packet/packed_packet.h"

std::shared_ptr<quicpp::ackhandler::packet>
quicpp::packet::packed_packet::to_ack_handler_packet() {
    auto result = std::make_shared<quicpp::ackhandler::packet>();

    result->packet_number = this->_header->packet_number();
    result->packet_type = this->_header->type();
    result->frames = this->_frames;
    result->len = this->_raw.size();
    result->encryption_level = this->_encryption_level;
    result->send_time = std::chrono::system_clock::now();

    return result;
}
