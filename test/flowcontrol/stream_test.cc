#include "flowcontrol/stream.h"
#include "gtest/gtest.h"

class spec_flow : public quicpp::flowcontrol::stream {
public:
    spec_flow(quicpp::base::stream_id_t stream_id,
           bool contributes_to_connection,
           quicpp::flowcontrol::connection &connection,
           uint64_t rwnd,
           uint64_t max_rwnd,
           uint64_t swnd,
           std::function<void (const quicpp::base::stream_id_t &)> update_func,
           quicpp::congestion::rtt &rtt)
        : quicpp::flowcontrol::stream(stream_id,
                                      contributes_to_connection,
                                      connection,
                                      rwnd,
                                      max_rwnd,
                                      swnd,
                                      update_func,
                                      rtt) {}

    uint64_t &ref_max_rwnd_size() { return this->max_rwnd_size; }
    uint64_t &ref_rwnd() { return this->rwnd; }
    uint64_t &ref_swnd() { return this->swnd; }
    bool &ref_contr_conn() { return this->contributes_to_connection; }
    quicpp::base::stream_id_t &ref_stream_id() { return this->stream_id; }
};

TEST(stream_constructor, set_send_recv) {
    uint64_t rwnd = 2000;
    uint64_t max_rwnd = 3000;
    uint64_t swnd = 4000;
    quicpp::congestion::rtt rtt;
    bool queued_window_update = false;
    bool queued_conn_window_update = false;
    quicpp::flowcontrol::connection conn_ctr(1000, 1000,
                                             [&] () -> void {
                                             queued_conn_window_update = true;
                                             },
                                             rtt);
    spec_flow ctr(quicpp::base::stream_id_t(10),
                  true, conn_ctr, rwnd, max_rwnd, swnd,
                  [&] (const quicpp::base::stream_id_t &) -> void {              
                  queued_window_update = true;
                  },
                  rtt);

    EXPECT_EQ(10, ctr.ref_stream_id());
    EXPECT_EQ(rwnd, ctr.ref_rwnd());
    EXPECT_EQ(max_rwnd, ctr.ref_max_rwnd_size());
    EXPECT_EQ(swnd, ctr.ref_swnd());
    EXPECT_TRUE(ctr.ref_contr_conn());
}

int main() {
    return RUN_ALL_TESTS();
}
