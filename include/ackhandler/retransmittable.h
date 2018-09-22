#ifndef _QUICPP_ACKHANDLER_RETRANSMITTABLE_
#define _QUICPP_ACKHANDLER_RETRANSMITTABLE_

#include "frame/type.h"
#include <vector>
#include <memory>

namespace quicpp {
namespace ackhandler {

std::vector<std::shared_ptr<quicpp::frame::frame>>
strip_non_retransmittable_frames(std::vector<std::shared_ptr<quicpp::frame::frame>> &fs);

bool is_frame_retransmittable(quicpp::frame::frame &fs);

bool has_retransmittable_frames(std::vector<std::shared_ptr<quicpp::frame::frame>> &fs);

}
}

#endif
