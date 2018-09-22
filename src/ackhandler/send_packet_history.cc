#include "ackhandler/send_packet_history.h"
#include "crypt/encryption_level.h"

quicpp::ackhandler::send_packet_history::send_packet_history()
    : num_outstanding_packets(0)
    , num_outstanding_handshake_packets(0)
    , _first_outstanding(this->packet_list.end()) {}

void 
quicpp::ackhandler::send_packet_history::
send_packet(std::shared_ptr<quicpp::ackhandler::packet> &p) {
    this->send_packet_implement(p);
}

std::list<std::shared_ptr<quicpp::ackhandler::packet>>::iterator
quicpp::ackhandler::send_packet_history::
send_packet_implement(std::shared_ptr<quicpp::ackhandler::packet> &p) {
    this->packet_list.push_back(p);
    std::list<std::shared_ptr<quicpp::ackhandler::packet>>::iterator el = 
        this->packet_list.rbegin().base();
    if (this->_first_outstanding == this->packet_list.end()) {
        this->_first_outstanding = el;
    }

    if (p->can_be_retransmitted) {
        this->num_outstanding_packets++;
        if (p->encryption_level < quicpp::crypt::encryption_forward_secure) {
            this->num_outstanding_handshake_packets++;
        }
    }

    return el;
}

std::shared_ptr<quicpp::ackhandler::packet>
quicpp::ackhandler::send_packet_history::get_packet(uint64_t packet_number) {
    if (this->packet_map.find(packet_number) == this->packet_map.end()) {
        return nullptr;
    }
    return this->packet_map[packet_number];
}

quicpp::base::error_t
quicpp::ackhandler::send_packet_history::
iterate(std::function<
        std::pair<bool, quicpp::base::error_t>
        (std::shared_ptr<quicpp::ackhandler::packet> &)> func) {
    bool cont = true;
    for (auto el = this->packet_list.begin(); 
         cont && el != this->packet_list.end();
         el++) {
        quicpp::base::error_t err;

        std::tie(cont, err) = func(*el);

        if (err != quicpp::error::success) {
            return err;
        }
    }

    return quicpp::error::success;
}

std::shared_ptr<quicpp::ackhandler::packet>
quicpp::ackhandler::send_packet_history::first_outstanding() const {
    if (this->_first_outstanding == this->packet_list.end()) {
        return nullptr;
    }

    return *this->_first_outstanding;
}

quicpp::base::error_t
quicpp::ackhandler::send_packet_history::
mark_cannot_be_retransmitted(uint64_t packet_number) {
    if (this->packet_map.find(packet_number) == this->packet_map.end()) {
        return quicpp::error::packet_not_found;
    }

    auto el = this->packet_map[packet_number];
    if (el->can_be_retransmitted) {
        this->num_outstanding_packets--;
        if (this->num_outstanding_packets < 0) {
            return quicpp::error::num_outstanding_handshake_packets_negative;
        }
        if (el->encryption_level < quicpp::crypt::encryption_forward_secure) {
            this->num_outstanding_handshake_packets--;
            if (this->num_outstanding_handshake_packets < 0) {
                return quicpp::error::num_outstanding_handshake_packets_negative;
            }
        }
    }

    el->can_be_retransmitted = false;
    if (el == this->first_outstanding()) {
        this->readjust_first_outstanding();
    }
    return quicpp::error::success;
}

void quicpp::ackhandler::send_packet_history::readjust_first_outstanding() {
    auto el = this->_first_outstanding;
    el++;
    while (el != this->packet_list.end() && (*el)->can_be_retransmitted) {
        el++;
    }
    this->_first_outstanding = el;
}

size_t quicpp::ackhandler::send_packet_history::size() const {
    return this->packet_map.size();
}

quicpp::base::error_t
quicpp::ackhandler::send_packet_history::remove(uint64_t packet_number) {
    auto el = this->packet_map.find(packet_number);
    if (el == this->packet_map.end()) {
        return quicpp::error::packet_not_found;
    }

    if (el->second == *this->_first_outstanding) {
        this->readjust_first_outstanding();
    }

    if (el->second->can_be_retransmitted) {
        this->num_outstanding_packets--;
        if (this->num_outstanding_packets < 0) {
            return quicpp::error::num_outstanding_handshake_packets_negative;
        }
        if (el->second->encryption_level < quicpp::crypt::encryption_forward_secure) {
            this->num_outstanding_handshake_packets--;
            if (this->num_outstanding_handshake_packets < 0) {
                return quicpp::error::num_outstanding_handshake_packets_negative;
            }
        }
    }

    el->second.reset();
    this->packet_map.erase(el);
    auto list_el = std::find(this->packet_list.begin(),
                             this->packet_list.end(),
                             el->second);
    this->packet_list.erase(list_el);

    return quicpp::error::success;
}

bool quicpp::ackhandler::send_packet_history::has_outstanding_packets() const {
    return this->num_outstanding_packets > 0;
} 

bool quicpp::ackhandler::send_packet_history::has_outstanding_handshake_packets() const {
    return this->num_outstanding_handshake_packets > 0;
}
