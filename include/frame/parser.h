#ifndef _QUICPP_FRAME_PARSER_
#define _QUICPP_FRAME_PARSER_

#include "frame/type.h"
#include <memory>

namespace quicpp {
namespace frame {

std::shared_ptr<quicpp::frame::frame>
parse_next_frame(std::basic_istream<uint8_t> &reader);

}
}

#endif
