#include "flowcontrol/connection.h"
#include "gtest/gtest.h"

class spec_flow : public quicpp::flowcontrol::connection {
public:
    spec_flow(uint64_t rwnd,
              uint64_t max_rwnd,
              std::function<void ()> update_func,
              quicpp::congestion::rtt &rtt)
        : quicpp::flowcontrol::connection(rwnd, max_rwnd, update_func, rtt) {}
    
    uint64_t &ref_rwnd() { return this->rwnd; }
    uint64_t &ref_max_rwnd_size() { return this->max_rwnd_size; }
    uint64_t &ref_highest_received() { return this->highest_received; }
    uint64_t &ref_rwnd_size() { return this->rwnd_size; }
    uint64_t &ref_read_bytes() { return this->read_bytes; }
    quicpp::congestion::rtt &ref_rtt() { return this->rtt; }
    std::chrono::system_clock::time_point &ref_epoch_time() {
        return this->epoch_start_time;
    }
    uint64_t &ref_epoch_offset() { return this->epoch_start_offset; }
};

TEST(connection_flowcontrol, constructor) {
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd, max_rwnd, [] () -> void {}, rtt);

    EXPECT_EQ(rwnd, ctr.ref_rwnd());
    EXPECT_EQ(max_rwnd, ctr.ref_max_rwnd_size());
}

TEST(connection_flowcontrol, receive_flowcontrol) {
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd, max_rwnd, [] () -> void {}, rtt);

    ctr.ref_highest_received() = 1337;
    ctr.increment_highest_received(123);
    EXPECT_EQ(1337 + 123, ctr.ref_highest_received());
}

TEST(connection_flowcontrol, queues_window_updates) {
    bool queued_window_update = false;
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd,
                  max_rwnd,
                  [&] () -> void { queued_window_update = true; },
                  rtt);
    ctr.ref_rwnd() = 100;
    ctr.ref_rwnd_size() = 60;
    ctr.ref_max_rwnd_size() = 1000;
    ctr.ref_read_bytes() = 100 - 60;

    ctr.maybe_update();
    EXPECT_FALSE(queued_window_update);
    ctr.read(30);
    ctr.maybe_update();
    EXPECT_TRUE(queued_window_update);
    EXPECT_NE(0, ctr.update());
    queued_window_update = false;
    ctr.maybe_update();
    EXPECT_FALSE(queued_window_update);
}

TEST(connection_flowcontrol, get_a_window_update) {
    bool queued_window_update = false;
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd,
                  max_rwnd,
                  [&] () -> void { queued_window_update = true; },
                  rtt);
    ctr.ref_rwnd() = 100;
    ctr.ref_rwnd_size() = 60;
    ctr.ref_max_rwnd_size() = 1000;
    ctr.ref_read_bytes() = 100 - 60;

    uint64_t wnd_size = ctr.ref_rwnd_size();
    uint64_t old_off = ctr.ref_read_bytes();
    uint64_t data_read = wnd_size / 2 - 1;

    ctr.read(data_read);
    uint64_t off = ctr.update();
    EXPECT_EQ(old_off + data_read + 60, off);
}

void set_rtt(spec_flow &ctr, std::chrono::milliseconds t) {
    ctr.ref_rtt().update(t, 
                         std::chrono::milliseconds(0),
                         std::chrono::system_clock::now());
    EXPECT_EQ(t, ctr.ref_rtt().smoothed());
}

TEST(connection_flowcontrol, autotunes_the_window) {
    bool queued_window_update = false;
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd,
                  max_rwnd,
                  [&] () -> void { queued_window_update = true; },
                  rtt);
    ctr.ref_rwnd() = 100;
    ctr.ref_rwnd_size() = 60;
    ctr.ref_max_rwnd_size() = 1000;
    ctr.ref_read_bytes() = 100 - 60;

    uint64_t old_off = ctr.ref_read_bytes();
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    set_rtt(ctr, std::chrono::milliseconds(20));
    ctr.ref_epoch_offset() = old_off;
    ctr.ref_epoch_time() = std::chrono::system_clock::now() - std::chrono::milliseconds(1);
    uint64_t data_read = old_wnd_size / 2 + 1;
    ctr.read(data_read);
    uint64_t off = ctr.update();
    uint64_t new_wnd_size = ctr.ref_rwnd_size();
    EXPECT_EQ(2 * old_wnd_size, new_wnd_size);
    EXPECT_EQ(old_off + data_read + new_wnd_size, off);
}

TEST(connection_flowcontrol, set_minimum) {
    bool queued_window_update = false;
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd,
                  max_rwnd,
                  [&] () -> void { queued_window_update = true; },
                  rtt);
    uint64_t rwnd_ = 10000;
    uint64_t rwnd_size = 1000;
    ctr.ref_rwnd() = rwnd_;
    ctr.ref_rwnd_size() = rwnd_size;
    ctr.ref_max_rwnd_size() = 3000;

    ctr.ensure_min_wnd(1800);
    EXPECT_EQ(1800, ctr.ref_rwnd_size());
}

TEST(connection_flowcontrol, not_reduce) {
    bool queued_window_update = false;
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd,
                  max_rwnd,
                  [&] () -> void { queued_window_update = true; },
                  rtt);
    uint64_t rwnd_ = 10000;
    uint64_t rwnd_size = 1000;
    ctr.ref_rwnd() = rwnd_;
    ctr.ref_rwnd_size() = rwnd_size;
    uint64_t old_wnd_size = ctr.ref_rwnd_size();
    ctr.ref_max_rwnd_size() = 3000;

    ctr.ensure_min_wnd(1);
    EXPECT_EQ(old_wnd_size, ctr.ref_rwnd_size());
}

TEST(connection_flowcontrol, not_increase) {
    bool queued_window_update = false;
    quicpp::congestion::rtt rtt;
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    spec_flow ctr(rwnd,
                  max_rwnd,
                  [&] () -> void { queued_window_update = true; },
                  rtt);
    uint64_t rwnd_ = 10000;
    uint64_t rwnd_size = 1000;
    ctr.ref_rwnd() = rwnd_;
    ctr.ref_rwnd_size() = rwnd_size;
    ctr.ref_max_rwnd_size() = 3000;

    uint64_t max = ctr.ref_max_rwnd_size();
    ctr.ensure_min_wnd(2 * max);
    EXPECT_EQ(max, ctr.ref_rwnd_size());
}

int main() {
    return RUN_ALL_TESTS();
}
