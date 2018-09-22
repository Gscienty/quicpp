#include "ackhandler/retransmittable.h"
#include "ackhandler/packet.h"
#include <algorithm>

std::vector<std::shared_ptr<quicpp::frame::frame>>
quicpp::ackhandler::
strip_non_retransmittable_frames(std::vector<std::shared_ptr<quicpp::frame::frame>> &fs) {
    std::vector<std::shared_ptr<quicpp::frame::frame>> result;
    for (auto f = fs.begin(); f != fs.end();) {
        if (quicpp::ackhandler::is_frame_retransmittable(**f)) {
            result.push_back(*f);
            f++;
        }
        else {
            f = fs.erase(f);
        }
    }
    return result;
}

bool
quicpp::ackhandler::
is_frame_retransmittable(quicpp::frame::frame &fs) {
    switch (fs.type()) {
    case quicpp::frame::frame_type_ack:
        return false;
    default:
        return true;
    }
}

bool
quicpp::ackhandler::
has_retransmittable_frames(std::vector<std::shared_ptr<quicpp::frame::frame>> &fs) {
    for (auto iter = fs.begin(); iter != fs.end(); iter++) {
        if (quicpp::ackhandler::is_frame_retransmittable(**iter)) {
            return true;
        }
    }
    return false;
}
