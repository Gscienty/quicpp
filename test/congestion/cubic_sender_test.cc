#include "gtest/gtest.h"
#include "congestion/cubic_sender.h"
#include "params.h"
#include <iostream>

int initial_cwnd_packets = 10;
uint64_t default_wind_tcp = initial_cwnd_packets * quicpp::default_tcp_mss;
uint64_t max_cwnd = 200 * quicpp::default_tcp_mss;

bool can_send(quicpp::congestion::cubic_sender &sender,
              uint64_t bytes) {
    return bytes < sender.cwnd();
}

int send_available_swnd_len(quicpp::congestion::cubic_sender &sender,
                             uint64_t &pn,
                             uint64_t &bytes,
                             uint64_t pl) {
    int ps = 0;
    while (can_send(sender, bytes)) {
        sender.on_packet_sent(pn, bytes, true);
        pn++;
        ps++;
        bytes += pl;
    }
    return ps;
}

void lose_n_packets_len(quicpp::congestion::cubic_sender &sender,
                        uint64_t &ack_pn,
                        uint64_t &bytes,
                        uint64_t pl,
                        int n) {
    for (int i = 0; i < n; i++) {
        ack_pn++;
        sender.on_packet_lost(ack_pn, pl, bytes);
    }
    bytes -= n * pl;
}

TEST(cubic_sender, right_value_at_startup) {
    quicpp::congestion::rtt rtt;
    quicpp::congestion::cubic_sender sender(rtt,
                                            initial_cwnd_packets * quicpp::default_tcp_mss,
                                            max_cwnd);

    EXPECT_EQ(default_wind_tcp, sender.cwnd());
    EXPECT_EQ(0, sender.time_until_send(0).count());
    EXPECT_TRUE(can_send(sender, 0));
    EXPECT_EQ(default_wind_tcp, sender.cwnd());

    uint64_t pn = 1;
    uint64_t bytes = 0;
    send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
    EXPECT_FALSE(can_send(sender, bytes));
}

void ack_n_packets(quicpp::congestion::cubic_sender &sender,
                   quicpp::congestion::rtt &rtt,
                   std::chrono::system_clock::time_point &now,
                   uint64_t &ack_pn,
                   uint64_t &bytes,
                   int n) {
    rtt.update(std::chrono::milliseconds(60),
               std::chrono::milliseconds(0),
               now);
    sender.maybe_exit_slowstart();
    for (int i = 0; i < n; i++) {
        ack_pn++;
        sender.on_packet_acked(ack_pn,
                               quicpp::default_tcp_mss,
                               bytes,
                               now);
    }
    bytes -= n * quicpp::default_tcp_mss;
    now += std::chrono::milliseconds(1);
}


TEST(cubic_sender, paces) {
    quicpp::congestion::rtt rtt;
    quicpp::congestion::cubic_sender sender(rtt,
                                            initial_cwnd_packets * quicpp::default_tcp_mss,
                                            max_cwnd);
    uint64_t ack_pn = 0;
    uint64_t pn = 1;
    uint64_t bytes = 0;
    send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
    std::chrono::system_clock::time_point now = 
        std::chrono::system_clock::now() + std::chrono::hours(1);
    ack_n_packets(sender, rtt, now, ack_pn, bytes, 1);
    auto delay = sender.time_until_send(bytes);
    EXPECT_NE(0, delay.count());
    EXPECT_NE(std::chrono::microseconds::max(), delay);
}

TEST(cubic_sender, app_limited_slowstart) {
    quicpp::congestion::rtt rtt;
    quicpp::congestion::cubic_sender sender(rtt,
                                            initial_cwnd_packets * quicpp::default_tcp_mss,
                                            max_cwnd);
    int number_of_acks = 5;
    EXPECT_EQ(0, sender.time_until_send(0).count());
    EXPECT_EQ(0, sender.time_until_send(0).count());

    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    uint64_t ack_pn = 0;
    uint64_t pn = 1;
    uint64_t bytes = 0;
    send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
    for (int i = 0; i < number_of_acks; i++) {
        ack_n_packets(sender, rtt, now, ack_pn, bytes, 2);
    }
    uint64_t bytes_to_send = sender.cwnd();
    EXPECT_EQ(default_wind_tcp + quicpp::default_tcp_mss * 2 * 2, bytes_to_send);
}

TEST(cubic_sender, exponential_slowstart) {
    quicpp::congestion::rtt rtt;
    quicpp::congestion::cubic_sender sender(rtt,
                                            initial_cwnd_packets * quicpp::default_tcp_mss,
                                            max_cwnd);
    int number_of_acks = 20;
    EXPECT_EQ(0, sender.time_until_send(0).count());
    EXPECT_EQ(0, sender.bandwidth_estimate());
    EXPECT_EQ(0, sender.time_until_send(0).count());
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    uint64_t ack_pn = 0;
    uint64_t pn = 1;
    uint64_t bytes = 0;

    for (int i = 0; i < number_of_acks; i++) {
        send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
        ack_n_packets(sender, rtt, now, ack_pn, bytes, 2);
    }

    uint64_t cwnd = sender.cwnd();
    EXPECT_EQ(default_wind_tcp + quicpp::default_tcp_mss * 2 * number_of_acks,
              cwnd);
    EXPECT_EQ(quicpp::congestion::__inl_bandwidth_from_delta(cwnd, rtt.smoothed()),
              sender.bandwidth_estimate());
}

TEST(cubic_sender, slowstart_packet_loss) {
    quicpp::congestion::rtt rtt;
    quicpp::congestion::cubic_sender sender(rtt,
                                            initial_cwnd_packets * quicpp::default_tcp_mss,
                                            max_cwnd);
    sender.set_num_emulated_connections(1);
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    uint64_t ack_pn = 0;
    uint64_t pn = 1;
    uint64_t bytes = 0;
    int number_of_acks = 10;
    for (int i = 0; i < number_of_acks; i++) {
        send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
        ack_n_packets(sender, rtt, now, ack_pn, bytes, 2);
    }
    send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
    uint64_t expected_swnd = default_wind_tcp + quicpp::default_tcp_mss * 2 * number_of_acks;
    EXPECT_EQ(expected_swnd, sender.cwnd());

    lose_n_packets_len(sender, ack_pn, bytes, quicpp::default_tcp_mss, 1);
    int packets_in_recovery_wnd = expected_swnd / quicpp::default_tcp_mss;

    expected_swnd = 0.7 * expected_swnd;
    EXPECT_EQ(expected_swnd, sender.cwnd());

    uint64_t number_of_packets_in_window = expected_swnd / quicpp::default_tcp_mss;
    ack_n_packets(sender, rtt, now, ack_pn, bytes, packets_in_recovery_wnd - 1);
    send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
    EXPECT_EQ(expected_swnd, sender.cwnd());

    ack_n_packets(sender, rtt, now, ack_pn, bytes, number_of_packets_in_window - 20);
    send_available_swnd_len(sender, pn, bytes, quicpp::default_tcp_mss);
    EXPECT_EQ(expected_swnd, sender.cwnd());
}


int main() {
    return RUN_ALL_TESTS();
}
