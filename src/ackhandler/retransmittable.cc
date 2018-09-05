#include "ackhandler/retransmittable.h"
#include "ackhandler/packet.h"
#include <algorithm>

std::vector<quicpp::frame::frame *>
quicpp::ackhandler::
strip_non_retransmittable_frames(std::vector<quicpp::frame::frame *> &fs) {
    std::vector<quicpp::frame::frame *> result;
    std::for_each(fs.begin(), fs.end(),
                  [&] (quicpp::frame::frame *f) -> void {
                  if (quicpp::ackhandler::is_frame_retransmittable(*f)) {
                    result.push_back(f);
                  }
                  else {
                    delete f;
                  }
                  });
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
has_retransmittable_frames(std::vector<quicpp::frame::frame *> &fs) {
    for (auto iter = fs.begin(); iter != fs.end(); iter++) {
        if (quicpp::ackhandler::is_frame_retransmittable(**iter)) {
            return true;
        }
    }
    return false;
}
