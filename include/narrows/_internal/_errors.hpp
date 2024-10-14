#pragma once

#include <cstdint>

namespace nrws {
enum class error_id : uint8_t {
    channel_full,
    channel_disconnected,
};

}
