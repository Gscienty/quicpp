#include "congestion/slow_start.h"
#include "gtest/gtest.h"

TEST(slowstart, works_in_a_simple_case) {
    using namespace quicpp::congestion;

    slow_start slowstart;

    uint64_t packet_number = 1;
    uint64_t end_packet_number = 3;
    slowstart.start_receive_round(end_packet_number);

    packet_number++;
    EXPECT_FALSE(slowstart.is_end_of_round(packet_number));
    EXPECT_FALSE(slowstart.is_end_of_round(packet_number));
    packet_number++;
    EXPECT_FALSE(slowstart.is_end_of_round(packet_number));
    packet_number++;
    EXPECT_TRUE(slowstart.is_end_of_round(packet_number));

    packet_number++;
    EXPECT_TRUE(slowstart.is_end_of_round(packet_number));

    end_packet_number = 20;
    slowstart.start_receive_round(end_packet_number);
    while (packet_number < end_packet_number) {
        packet_number++;
        EXPECT_FALSE(slowstart.is_end_of_round(packet_number));
    }

    packet_number++;
    EXPECT_TRUE(slowstart.is_end_of_round(packet_number));
}

TEST(slowstart, works_with_delay) {
    using namespace quicpp::congestion;

    slow_start slowstart;

    std::chrono::milliseconds rtt(60);
    int hybrid_start_min_samples = 8;

    uint64_t end_packet_number = 1;
    end_packet_number++;
    slowstart.start_receive_round(end_packet_number);

    for (int n = 0; n < hybrid_start_min_samples; n++) {
        EXPECT_FALSE(slowstart.should_exist_slowstart(rtt +
                                                      std::chrono::milliseconds(n),
                                                      rtt,
                                                      100));
    }
    end_packet_number++;
    slowstart.start_receive_round(end_packet_number);
    for (int n = 1; n < hybrid_start_min_samples; n++) {
        EXPECT_FALSE(slowstart.should_exist_slowstart(rtt +
                                                      std::chrono::milliseconds(n + 10),
                                                      rtt,
                                                      100));
    }
    EXPECT_TRUE(slowstart.should_exist_slowstart(rtt + std::chrono::milliseconds(10),
                                                 rtt,
                                                 100));
}

int main() {
    return RUN_ALL_TESTS();
}
