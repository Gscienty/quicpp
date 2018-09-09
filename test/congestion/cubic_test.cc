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

int main() {
    return RUN_ALL_TESTS();
}
