#pragma once

#include "_internal/_multi_channel.hpp"

#include <atomic>
#include <condition_variable>
#include <optional>
#include <vector>

namespace nrws {

template<typename T>
using bounded_channel = _multi_channel<T, std::vector>;

template<typename T>
class _multi_channel<T, std::vector>
{
  public:
    using value_type = std::decay_t<T>;
    using container_type = std::vector<value_type>;
    using size_type = container_type::size_type;

    constexpr explicit _multi_channel(const std::size_t capacity) : size_(0U), vec_(capacity), head_(0U), tail_(0U) {}
    constexpr _multi_channel(_multi_channel &&other) = default;

    _multi_channel(const _multi_channel &) = delete;
    _multi_channel &operator=(const _multi_channel &) = delete;

    // Push/Pop API
    inline auto push(const value_type &value) -> void;
    inline auto push(value_type &&value) -> void;

    [[nodiscard]] inline auto pop() -> std::optional<value_type>;

    // Closing a channel
    inline auto close() -> void;

    // Channel status
    [[nodiscard]] inline auto size() const noexcept -> size_type;
    [[nodiscard]] inline auto empty() const noexcept -> bool;
    [[nodiscard]] inline auto full() const noexcept -> bool;
    [[nodiscard]] inline auto closed() const noexcept -> bool;

    // Iterator type
    class _iter
    {
      public:
        using value_type = value_type;

        explicit _iter(_multi_channel &mc) : mc_(mc) {}

        value_type operator*();
        _iter &operator++();
        void operator++(int);

        bool operator==(const _iter &other) const noexcept;
        bool operator!=(const _iter &other) const noexcept;

      private:
        _multi_channel &mc_;
    };

    // range functions
    [[nodiscard]] inline auto begin() -> _iter;
    [[nodiscard]] inline auto end() -> _iter;

  private:
    std::atomic<bool> closed_{ false };
    std::atomic<size_type> size_;
    container_type vec_;

    size_type head_;
    size_type tail_;

    std::mutex mutex_;
    std::condition_variable cv_;
};

template<typename T>
inline auto _multi_channel<T, std::vector>::push(const value_type &value) -> void
{
    {
        // obtain a lock, then wait for the channel to not be full
        std::unique_lock lock{ mutex_ };
        cv_.wait(lock, [this]() { return !full(); });

        // place the value in the vector
        vec_[head_] = value;
        size_++;

        // update the head pointer by either incrementing to the next point
        // or by reseting to the front of the vector
        if (head_ == vec_.size() - 1) {
            head_ = 0U;
        } else {
            head_++;
        }
    }

    cv_.notify_one();
}

template<typename T>
inline auto _multi_channel<T, std::vector>::push(value_type &&value) -> void
{
    {
        // obtain a lock, then wait for the channel to not be full
        std::unique_lock lock{ mutex_ };
        cv_.wait(lock, [this]() { return !full(); });

        // place the value in the vector
        vec_[head_] = std::forward<value_type>(value);
        size_++;

        // update the head pointer by either incrementing to the next point
        // or by reseting to the front of the vector
        if (head_ == vec_.size() - 1) {
            head_ = 0U;
        } else {
            head_++;
        }
    }

    cv_.notify_one();
}

template<typename T>
[[nodiscard]] inline auto _multi_channel<T, std::vector>::pop() -> std::optional<value_type>
{
    std::optional<value_type> value{ std::nullopt };

    {
        // obtain a lock and wait for the channel to not be empty or for the
        // channel to close
        std::unique_lock lock{ mutex_ };
        cv_.wait(lock, [this]() { return !empty() || closed(); });

        if (!empty()) { value = vec_[tail_]; }

        // update the tail index
        if (tail_ == vec_.size() - 1) {
            tail_ = 0U;
        } else {
            tail_++;
        }

        // update the size
        size_--;
    }

    cv_.notify_one();

    // return the value from the vector
    return value;
}

template<typename T>
inline auto _multi_channel<T, std::vector>::close() -> void
{
    closed_ = true;
    cv_.notify_all();
}

template<typename T>
[[nodiscard]] inline auto _multi_channel<T, std::vector>::size() const noexcept -> size_type
{
    return size_;
}

template<typename T>
[[nodiscard]] inline auto _multi_channel<T, std::vector>::empty() const noexcept -> bool
{
    return size_ == 0;
}

template<typename T>
[[nodiscard]] inline auto _multi_channel<T, std::vector>::full() const noexcept -> bool
{
    return size_ == vec_.size();
}

template<typename T>
[[nodiscard]] inline auto _multi_channel<T, std::vector>::closed() const noexcept -> bool
{
    return closed_;
}

template<typename T>
_multi_channel<T, std::vector>::_iter::value_type _multi_channel<T, std::vector>::_iter::operator*()
{
    return *mc_.pop();
}

template<typename T>
_multi_channel<T, std::vector>::_iter &_multi_channel<T, std::vector>::_iter::operator++()
{
    return *this;
}

template<typename T>
void _multi_channel<T, std::vector>::_iter::operator++(int)
{}

template<typename T>
bool _multi_channel<T, std::vector>::_iter::operator==(const _iter &other) const noexcept
{
    std::unique_lock lock{ mc_.mutex_ };
    mc_.cv_.wait(lock, [this]() { return !mc_.empty() || mc_.closed(); });

    return mc_.empty() || mc_.closed();
}

template<typename T>
bool _multi_channel<T, std::vector>::_iter::operator!=(const _iter &other) const noexcept
{
    return not(this->operator==(other));
}

template<typename T>
[[nodiscard]] inline auto _multi_channel<T, std::vector>::begin() -> _iter
{
    return _iter(*this);
}

template<typename T>
[[nodiscard]] inline auto _multi_channel<T, std::vector>::end() -> _iter
{
    return _iter(*this);
}

}// namespace nrws
