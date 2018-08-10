#include "flowcontrol/base.h"
#include "gtest/gtest.h"

class special_base : public quicpp::flowcontrol::base {
private:
    quicpp::congestion::rtt rtt;
public:
    special_base() : quicpp::flowcontrol::base(0, 0, this->rtt) {}

    virtual uint64_t send_window() const override { return this->_send_window(); }
    virtual uint64_t update() override { return 0; }
    virtual void maybe_update() override {  }

    uint64_t &ref_sent_bytes() { return this->sent_bytes; }
    uint64_t &ref_swnd() { return this->swnd; }
};

TEST(base, add_bytes_sent) {
    special_base controller;
    controller.ref_sent_bytes() = 5;
    controller.sent(6);

    EXPECT_EQ(5 + 6, controller.ref_sent_bytes());
}

TEST(base, get_remaining_flow_control_window) {
    special_base controller;
    controller.ref_swnd() = 12;
    controller.ref_sent_bytes() = 5;

    EXPECT_EQ(12 - 5, controller.send_window());
}

TEST(base, update_size_flow_control_window) {
    special_base controller;
    controller.sent(5);
    controller.base::send_window(15);

    EXPECT_EQ(15, controller.ref_swnd());
    EXPECT_EQ(15 - 5, controller.send_window());
}

TEST(base, window_zero) {
    special_base controller;
    controller.sent(15);
    controller.base::send_window(10);

    EXPECT_EQ(0, controller.send_window());
}

TEST(base, dont_decrease_window) {
    special_base controller;

    controller.base::send_window(20);
    EXPECT_EQ(20, controller.send_window());
    controller.base::send_window(10);
    EXPECT_EQ(20, controller.send_window());
}

TEST(base, blocked) {
    special_base controller;

    controller.base::send_window(100);
    EXPECT_FALSE(controller.is_newly_blocked().first);
    controller.sent(100);
    bool blocked;
    uint64_t offset;
    std::tie(blocked, offset) = controller.is_newly_blocked();
    EXPECT_TRUE(blocked);
    EXPECT_EQ(100, offset);
}

TEST(base, blocked2) {
    special_base controller;

    controller.base::send_window(100);
    controller.sent(100);
    bool blocked;
    uint64_t offset;
    std::tie(blocked, offset) = controller.is_newly_blocked();
    EXPECT_TRUE(blocked);
    EXPECT_EQ(100, offset);

    EXPECT_FALSE(controller.is_newly_blocked().first);


    controller.base::send_window(150);
    controller.sent(150);
    EXPECT_TRUE(controller.is_newly_blocked().first);
}

int main() {
    return RUN_ALL_TESTS();
}
