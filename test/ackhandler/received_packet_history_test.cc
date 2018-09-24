#include "ackhandler/received_packet_handler.h"
#include "gtest/gtest.h"
#include <utility>

TEST(ranges, add_first_packet) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    
    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(4UL, 4UL), hist.ranges().front());
}

TEST(ranges, add_few_consecutive_packets) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(5);
    hist.received_packet(6);
    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(4UL, 6UL), hist.ranges().front());
}

TEST(ranges, dosent_care_duplicate_packet) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(5);
    hist.received_packet(6);
    hist.received_packet(5);

    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(4UL, 6UL), hist.ranges().front());
}

TEST(ranges, extends_range_at_front) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(3);
    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(3UL, 4UL), hist.ranges().front());
}

TEST(ranges, lost) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(6);
    EXPECT_EQ(2, hist.ranges().size());
    EXPECT_EQ(std::make_pair(4UL, 4UL), hist.ranges().front());
    EXPECT_EQ(std::make_pair(6UL, 6UL), hist.ranges().back());
}

TEST(ranges, between_two_ranges) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(10);
    EXPECT_EQ(2, hist.ranges().size());
    hist.received_packet(7);
    EXPECT_EQ(3, hist.ranges().size());
    auto itr = hist.ranges().begin();
    EXPECT_EQ(std::make_pair(4UL, 4UL), *itr++);
    EXPECT_EQ(std::make_pair(7UL, 7UL), *itr++);
    EXPECT_EQ(std::make_pair(10UL, 10UL), *itr);
}

TEST(ranges, extend_previous) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(7);
    hist.received_packet(5);

    EXPECT_EQ(2, hist.ranges().size());
    auto itr = hist.ranges().begin();
    EXPECT_EQ(std::make_pair(4UL, 5UL), *itr++);
    EXPECT_EQ(std::make_pair(7UL, 7UL), *itr);
}

TEST(ranges, extend_front) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(7);
    hist.received_packet(6);

    EXPECT_EQ(2, hist.ranges().size());
    auto itr = hist.ranges().begin();
    EXPECT_EQ(std::make_pair(4UL, 4UL), *itr++);
    EXPECT_EQ(std::make_pair(6UL, 7UL), *itr);
}

TEST(ranges, close_range) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(6);

    EXPECT_EQ(2, hist.ranges().size());
    hist.received_packet(5);
    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(4UL, 6UL), hist.ranges().front());
}

TEST(ranges, close_middle) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(1);
    hist.received_packet(10);
    hist.received_packet(4);
    hist.received_packet(6);
    EXPECT_EQ(4, hist.ranges().size());
    hist.received_packet(5);
    EXPECT_EQ(3, hist.ranges().size());
    auto itr = hist.ranges().begin();
    EXPECT_EQ(std::make_pair(1UL, 1UL), *itr++);
    EXPECT_EQ(std::make_pair(4UL, 6UL), *itr++);
    EXPECT_EQ(std::make_pair(10UL, 10UL), *itr);
}

TEST(deleting, dose_nothing_when_empty) {
    quicpp::ackhandler::received_packet_history hist;
    hist.delete_below(5);
    EXPECT_TRUE(hist.ranges().empty());
}

TEST(deleting, deletes_multiple_ranges) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(1);
    hist.received_packet(5);
    hist.received_packet(10);
    hist.delete_below(8);
    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(10UL, 10UL), hist.ranges().front());
}

TEST(deleting, adjust_range) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(3);
    hist.received_packet(4);
    hist.received_packet(5);
    hist.received_packet(6);
    hist.received_packet(7);
    hist.delete_below(5);
    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(5UL, 7UL), hist.ranges().front());
}

TEST(deleting, adjust_range_1) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(5);
    hist.received_packet(7);
    hist.delete_below(5);
    EXPECT_EQ(2, hist.ranges().size());
    EXPECT_EQ(std::make_pair(5UL, 5UL), hist.ranges().front());
    EXPECT_EQ(std::make_pair(7UL, 7UL), hist.ranges().back());
}

TEST(deleting, keep) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.delete_below(4);

    EXPECT_EQ(1, hist.ranges().size());
    EXPECT_EQ(std::make_pair(4UL, 4UL), hist.ranges().front());
}

TEST(ack, export) {
    quicpp::ackhandler::received_packet_history hist;
    auto ack = hist.get_ack_ranges();
    EXPECT_TRUE(ack.empty());
}

TEST(ack, single_ack_range) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(5);
    auto ack = hist.get_ack_ranges();
    EXPECT_EQ(1, ack.size());
    EXPECT_EQ(std::make_pair(4UL, 5UL), ack[0]);
}

TEST(ack, multiple_ack) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(5);
    hist.received_packet(6);
    hist.received_packet(1);
    hist.received_packet(11);
    hist.received_packet(10);
    hist.received_packet(2);

    auto ack = hist.get_ack_ranges();

    EXPECT_EQ(3, ack.size());
    EXPECT_EQ(std::make_pair(10UL, 11UL), ack[0]);
    EXPECT_EQ(std::make_pair(4UL, 6UL), ack[1]);
    EXPECT_EQ(std::make_pair(1UL, 2UL), ack[2]);
}

TEST(highest_ack, zero) {
    quicpp::ackhandler::received_packet_history hist;
    EXPECT_EQ(std::make_pair(0UL, 0UL), hist.get_highest_ack_range());
}

TEST(highest_ack, single_ack) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(4);
    hist.received_packet(5);

    EXPECT_EQ(std::make_pair(4UL, 5UL), hist.get_highest_ack_range());
}

TEST(highest_ack, multiple_ack) {
    quicpp::ackhandler::received_packet_history hist;
    hist.received_packet(3);
    hist.received_packet(6);
    hist.received_packet(7);

    EXPECT_EQ(std::make_pair(6UL, 7UL), hist.get_highest_ack_range());
}

int main() {
    return RUN_ALL_TESTS();
}
