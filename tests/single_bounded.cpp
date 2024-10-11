#include "narrows/single_bounded.hpp"
#include <gtest/gtest.h>

#include <thread>

TEST(SingleBounded, Construction)
{
    using namespace nrws;

    auto [s, r] = bounded<int>(15U);
}

TEST(SingleBounded, SendOneInt)
{
    using namespace nrws;

    auto [s, r] = bounded<int>(15U);

    const auto status = s.send(1);
    EXPECT_TRUE(status.has_value());

    const auto value = r.receive();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), 1);
}

TEST(SingleBounded, SendMultipleInt)
{
    using namespace nrws;

    constexpr int max = 100;

    auto [s, r] = bounded<int>(15U);

    for (int i = 0; i < max; i++) {
        const auto status = s.send(i);
        EXPECT_TRUE(status.has_value());
    }

    s.close();

    int expected = 0;
    for (const auto actual : r) {
        EXPECT_EQ(actual, expected);
        expected++;
    }
}
