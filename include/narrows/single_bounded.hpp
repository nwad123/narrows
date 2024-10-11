#pragma once

#include "concepts.hpp"

#include <expected>
#include <type_traits>

namespace nrws {

enum class sender_error_t : uint8_t { ChannelClosed, ChannelFull };
enum class receiver_error_t : uint8_t { ChannelClosed, ChannelFull };

template<typename T>
class Sender
{
  public:
    using value_type = std::decay_t<T>;
    using error_type = sender_error_t;
    using result_type = std::expected<void, error_type>;

    Sender() = default;

    [[nodiscard]] inline auto send(const value_type &val) -> result_type;
    [[nodiscard]] inline auto send(value_type &&val) -> result_type;
};
static_assert(is_sender<Sender<int>>, "Must satisfy the sender concept.");

template<typename T>
class Receiver {
public:
    using value_type = std::decay_t<T>;
    using error_type = sender_error_t;
    using result_type = std::expected<value_type, error_type>;
   
    Receiver() = default;

    [[nodiscard]] inline auto receive() -> result_type;
};
static_assert(is_receiver<Receiver<int>>, "Must satisfy the receiver concept");

};// namespace nrws
