#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iterator>
#include <mutex>

namespace nrws {

// generic queue that can be built off of
// T = value, S = storage
template<typename T, template<typename> typename S, auto Push_Ref_, auto Push_Move_, auto Pop_>
    requires std::is_convertible_v<decltype(Push_Ref_), std::function<void(const std::decay_t<T> &)>>
             && std::is_convertible_v<decltype(Push_Move_), std::function<void(std::decay_t<T> &&)>>
             && std::is_convertible_v<decltype(Pop_), std::function<std::optional<std::decay_t<T>>(void)>>
class __channel
{
  public:
    using value_type = std::decay_t<T>;
    using container_type = S<T>;
    using size_type = container_type::size_type;

    // A non-specialized __channel class cannot be constructed
    __channel() = delete;
    __channel(const __channel &) = delete;
    __channel &operator=(const __channel &) = delete;

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

        _iter(__channel &);

        value_type operator*();
        _iter &operator++();
        void operator++(int);

        bool operator==(const _iter& other);
        bool operator!=(const _iter &other);
    };
    static_assert(std::input_iterator<_iter>, "The channel base iterator must satisfy input iterator");

    // Iterator function
    [[nodiscard]] inline auto begin() -> _iter;
    [[nodiscard]] inline auto end() -> _iter;

  private:
    std::atomic<bool> closed_{ false };
    container_type container_{};
    std::mutex mutex_;
    std::condition_variable cv_;
};

};// namespace nrws
