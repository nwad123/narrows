#pragma once

#include "concepts.hpp"
#include "narrows/bounded.hpp"

#include <expected>
#include <memory>
#include <type_traits>

namespace nrws {

enum class sender_error_t : uint8_t { ChannelClosed, ChannelFull };
enum class receiver_error_t : uint8_t { ChannelClosed, ChannelFull };

template<typename T, template<typename V = T> typename Backend>
class Sender
{
  public:
    using value_type = std::decay_t<T>;
    using error_type = sender_error_t;
    using result_type = std::expected<void, error_type>;
    using channel_type = std::decay_t<Backend<T>>;

    explicit Sender(std::shared_ptr<channel_type> backend) : backend_(backend) {}

    [[nodiscard]] inline auto send(const value_type &val) -> result_type;
    [[nodiscard]] inline auto send(value_type &&val) -> result_type;

    inline auto close() { backend_->close(); }

  private:
    std::shared_ptr<channel_type> backend_;
};
static_assert(is_sender<Sender<int, bounded_channel>>, "Must satisfy the sender concept.");

template<typename T, template<typename V = T> typename Backend>
class Receiver
{
  public:
    using value_type = std::decay_t<T>;
    using error_type = receiver_error_t;
    using result_type = std::expected<value_type, error_type>;
    using channel_type = std::decay_t<Backend<T>>;

    explicit Receiver(std::shared_ptr<channel_type> backend) : backend_(backend) {}

    [[nodiscard]] inline auto receive() -> result_type;

    [[nodiscard]] inline auto begin();
    [[nodiscard]] inline auto end();

    inline auto close() { backend_->close(); }

  private:
    std::shared_ptr<channel_type> backend_;
};
static_assert(is_receiver<Receiver<int, bounded_channel>>, "Must satisfy the receiver concept");

template<typename T>
[[nodiscard]] auto bounded(const std::size_t capacity)
    -> std::pair<Sender<T, bounded_channel>, Receiver<T, bounded_channel>>
{
    auto bounded_ch = std::make_shared<bounded_channel<T>>(capacity);
    return { Sender<T, bounded_channel>(bounded_ch), Receiver<T, bounded_channel>(bounded_ch) };
}

template<typename T, template<typename V = T> typename Backend>
[[nodiscard]] inline auto Sender<T, Backend>::send(const value_type &val) -> result_type
{
    backend_->push(val);
    return result_type{};
}

template<typename T, template<typename V = T> typename Backend>
[[nodiscard]] inline auto Sender<T, Backend>::send(value_type &&val) -> result_type
{
    backend_->push(std::forward<T>(val));
    return result_type{};
}

template<typename T, template<typename V = T> typename Backend>
[[nodiscard]] inline auto Receiver<T, Backend>::receive() -> result_type
{
    const auto received = backend_->pop();
    if (received.has_value()) { return received.value(); }
    return std::unexpected(receiver_error_t::ChannelClosed);
}

template<typename T, template<typename V = T> typename Backend>
[[nodiscard]] inline auto Receiver<T, Backend>::begin()
{
    return backend_->begin();
}

template<typename T, template<typename V = T> typename Backend>
[[nodiscard]] inline auto Receiver<T, Backend>::end()
{
    return backend_->end();
}

};// namespace nrws
