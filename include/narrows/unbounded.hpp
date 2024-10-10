#pragma once

#include <atomic>
#include <condition_variable>
#include <queue>

namespace nrws {

// This class is roughly based on Andrei Avram's excellent cpp channel
// https://blog.andreiavram.ro/cpp-channel-thread-safe-container-share-data-threads/
template<typename T>
class channel
{
  public:
    using value_type = std::decay<T>;
    using size_type = std::size_t;

    constexpr channel() = default;

    inline auto push(const value_type &value) -> void;
    inline auto push(value_type &&value) -> void;

    [[nodiscard]] inline auto pop() -> std::optional<value_type>;

    inline auto close() -> void;

    [[nodiscard]] inline auto size() const noexcept -> size_type;
    [[nodiscard]] inline auto empty() const noexcept -> bool;
    [[nodiscard]] inline auto closed() const noexcept -> bool;

  private:
    std::atomic<size_type> size_{ 0 };
    std::atomic<bool> closed_{ false };
    std::queue<value_type> queue_{};
    std::mutex mutex_;
    std::condition_variable cv_;
};
}// namespace nrws

/* Unbounded Channel Implementations */

template<typename T>
inline auto nrws::channel<T>::push(const value_type &value) -> void
{
    {
        std::unique_lock<std::mutex> lock{ mutex_ };

        queue_.push(value);
        size_ += 1;
    }

    cv_.notify_one();
}

template<typename T>
inline auto nrws::channel<T>::push(value_type &&value) -> void
{
    {
        std::unique_lock<std::mutex> lock{ mutex_ };

        queue_.push(std::forward<T>(value));
        size_ += 1;
    }

    cv_.notify_one();
}

template<typename T>
[[nodiscard]] inline auto nrws::channel<T>::pop() -> std::optional<value_type>
{
    std::optional<value_type> value{ std::nullopt };

    {
        std::unique_lock lock{ mutex_ };
        cv_.wait(lock, [this]() { return !empty() || closed(); });

        if (!empty()) {
            value = std::move(queue_.front());
            queue_.pop();
            size_ -= 1;
        }
    }

    cv_.notify_one();

    return value;
}

template<typename T>
inline auto nrws::channel<T>::close() -> void
{
    {
        std::unique_lock<std::mutex> lock{ mutex_ };
        closed_ = true;
    }

    cv_.notify_all();
}

template<typename T>
[[nodiscard]] inline auto nrws::channel<T>::size() const noexcept -> size_type
{
    return size_;
}

template<typename T>
[[nodiscard]] inline auto nrws::channel<T>::empty() const noexcept -> bool
{
    return size_ == 0;
}

template<typename T>
[[nodiscard]] inline auto nrws::channel<T>::closed() const noexcept -> bool
{
    return closed_;
}
