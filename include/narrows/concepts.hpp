#pragma once

#include <__expected/expected.h>
#include <concepts>
#include <iterator>

namespace nrws {
template<typename I>
concept is_channel_iterator = std::input_iterator<I>;

template<typename C>
concept is_channel_base = requires(C channel) {
    typename C::size_type;

    { channel.empty() } -> std::same_as<bool>;
    { channel.closed() } -> std::same_as<bool>;
    { channel.size() } -> std::same_as<typename C::size_type>;
};

template<typename S>
concept is_sender = requires(S sender) {
    typename S::value_type;
    typename S::error_type;
    typename S::result_type;

    // result types must be convertible to expected
    std::convertible_to<typename S::result_type, std::expected<void, typename S::result_type>>;
} && requires(S sender, const S::value_type &val) {
    { sender.send(val) } -> std::same_as<typename S::result_type>;
} && requires(S sender, S::value_type &&val) {
    { sender.send(val) } -> std::same_as<typename S::result_type>;
};

template<typename R>
concept is_receiver = requires(R receiver) {
    typename R::value_type;
    typename R::error_type;
    typename R::result_type;

    // result type must be convertible to expected
    std::convertible_to<typename R::result_type, std::expected<typename R::value_type, typename R::result_type>>;
} && requires(R receiver) {
    { receiver.receive() } -> std::same_as<typename R::result_type>;
};

}// namespace nrws
