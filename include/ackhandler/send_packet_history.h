#ifndef _QUICPP_ACKHANDLER_SEND_PACKET_HISTORY_
#define _QUICPP_ACKHANDLER_SEND_PACKET_HISTORY_

#include "ackhandler/packet.h"
#include "base/error.h"
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <utility>
#include <memory>

namespace quicpp {
namespace ackhandler {

class send_packet_history {
private:
    std::list<std::shared_ptr<quicpp::ackhandler::packet>> packet_list;
    std::map<uint64_t, std::shared_ptr<quicpp::ackhandler::packet>> packet_map;

    int num_outstanding_packets;
    int num_outstanding_handshake_packets;

    std::list<std::shared_ptr<quicpp::ackhandler::packet>>::iterator _first_outstanding;

public:
    send_packet_history();
    void send_packet(std::shared_ptr<quicpp::ackhandler::packet> &p);
    std::list<std::shared_ptr<quicpp::ackhandler::packet>>::iterator
    send_packet_implement(std::shared_ptr<quicpp::ackhandler::packet> &packet);

    template <typename _T_Packet_Iterator>
    void send_packets_as_retransmission(_T_Packet_Iterator packet_begin,
                                        _T_Packet_Iterator packet_end,
                                        uint64_t retransmission_of) {
        if (this->packet_map.find(retransmission_of) == this->packet_map.end()) {
            std::for_each(packet_begin, packet_end,
                          [this] (std::shared_ptr<quicpp::ackhandler::packet> &p) -> void {
                          this->send_packet_implement(p);
                          });
            return;
        }

        auto retransmission = this->packet_map[retransmission_of];

        retransmission->retransmitted_as.resize(packet_end - packet_begin);

        int i = 0;
        std::for_each(packet_begin, packet_end,
                      [&, this] (std::shared_ptr<quicpp::ackhandler::packet> &p) -> void {
                      retransmission->retransmitted_as[i] = p->packet_number;
                      auto el = this->send_packet_implement(p);
                      (*el)->is_retransmission = true;
                      (*el)->retransmitted_of = retransmission_of;
                      });
    }

    std::shared_ptr<quicpp::ackhandler::packet> get_packet(uint64_t packet_number);

    quicpp::base::error_t 
    iterate(std::function<
            std::pair<bool, quicpp::base::error_t>
            (std::shared_ptr<quicpp::ackhandler::packet> &)> func);

    std::shared_ptr<quicpp::ackhandler::packet> first_outstanding() const;
    quicpp::base::error_t mark_cannot_be_retransmitted(uint64_t packet_number);
    void readjust_first_outstanding();
    size_t size() const;
    quicpp::base::error_t remove(uint64_t packet_number);
    bool has_outstanding_packets() const;
    bool has_outstanding_handshake_packets() const;
};

}
}

#endif
