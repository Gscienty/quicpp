#include "congestion/rtt.h"
#include "gtest/gtest.h"

TEST(rtt, default) {
    quicpp::congestion::rtt rtt;

    EXPECT_EQ(std::chrono::microseconds::zero(), rtt.min());
    EXPECT_EQ(std::chrono::microseconds::zero(), rtt.smoothed());
}

TEST(rtt, smoothed) {
    quicpp::congestion::rtt rtt;

    rtt.update(std::chrono::milliseconds(300), std::chrono::milliseconds(100), std::chrono::system_clock::now());
    EXPECT_EQ(std::chrono::milliseconds(300), rtt.latest());
    EXPECT_EQ(std::chrono::milliseconds(300), rtt.smoothed());

    rtt.update(std::chrono::milliseconds(350), std::chrono::milliseconds(50), std::chrono::system_clock::now());
    EXPECT_EQ(std::chrono::milliseconds(300), rtt.latest());
    EXPECT_EQ(std::chrono::milliseconds(300), rtt.smoothed());

    rtt.update(std::chrono::milliseconds(200), std::chrono::milliseconds(300), std::chrono::system_clock::now());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.latest());
    EXPECT_EQ(std::chrono::microseconds(287500), rtt.smoothed());
}

TEST(rtt, smoothed_or_initial) {
    quicpp::congestion::rtt rtt;

    EXPECT_EQ(quicpp::congestion::default_initial_rtt, rtt.smoothed_or_initial());
    rtt.update(std::chrono::milliseconds(300), std::chrono::milliseconds(100), std::chrono::system_clock::now());
    EXPECT_EQ(std::chrono::milliseconds(300), rtt.smoothed_or_initial());
}

TEST(rtt, min_rtt) {
    quicpp::congestion::rtt rtt;

    rtt.update(std::chrono::milliseconds(200), std::chrono::milliseconds(0), std::chrono::system_clock::now());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.min());
    rtt.update(std::chrono::milliseconds(10), std::chrono::milliseconds(0), std::chrono::system_clock::now() += std::chrono::milliseconds(10));
    EXPECT_EQ(std::chrono::milliseconds(10), rtt.min());
    rtt.update(std::chrono::milliseconds(50), std::chrono::milliseconds(0), std::chrono::system_clock::now() += std::chrono::milliseconds(20));
    EXPECT_EQ(std::chrono::milliseconds(10), rtt.min());
    rtt.update(std::chrono::milliseconds(50), std::chrono::milliseconds(0), std::chrono::system_clock::now() += std::chrono::milliseconds(30));
    EXPECT_EQ(std::chrono::milliseconds(10), rtt.min());
    rtt.update(std::chrono::milliseconds(50), std::chrono::milliseconds(0), std::chrono::system_clock::now() += std::chrono::milliseconds(40));
    EXPECT_EQ(std::chrono::milliseconds(10), rtt.min());
    rtt.update(std::chrono::milliseconds(7), std::chrono::milliseconds(2), std::chrono::system_clock::now() += std::chrono::milliseconds(50));
    EXPECT_EQ(std::chrono::milliseconds(7), rtt.min());
}

TEST(rtt, expire_smoothed_metrics) {
    std::chrono::milliseconds initial_rtt(10);
    quicpp::congestion::rtt rtt;
    rtt.update(initial_rtt, std::chrono::milliseconds(0), std::chrono::system_clock::now());
    EXPECT_EQ(initial_rtt, rtt.min());
    EXPECT_EQ(initial_rtt, rtt.smoothed());
    EXPECT_EQ(initial_rtt / 2, rtt.mean_deviation());

    std::chrono::milliseconds double_rtt = initial_rtt * 2;
    rtt.update(double_rtt, std::chrono::milliseconds(0), std::chrono::system_clock::now());
    EXPECT_EQ(initial_rtt * 1.125, rtt.smoothed());

    rtt.expire_smoothed_metrics();
    EXPECT_EQ(double_rtt, rtt.smoothed());
    EXPECT_EQ(initial_rtt * 0.875, rtt.mean_deviation());

    std::chrono::milliseconds half_rtt = initial_rtt / 2;
    rtt.update(half_rtt, std::chrono::milliseconds(0), std::chrono::system_clock::now());
    EXPECT_GT(double_rtt, rtt.smoothed());
    EXPECT_LE(initial_rtt, rtt.mean_deviation());
}

TEST(rtt, update_rtt_with_bad_send_deltas) {
    std::chrono::milliseconds initial_rtt(10);
    quicpp::congestion::rtt rtt;
    rtt.update(initial_rtt, std::chrono::milliseconds(0), std::chrono::system_clock::now());
    EXPECT_EQ(initial_rtt, rtt.min());
    EXPECT_EQ(initial_rtt, rtt.smoothed());

    std::chrono::milliseconds bad[] = {
        std::chrono::milliseconds(0),
        std::chrono::milliseconds::max(),
        std::chrono::milliseconds(-1000)
    };

    for (int i = 0; i < 3; i++) {
        rtt.update(bad[i], std::chrono::milliseconds(0), std::chrono::system_clock::now());
        EXPECT_EQ(initial_rtt, rtt.min());
        EXPECT_EQ(initial_rtt, rtt.smoothed());
    }
}

TEST(rtt, reset_after_connection_migrations) {
    quicpp::congestion::rtt rtt;
    rtt.update(std::chrono::milliseconds(200), std::chrono::milliseconds(0), std::chrono::system_clock::now());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.latest());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.smoothed());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.min());
    rtt.update(std::chrono::milliseconds(300), std::chrono::milliseconds(100), std::chrono::system_clock::now());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.latest());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.smoothed());
    EXPECT_EQ(std::chrono::milliseconds(200), rtt.min());

    rtt.on_connection_migration();
    EXPECT_EQ(std::chrono::milliseconds(0), rtt.latest());
    EXPECT_EQ(std::chrono::milliseconds(0), rtt.smoothed());
    EXPECT_EQ(std::chrono::milliseconds(0), rtt.min());
}

int main() {
    return RUN_ALL_TESTS();
}
