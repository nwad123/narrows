#pragma once

#include <iterator>

namespace nrws {
template <typename I>
concept channel_iterator = std::input_iterator<I>;
} // namespace nrws
