#include "congestion/cubic.h"
#include "gtest/gtest.h"
#include <cstdint>

uint32_t num_conns = 2;
double n_conn_beta = 
    (double(num_conns) - 1 + quicpp::congestion::_beta) /
    double(num_conns);

double n_conn_beta_last_max =
    (double(num_conns) - 1 + quicpp::congestion::beta_last_max) /
    double(num_conns);

double n_conn_alpha =
    3 * double(num_conns) * double(num_conns) *
    (1 - n_conn_beta) / (1 + n_conn_beta);

std::chrono::milliseconds max_cubic_time_interval(30);

uint64_t reno_cwnd(uint64_t current_cwnd) {
    return current_cwnd + 
        uint64_t(double(quicpp::default_tcp_mss) *
                 n_conn_alpha *
                 double(quicpp::default_tcp_mss) /
                 double(current_cwnd));
}

uint64_t cubic_convex_cwnd(uint64_t initial_cwnd,
                           std::chrono::microseconds rtt,
                           std::chrono::microseconds elapsed_time) {
    uint64_t offset = ((elapsed_time + rtt).count() << 10) / 1000000;
    uint64_t delta_cwnd = 410 * offset * offset * offset * quicpp::default_tcp_mss >> 40;
    return initial_cwnd + delta_cwnd;
}

TEST(cubic, works_above_origin) {
    using namespace quicpp::congestion;
    
    cubic _cubic;

    std::chrono::milliseconds rtt_min(100);
    double rtt_mins = double(rtt_min.count() / 1000.0);
    uint64_t current_cwnd = 10 * quicpp::default_tcp_mss;
    std::chrono::system_clock::time_point _clock = 
        std::chrono::system_clock::now() +
        std::chrono::milliseconds(1);
    std::chrono::system_clock::time_point initial_time = _clock;
    uint64_t initial_cwnd = current_cwnd;

    uint64_t expected_first_cwnd = reno_cwnd(current_cwnd);
    current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                         current_cwnd,
                                         rtt_min,
                                         _clock);
    EXPECT_EQ(current_cwnd, expected_first_cwnd);

    int max_reno_rtts = 
        std::sqrt(n_conn_alpha / (0.4 * rtt_mins * rtt_mins * rtt_mins)) - 2;
    for (int i = 0; i < max_reno_rtts; i++) {
        int num_acks_this_epoch =
            int(double(current_cwnd / quicpp::default_tcp_mss) / n_conn_alpha);

        for (int n = 0; n < num_acks_this_epoch; n++) {
            uint64_t expected_next_cwnd = reno_cwnd(current_cwnd);
            current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                                 current_cwnd,
                                                 rtt_min,
                                                 _clock);
            EXPECT_EQ(current_cwnd, expected_next_cwnd);
        }

        _clock += std::chrono::milliseconds(100);
    }

    for (int i = 0; i < 54; i++) {
        int max_acks_this_epoch = current_cwnd / quicpp::default_tcp_mss;
        std::chrono::microseconds interval(100 * 1000 / max_acks_this_epoch);
        for (int n = 0; n < max_acks_this_epoch; n++) {
            _clock += interval;
            current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                                 current_cwnd,
                                                 rtt_min,
                                                 _clock);
            using namespace std::chrono;
            uint64_t expected_cwnd = 
                cubic_convex_cwnd(initial_cwnd,
                                  rtt_min,
                                  duration_cast<microseconds>(_clock - initial_time));
            EXPECT_EQ(current_cwnd, expected_cwnd);
        }
    }
}

TEST(cubic, works_above_origin2) {
    using namespace quicpp::congestion;

    cubic _cubic;

    uint64_t current_cwnd = 1000 * quicpp::default_tcp_mss;
    uint64_t initial_cwnd = current_cwnd;
    std::chrono::milliseconds rtt_min(100);
    std::chrono::system_clock::time_point _clock = 
        std::chrono::system_clock::now() + std::chrono::milliseconds(1);
    std::chrono::system_clock::time_point initial_time = _clock;

    current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                         current_cwnd,
                                         rtt_min,
                                         _clock);
    _clock += std::chrono::milliseconds(600);
    current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                         current_cwnd,
                                         rtt_min,
                                         _clock);

    for (int i = 0; i < 100; i++) {
        using namespace std::chrono;
        _clock += std::chrono::milliseconds(10);
        uint64_t expected_cwnd = 
            cubic_convex_cwnd(initial_cwnd,
                              rtt_min,
                              duration_cast<microseconds>(_clock - initial_time));
        uint64_t next_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                                   current_cwnd,
                                                   rtt_min,
                                                   _clock);
        EXPECT_EQ(next_cwnd, expected_cwnd);
        EXPECT_GT(next_cwnd, current_cwnd);
        
        current_cwnd = next_cwnd;
    }
}

TEST(cubic, handles_per_ack_updates) {
    using namespace quicpp::congestion;

    cubic _cubic;

    int initial_cwnd_packets = 150;
    uint64_t current_cwnd(initial_cwnd_packets * quicpp::default_tcp_mss);
    std::chrono::milliseconds rtt_min(350);
    std::chrono::system_clock::time_point _clock = 
        std::chrono::system_clock::now();

    _clock += std::chrono::milliseconds(1);
    uint64_t r_cwnd = reno_cwnd(current_cwnd);
    current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                         current_cwnd,
                                         rtt_min,
                                         _clock);
    uint64_t initial_cwnd = current_cwnd;

    int max_acks = int(double(initial_cwnd_packets) / n_conn_alpha);
    int interval = max_cubic_time_interval / std::chrono::milliseconds(max_acks + 1);

    _clock += std::chrono::milliseconds(interval);
    r_cwnd = reno_cwnd(r_cwnd);

    EXPECT_EQ(current_cwnd,
              _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                    current_cwnd,
                                    rtt_min,
                                    _clock));

    for (int i = 1; i < max_acks; i++) {
        _clock += std::chrono::milliseconds(interval);

        uint64_t next_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                                   current_cwnd,
                                                   rtt_min,
                                                   _clock);
        r_cwnd = reno_cwnd(r_cwnd);

        EXPECT_GT(next_cwnd, current_cwnd);
        EXPECT_EQ(next_cwnd, r_cwnd);
        current_cwnd = next_cwnd;
    }

    uint64_t minimum_expected_increase = quicpp::default_tcp_mss * 9 / 10;
    EXPECT_GT(current_cwnd, initial_cwnd + minimum_expected_increase);
}

TEST(cubic, handles_loss_events) {
    using namespace quicpp::congestion;

    cubic _cubic;

    std::chrono::milliseconds rtt_min(100);
    uint64_t current_cwnd = 422 * quicpp::default_tcp_mss;
    uint64_t expected_cwnd = reno_cwnd(current_cwnd);

    std::chrono::system_clock::time_point _clock = 
        std::chrono::system_clock::now();

    _clock += std::chrono::milliseconds(1);
    EXPECT_EQ(_cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                    current_cwnd,
                                    rtt_min,
                                    _clock),
              expected_cwnd);

    uint64_t pre_loss_cwnd = current_cwnd;
    EXPECT_EQ(_cubic.last_max_congestion_window(), 0);
    expected_cwnd = uint64_t(double(current_cwnd) * n_conn_beta);
    EXPECT_EQ(_cubic.cwnd_after_packet_loss(current_cwnd), expected_cwnd);
    EXPECT_EQ(_cubic.last_max_congestion_window(), pre_loss_cwnd);

    current_cwnd = expected_cwnd;

    pre_loss_cwnd = current_cwnd;
    expected_cwnd = uint64_t(double(current_cwnd) * n_conn_beta);
    EXPECT_EQ(_cubic.cwnd_after_packet_loss(current_cwnd), expected_cwnd);
    current_cwnd = expected_cwnd;
    EXPECT_GT(pre_loss_cwnd, _cubic.last_max_congestion_window());
    uint64_t expected_last_max = uint64_t(double(pre_loss_cwnd) * n_conn_beta_last_max);
    EXPECT_EQ(_cubic.last_max_congestion_window(), expected_last_max);
    EXPECT_LT(expected_cwnd, _cubic.last_max_congestion_window());
    current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                         current_cwnd,
                                         rtt_min,
                                         _clock);
    EXPECT_GT(_cubic.last_max_congestion_window(), current_cwnd);

    current_cwnd = _cubic.last_max_congestion_window() - 1;
    pre_loss_cwnd = current_cwnd;
    expected_cwnd = uint64_t(double(current_cwnd) * n_conn_beta);
    EXPECT_EQ(_cubic.cwnd_after_packet_loss(current_cwnd), expected_cwnd);
    expected_last_max = pre_loss_cwnd;
    EXPECT_EQ(_cubic.last_max_congestion_window(), expected_last_max);
}

TEST(cubic, works_below_origin) {
    using namespace quicpp::congestion;
    cubic _cubic;

    std::chrono::milliseconds rtt_min(100);
    uint64_t current_cwnd = 422 * quicpp::default_tcp_mss;
    uint64_t expected_cwnd = reno_cwnd(current_cwnd);
    std::chrono::system_clock::time_point _clock =
        std::chrono::system_clock::now();
    _clock += std::chrono::milliseconds(1);

    EXPECT_EQ(_cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                    current_cwnd,
                                    rtt_min,
                                    _clock),
              expected_cwnd);

    expected_cwnd = uint64_t(double(current_cwnd) * n_conn_beta);
    EXPECT_EQ(_cubic.cwnd_after_packet_loss(current_cwnd), expected_cwnd);
    current_cwnd = expected_cwnd;
    current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                         current_cwnd,
                                         rtt_min,
                                         _clock);
    for (int i = 0; i < 40; i++) {
        _clock += std::chrono::milliseconds(100);
        current_cwnd = _cubic.cwnd_after_ack(quicpp::default_tcp_mss,
                                             current_cwnd,
                                             rtt_min,
                                             _clock);
    }

    expected_cwnd = 553632;
    EXPECT_EQ(current_cwnd, expected_cwnd);
}

int main() {
    return RUN_ALL_TESTS();
}
