#pragma once

#include <condition_variable>
#include <iterator>
#include <mutex>
#include <optional>

namespace nrws {

// generic queue that can be built off of
// T = value, S = storage
template<typename T, template<typename, typename...> typename S, typename... Args>
class _multi_channel
{
  public:
    using value_type = std::decay_t<T>;
    using container_type = S<value_type, Args...>;
    using size_type = container_type::size_type;

    // A non-specialized __channel class cannot be constructed
    _multi_channel() = delete;
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
    [[nodiscard]] inline auto closed() const noexcept -> bool;

    // Iterator type
    class _iter
    {
        using value_type = value_type;

        _iter(_multi_channel &mc) : mc_(mc) {}

        value_type operator*();
        _iter &operator++();
        void operator++(int);

        bool operator==(const _iter &other) const noexcept;
        bool operator!=(const _iter &other) const noexcept;

      private:
        _multi_channel &mc_;
    };
    static_assert(std::input_iterator<_iter>, "The channel base iterator must satisfy input iterator");

    // Iterator function
    [[nodiscard]] inline auto begin() -> _iter;
    [[nodiscard]] inline auto end() -> _iter;

  private:
    std::mutex mutex_;
    std::condition_variable cv_;
};

template<typename T, template<typename, typename...> typename S, typename... Args>
_multi_channel<T, S, Args...>::_iter::value_type _multi_channel<T, S, Args...>::_iter::operator*()
{
    return *mc_.pop();
}

template<typename T, template<typename, typename...> typename S, typename... Args>
_multi_channel<T, S, Args...>::_iter &_multi_channel<T, S, Args...>::_iter::operator++()
{
    return *this;
}

template<typename T, template<typename, typename...> typename S, typename... Args>
void _multi_channel<T, S, Args...>::_iter::operator++(int)
{
    return;
}

template<typename T, template<typename, typename...> typename S, typename... Args>
bool _multi_channel<T, S, Args...>::_iter::operator==(const _iter &other) const noexcept
{
    std::unique_lock lock{ mc_.mutex_ };
    mc_.cv_.wait(lock, [this]() { return !mc_.empty() || mc_.closed(); });

    return mc_.empty() || mc_.closed();
}

template<typename T, template<typename, typename...> typename S, typename... Args>
bool _multi_channel<T, S, Args...>::_iter::operator!=(const _iter &other) const noexcept
{
    return not(this->operator==(other));
}
};// namespace nrws
