#include "ackhandler/received_packet_handler.h"
#include "params.h"
#include "base/error.h"
#include "gtest/gtest.h"

TEST(accepting_packets, handlers_packet_arrives_late) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    auto ret = handler.received_packet(1, std::chrono::system_clock::now(), true);
    EXPECT_EQ(ret, quicpp::error::success);
    ret = handler.received_packet(3, std::chrono::system_clock::now(), true);
    EXPECT_EQ(ret, quicpp::error::success);
    ret = handler.received_packet(2, std::chrono::system_clock::now(), true);
    EXPECT_EQ(ret, quicpp::error::success);
}

TEST(accepting_packets, updates_largest_observed) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    auto now = std::chrono::system_clock::now();
    handler.largest_observed() = 3;
    handler.largest_observed_received_time() = now - std::chrono::seconds(1);
    auto err = handler.received_packet(5, now, true);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_EQ(5, handler.largest_observed());
    EXPECT_EQ(now, handler.largest_observed_received_time());
}

TEST(accepting_packets, dosent_update_largest_observed) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    auto now = std::chrono::system_clock::now();
    auto timestamp = now - std::chrono::seconds(1);
    handler.largest_observed() = 5;
    handler.largest_observed_received_time() = timestamp;
    auto err = handler.received_packet(4, now, true);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_EQ(5, handler.largest_observed());
    EXPECT_EQ(timestamp, handler.largest_observed_received_time());
}

TEST(accepting_packets, passes_on_errors) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    
    quicpp::base::error_t err;
    for (int i = 0; i < 5 * quicpp::max_tracked_received_ack_ranges; i++) {
        err = handler.received_packet(2 * i + 1,
                                      std::chrono::system_clock::time_point::min(),
                                      true);

        if (err != quicpp::error::success) {
            break;
        }
    }

    EXPECT_NE(err, quicpp::error::success);
}

TEST(ack, always_queue_first_packet) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    
    auto err = handler.received_packet(1,
                                       std::chrono::system_clock::now(),
                                       false);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_TRUE(handler.ack_queued());
    EXPECT_EQ(std::chrono::system_clock::time_point::min(),
              handler.alarm_timeout());
}

TEST(ack, works_packetnumber_0) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    auto err = handler.received_packet(0, std::chrono::system_clock::now(), false);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_TRUE(handler.ack_queued());
    EXPECT_EQ(std::chrono::system_clock::time_point::min(),
              handler.alarm_timeout());
}

void receive_and_ack_10_packet(quicpp::ackhandler::received_packet_handler &handler) {
    for (int i = 1; i <= 10; i++) {
        auto err = handler.received_packet(i,
                                           std::chrono::system_clock::now(),
                                           true);
        EXPECT_EQ(err, quicpp::error::success);
    }
    EXPECT_TRUE(bool(handler.get_ack_frame()));
    EXPECT_FALSE(handler.ack_queued());
}

TEST(ack, queue_ack_for_every_second) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    
    receive_and_ack_10_packet(handler);
    uint64_t p = 11;
    for (int i = 0; i <= 20; i++) {
        auto err = handler.received_packet(p,
                                           std::chrono::system_clock::now(),
                                           true);
        EXPECT_EQ(err, quicpp::error::success);
        EXPECT_FALSE(handler.ack_queued());
        p++;
        err = handler.received_packet(p,
                                      std::chrono::system_clock::now(),
                                      true);
        EXPECT_TRUE(handler.ack_queued());
        p++;
        EXPECT_TRUE(bool(handler.get_ack_frame()));
    }
}

TEST(ack, queue_ack_for_every_second_1) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    
    receive_and_ack_10_packet(handler);
    uint64_t p = 10000;
    for (int i = 0; i < 9; i++) {
        auto err = handler.received_packet(p,
                                           std::chrono::system_clock::now(),
                                           true);
        EXPECT_EQ(err, quicpp::error::success);
        EXPECT_FALSE(handler.ack_queued());
        p++;
    }
    EXPECT_NE(std::chrono::system_clock::time_point::min(),
              handler.alarm_timeout());
    auto err = handler.received_packet(p,
                                       std::chrono::system_clock::now(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_TRUE(handler.ack_queued());
    EXPECT_EQ(std::chrono::system_clock::time_point::min(),
              handler.alarm_timeout());
}

TEST(ack, only_sets_timer) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    receive_and_ack_10_packet(handler);
    auto err = handler.received_packet(11,
                                       std::chrono::system_clock::now(),
                                       false);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_FALSE(handler.ack_queued());
    EXPECT_EQ(std::chrono::system_clock::time_point::min(),
              handler.alarm_timeout());
    auto rcv_time = std::chrono::system_clock::now() +
        std::chrono::milliseconds(10);
    err = handler.received_packet(12, rcv_time, true);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_FALSE(handler.ack_queued());
    EXPECT_LE(rcv_time, handler.alarm_timeout());
}

TEST(ack, queues_an_ack) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    receive_and_ack_10_packet(handler);
    auto err = handler.received_packet(11,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    err = handler.received_packet(13,
                                  std::chrono::system_clock::time_point::min(),
                                  true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_TRUE(bool(ack));
    EXPECT_TRUE(ack->has_missing_ranges());
    EXPECT_FALSE(handler.ack_queued());
    err = handler.received_packet(12,
                                  std::chrono::system_clock::time_point::min(),
                                  false);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_TRUE(handler.ack_queued());
}

TEST(ack, dosent_queue_an_ack) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    receive_and_ack_10_packet(handler);

    auto err = handler.received_packet(12,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    err = handler.received_packet(13,
                                  std::chrono::system_clock::time_point::min(),
                                  true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_TRUE(bool(ack));
    handler.ignore_below(12);
    err = handler.received_packet(11,
                                  std::chrono::system_clock::time_point::min(),
                                  false);
    EXPECT_EQ(err, quicpp::error::success);
    ack = handler.get_ack_frame();
    EXPECT_FALSE(bool(ack));
}

void recieved_and_ack_until_ack_decimation(quicpp::ackhandler::received_packet_handler &handler) {
    for (int i = 1; i <= quicpp::ackhandler::min_received_before_ack_decimation; i++) {
        auto err = handler.received_packet(i,
                                           std::chrono::system_clock::time_point::min(),
                                           true);
        EXPECT_EQ(err, quicpp::error::success);
    }
    EXPECT_TRUE(bool(handler.get_ack_frame()));
    EXPECT_FALSE(handler.ack_queued());
}

TEST(ack, dosent_queue_an_ack_1) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    recieved_and_ack_until_ack_decimation(handler);
    uint64_t p = quicpp::ackhandler::min_received_before_ack_decimation + 1;
    auto err = handler.received_packet(p + 1,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_FALSE(handler.ack_queued());
    EXPECT_EQ(std::chrono::system_clock::time_point::min(),
              handler.alarm_timeout());
    err = handler.received_packet(p,
                                  std::chrono::system_clock::now(),
                                  true);
    EXPECT_EQ(err, quicpp::error::success);
    EXPECT_FALSE(handler.ack_queued());
}

TEST(ack, set_ack) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);

    auto now = std::chrono::system_clock::now() - std::chrono::hours(1);
    auto p = quicpp::ackhandler::min_received_before_ack_decimation + 1;
    for (int i = p; i < p + 6; i++) {
        auto err = handler.received_packet(i, now, true);
        EXPECT_EQ(err, quicpp::error::success);
    }
    auto err = handler.received_packet(p + 10, now, true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_TRUE(ack->has_missing_ranges());
}

TEST(ack_gener, generates_simple_ack_frame) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    handler.ack_queued() = true;

    auto err = handler.received_packet(1,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    err = handler.received_packet(2,
                                  std::chrono::system_clock::time_point::min(),
                                  true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_TRUE(bool(ack));
    EXPECT_EQ(2, ack->largest());
    EXPECT_EQ(1, ack->smallest());
    EXPECT_FALSE(ack->has_missing_ranges());
}

TEST(ack_gener, generate_ack_pnum_0) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    handler.ack_queued() = true;

    auto err = handler.received_packet(0,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_TRUE(bool(ack));
    EXPECT_EQ(0, ack->largest());
    EXPECT_EQ(0, ack->smallest());
    EXPECT_FALSE(ack->has_missing_ranges());
}

TEST(ack_gener, saves_last_sent_ack) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    handler.ack_queued() = true;
    
    auto err = handler.received_packet(1,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_TRUE(bool(ack));
    EXPECT_EQ(*ack, *handler.last_ack());
    err = handler.received_packet(2,
                                  std::chrono::system_clock::time_point::min(),
                                  true);
    EXPECT_EQ(err, quicpp::error::success);
    handler.ack_queued() = true;
    ack = handler.get_ack_frame();
    EXPECT_TRUE(bool(ack));
    EXPECT_EQ(*ack, *handler.last_ack());
}

TEST(ack_gener, generates_missing_packets) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    handler.ack_queued() = true;

    auto err = handler.received_packet(1,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    err = handler.received_packet(4,
                                  std::chrono::system_clock::time_point::min(),
                                  true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_TRUE(bool(ack));
    EXPECT_EQ(4, ack->largest());
    EXPECT_EQ(1, ack->smallest());
    EXPECT_EQ(std::make_pair(4UL, 4UL), ack->ranges().front());
    EXPECT_EQ(std::make_pair(1UL, 1UL), ack->ranges().back());
}

TEST(ack_gener, generate_pnum0) {
    quicpp::congestion::rtt rtt;
    quicpp::ackhandler::received_packet_handler handler(rtt);
    handler.ack_queued() = true;

    auto err = handler.received_packet(0,
                                       std::chrono::system_clock::time_point::min(),
                                       true);
    EXPECT_EQ(err, quicpp::error::success);
    err = handler.received_packet(1,
                                 std::chrono::system_clock::time_point::min(),
                                 true);
    EXPECT_EQ(err, quicpp::error::success);
    err = handler.received_packet(3,
                                  std::chrono::system_clock::time_point::min(),
                                  true);
    EXPECT_EQ(err, quicpp::error::success);
    auto ack = handler.get_ack_frame();
    EXPECT_EQ(3, ack->largest());
    EXPECT_EQ(0, ack->smallest());

    EXPECT_EQ(std::make_pair(3UL, 3UL), ack->ranges().front());
    EXPECT_EQ(std::make_pair(0UL, 1UL), ack->ranges().back());
}

int main() {
    return RUN_ALL_TESTS();
}
