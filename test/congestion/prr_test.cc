#include "congestion/prr.h"
#include "params.h"
#include "gtest/gtest.h"

TEST(prr_test, single_loss) {
    using namespace quicpp::congestion;

    prr _prr;
    uint64_t num_packets_inflight(50);
    uint64_t bytes_inflight(num_packets_inflight * quicpp::default_tcp_mss);
    uint64_t sshthresh_after_loss(num_packets_inflight / 2);
    uint64_t congestion_window(sshthresh_after_loss * quicpp::default_tcp_mss);

    _prr.on_packet_lost(bytes_inflight);
    _prr.on_packet_acked(quicpp::default_tcp_mss);
    bytes_inflight -= quicpp::default_tcp_mss;
    EXPECT_TRUE(_prr.can_send(congestion_window, bytes_inflight,
                              sshthresh_after_loss * quicpp::default_tcp_mss));

    _prr.on_packet_send(quicpp::default_tcp_mss);
    EXPECT_FALSE(_prr.can_send(congestion_window, bytes_inflight,
                               sshthresh_after_loss * quicpp::default_tcp_mss));

    for (uint64_t i = 0; i < sshthresh_after_loss - 1; i++) {
        _prr.on_packet_acked(quicpp::default_tcp_mss);
        bytes_inflight -= quicpp::default_tcp_mss;
        EXPECT_FALSE(_prr.can_send(congestion_window, bytes_inflight,
                                   sshthresh_after_loss * quicpp::default_tcp_mss));
        _prr.on_packet_acked(quicpp::default_tcp_mss);
        bytes_inflight -= quicpp::default_tcp_mss;
        EXPECT_TRUE(_prr.can_send(congestion_window, bytes_inflight,
                                  sshthresh_after_loss * quicpp::default_tcp_mss));
        _prr.on_packet_send(quicpp::default_tcp_mss);
        bytes_inflight += quicpp::default_tcp_mss;
    }

    EXPECT_EQ(congestion_window, bytes_inflight);

    for (int i = 0; i < 10; i++) {
        _prr.on_packet_acked(quicpp::default_tcp_mss);
        bytes_inflight -= quicpp::default_tcp_mss;
        EXPECT_TRUE(_prr.can_send(congestion_window, bytes_inflight,
                                  sshthresh_after_loss * quicpp::default_tcp_mss));
        _prr.on_packet_send(quicpp::default_tcp_mss);
        bytes_inflight += quicpp::default_tcp_mss;

        EXPECT_EQ(congestion_window, bytes_inflight);
        EXPECT_FALSE(_prr.can_send(congestion_window, bytes_inflight,
                                   sshthresh_after_loss * quicpp::default_tcp_mss));
    }
}

TEST(prr_test, burst_loss) {
    using namespace quicpp::congestion;

    prr _prr;
    
    uint64_t bytes_inflight(20 * quicpp::default_tcp_mss);
    int num_packets_lost(13);
    int ssthresh_after_loss(10);
    uint64_t cwnd(ssthresh_after_loss * quicpp::default_tcp_mss);

    bytes_inflight -= num_packets_lost * quicpp::default_tcp_mss;
    _prr.on_packet_lost(bytes_inflight);

    for (int i = 0; i < 3; i++) {
        _prr.on_packet_acked(quicpp::default_tcp_mss);
        bytes_inflight -= quicpp::default_tcp_mss;
        for (int j = 0; j < 2; j++) {
            EXPECT_TRUE(_prr.can_send(cwnd, bytes_inflight,
                                      ssthresh_after_loss * quicpp::default_tcp_mss));
            _prr.on_packet_send(quicpp::default_tcp_mss);
            bytes_inflight += quicpp::default_tcp_mss;
        }
        EXPECT_FALSE(_prr.can_send(cwnd, bytes_inflight,
                                   ssthresh_after_loss * quicpp::default_tcp_mss));
    }

    for (int i = 0; i < 10; i++) {
        _prr.on_packet_acked(quicpp::default_tcp_mss);
        bytes_inflight -= quicpp::default_tcp_mss;
        EXPECT_TRUE(_prr.can_send(cwnd, bytes_inflight,
                                  ssthresh_after_loss * quicpp::default_tcp_mss));
        _prr.on_packet_send(quicpp::default_tcp_mss);
        bytes_inflight += quicpp::default_tcp_mss;
    }
}

int main() {
    return RUN_ALL_TESTS();
}
