#pragma once

#include <chrono>
#include <expected>
#include <type_traits>

#include "narrows/_internal/_errors.hpp"

namespace nrws::single {

template<typename T, template<typename> typename S>
class sender
{
  public:
    using value_type = std::decay_t<T>;
    using container_type = S<value_type>;
    using error_type = std::expected<void, error_id>;
    template<typename Rep, typename Period>
    using duration = std::chrono::duration<Rep, Period>;

    sender() = delete;
    sender(const sender &) = delete;
    sender &operator=(const sender &) = delete;

    [[nodiscard]] auto send(const value_type &value) -> error_type;
    [[nodiscard]] auto send(value_type &&value) -> error_type;

    template<typename Rep, typename Period>
    [[nodiscard]] auto send_timeout(const value_type &value, const duration<Rep, Period> &timeout) -> error_type;
    template<typename Rep, typename Period>
    [[nodiscard]] auto send_timeout(value_type &&value, const duration<Rep, Period> &timeout) -> error_type;

private:
    
};

template<typename T, template<typename> typename S>
class receiver
{
  public:
    using value_type = std::decay_t<T>;
    using container_type = S<value_type>;

    receiver() = delete;
    receiver(const receiver &) = delete;
    receiver &operator=(const receiver &) = delete;
};

}// namespace nrws::single
