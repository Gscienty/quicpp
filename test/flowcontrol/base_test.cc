#include "flowcontrol/base.h"
#include "params.h"
#include "gtest/gtest.h"

class special_base : public quicpp::flowcontrol::base {
private:
    quicpp::congestion::rtt rtt;
public:
    special_base() : quicpp::flowcontrol::base(0, 0, this->rtt) {}

    virtual uint64_t send_window() const override { return this->_send_window(); }
    virtual uint64_t update() override { return this->_update(); }
    virtual void maybe_update() override {  }

    uint64_t &ref_sent_bytes() { return this->sent_bytes; }
    uint64_t &ref_swnd() { return this->swnd; }

    uint64_t &ref_read_bytes() { return this->read_bytes; }
    uint64_t &ref_rwnd() { return this->rwnd; }
    uint64_t &ref_rwnd_size() { return this->rwnd_size; }
    uint64_t &ref_max_rwnd_size() { return this->max_rwnd_size; }
    uint64_t &ref_epoch_offset() { return this->epoch_start_offset; }
    std::chrono::system_clock::time_point &ref_epoch_time() {
        return this->epoch_start_time;
    }

    quicpp::congestion::rtt &ref_rtt() { return this->rtt; }
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

uint64_t rwnd = 10000;
uint64_t rwnd_size = 1000;

TEST(base_recv, add_bytes_read) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;

    ctr.ref_read_bytes() = 5;
    ctr.read(6);
    EXPECT_EQ(5 + 6, ctr.ref_read_bytes());
}

TEST(base_recv, triggers) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;

    double consumed = rwnd_size * quicpp::window_update_threshole + 1;
    uint64_t remain = rwnd_size - uint64_t(consumed);

    uint64_t pos = rwnd - remain;
    ctr.ref_read_bytes() = pos;
    EXPECT_EQ(pos + rwnd_size, ctr.update());
    EXPECT_EQ(pos + rwnd_size, ctr.ref_rwnd());
}

TEST(base_recv, not_triggers) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;
    
    double consumed = rwnd_size * quicpp::window_update_threshole - 1;
    uint64_t remain = rwnd_size - uint64_t(consumed);

    uint64_t pos = rwnd - remain;
    ctr.ref_read_bytes() = pos;
    EXPECT_EQ(0, ctr.update());
}

TEST(base_recv, not_increment) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    ctr.ref_max_rwnd_size() = 5000;

    ctr.maybe_adjust_window();
    EXPECT_EQ(old_wnd_size, ctr.ref_rwnd_size());
}

void set_rtt(special_base &ctr, std::chrono::microseconds t) {
    ctr.ref_rtt().update(t, 
                         std::chrono::microseconds(0),
                         std::chrono::system_clock::now());
    EXPECT_EQ(t, ctr.ref_rtt().smoothed());
}

TEST(base_recv, not_increase_no_rtt) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    ctr.ref_max_rwnd_size() = 5000;

    set_rtt(ctr, std::chrono::microseconds(0));
    ctr.start_new_auto_tuning_epoch();
    ctr.read(400);
    uint64_t off = ctr.update();
    EXPECT_NE(0, off);
    EXPECT_EQ(old_wnd_size, ctr.ref_rwnd_size());
    
}

TEST(base_recv, increase) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    ctr.ref_max_rwnd_size() = 5000;

    uint64_t read_bytes = ctr.ref_read_bytes();
    std::chrono::microseconds rtt(std::chrono::milliseconds(20));
    set_rtt(ctr, rtt);

    uint64_t data_read = rwnd_size * 2 / 3 + 1;
    ctr.ref_epoch_offset() = ctr.ref_read_bytes();
    ctr.ref_epoch_time() = std::chrono::system_clock::now() + (-rtt * 4 * 2 / 3);
    ctr.read(data_read);
    uint64_t off = ctr.update();
    EXPECT_NE(0, off);
    uint64_t new_wnd_size = ctr.ref_rwnd_size();
    EXPECT_EQ(2 * old_wnd_size, new_wnd_size);
    EXPECT_EQ(read_bytes + data_read + new_wnd_size, off);
}

TEST(base_recv, not_increment1) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    ctr.ref_max_rwnd_size() = 5000;

    uint64_t bytes_read = ctr.ref_read_bytes();
    std::chrono::milliseconds rtt(20);
    set_rtt(ctr, rtt);
    uint64_t data_read = rwnd_size * 1 / 3 + 1;
    ctr.ref_epoch_offset() = ctr.ref_read_bytes();
    ctr.ref_epoch_time() = std::chrono::system_clock::now() + (-rtt * 4 * 1 / 3);
    ctr.read(data_read);
    uint64_t off = ctr.update();
    EXPECT_NE(0, off);
    uint64_t new_wnd_size = ctr.ref_rwnd_size();
    EXPECT_EQ(old_wnd_size, new_wnd_size);
    EXPECT_EQ(bytes_read + data_read + new_wnd_size, off);
}

TEST(base_recv, not_increment2) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    ctr.ref_max_rwnd_size() = 5000;

    
    uint64_t bytes_read = ctr.ref_read_bytes();
    std::chrono::milliseconds rtt(std::chrono::seconds(20));
    set_rtt(ctr, rtt);
    uint64_t data_read = rwnd_size * 2 / 3 - 1;

    ctr.ref_epoch_offset() = ctr.ref_read_bytes();
    ctr.ref_epoch_time() = std::chrono::system_clock::now() + (-rtt * 4 * 2 / 3);
    ctr.read(data_read);
    uint64_t off = ctr.update();
    EXPECT_NE(0, off);
    EXPECT_EQ(old_wnd_size, ctr.ref_rwnd_size());
    EXPECT_EQ(bytes_read + data_read + old_wnd_size, off);
}

void reset_epoch(special_base &ctr) {
    ctr.ref_epoch_time() = std::chrono::system_clock::now() + (-std::chrono::milliseconds(1));
    ctr.ref_epoch_offset() = ctr.ref_read_bytes();
    ctr.read(ctr.ref_rwnd_size() / 2 + 1);
}

TEST(base_recv, not_increment3) {
    special_base ctr;
    ctr.ref_read_bytes() = rwnd - rwnd_size;
    ctr.ref_rwnd() = rwnd;
    ctr.ref_rwnd_size() = rwnd_size;
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    ctr.ref_max_rwnd_size() = 5000;
    
    set_rtt(ctr, std::chrono::seconds(20));
    reset_epoch(ctr);
    ctr.maybe_adjust_window();
    EXPECT_EQ(2 * old_wnd_size, ctr.ref_rwnd_size());
    
    reset_epoch(ctr);
    ctr.maybe_adjust_window();
    EXPECT_EQ(2 * 2 * old_wnd_size, ctr.ref_rwnd_size());

    reset_epoch(ctr);
    ctr.maybe_adjust_window();
    EXPECT_EQ(ctr.ref_max_rwnd_size(), ctr.ref_rwnd_size());
    ctr.maybe_adjust_window();
    EXPECT_EQ(ctr.ref_max_rwnd_size(), ctr.ref_rwnd_size());
}

int main() {
    return RUN_ALL_TESTS();
}
