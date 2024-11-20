#include "narrows/bounded.hpp"
#include <gtest/gtest.h>

#include <thread>

TEST(Bounded, IntConstruction)
{
    using namespace nrws;

    bounded_channel<int> ch(15);
    EXPECT_EQ(ch.size(), 0);
    EXPECT_EQ(ch.capacity(), 15);
    EXPECT_FALSE(ch.closed());
}

TEST(Bounded, IntPush)
{
    using namespace nrws;

    bounded_channel<int> ch(15);

    ch.push(1);
    EXPECT_EQ(ch.size(), 1);

    ch.push(2);
    EXPECT_EQ(ch.size(), 2);
}

TEST(Bounded, IntPop)
{
    using namespace nrws;

    bounded_channel<int> ch(15);
    ch.push(1);
    ch.push(2);
    EXPECT_EQ(ch.size(), 2);

    const auto _1 = ch.pop();
    EXPECT_TRUE(_1.has_value());
    EXPECT_EQ(_1.value(), 1);

    const auto _2 = ch.pop();
    EXPECT_TRUE(_2.has_value());
    EXPECT_EQ(_2.value(), 2);
}

TEST(Bounded, IntClose)
{
    using namespace nrws;

    bounded_channel<int> ch(15);
    ch.push(1);
    ch.push(2);

    const auto _1 = ch.pop();
    EXPECT_TRUE(_1.has_value());
    EXPECT_EQ(_1.value(), 1);

    EXPECT_FALSE(ch.closed());
    ch.close();
    EXPECT_TRUE(ch.closed());

    const auto _2 = ch.pop();
    EXPECT_TRUE(_2.has_value());
    EXPECT_EQ(_2.value(), 2);

    const auto _empty = ch.pop();
    EXPECT_FALSE(_empty.has_value());
};

TEST(Bounded, IntIter)
{
    using namespace nrws;

    bounded_channel<int> ch(15);
    ch.push(1);
    ch.push(2);
    ch.close();

    int expected = 1;
    for (const auto actual : ch) {
        EXPECT_EQ(actual, expected);
        expected++;
    }
}

TEST(Bounded, IntIterTwoThreads)
{
    using namespace nrws;

    constexpr static int max = 10000;

    bounded_channel<int> ch(15);

    auto producer = [&ch]() {
        for (int i = 0; i < max; i++) { ch.push(i); }
        ch.close();
    };

    auto consumer = [&ch]() {
        int expected = 0;
        for (const auto actual : ch) {
            EXPECT_EQ(actual, expected);
            expected++;
        }
    };

    std::thread p(producer);
    std::thread c(consumer);

    p.join();
    c.join();
}


TEST(Bounded, IntIterMulipleWriteThreads)
{
    using namespace nrws;

    constexpr static int max = 10000;

    bounded_channel<int> ch(15);

    auto producer_1 = [&ch]() {
        for (int i = 0; i < max; i++) { ch.push(i); }
    };

    auto producer_2 = [&ch]() {
        for (int i = 0; i < max; i++) { ch.push(-i); }
    };

    auto consumer = [&ch]() {
        int sum = 0;
        for (const auto actual : ch) { sum += actual; }
        EXPECT_EQ(sum, 0);
    };

    std::thread p_1(producer_1);
    std::thread p_2(producer_2);
    std::thread c(consumer);

    p_1.join();
    p_2.join();
    ch.close();

    c.join();
}

TEST(Bounded, IntIterMulipleReadThreads)
{
    using namespace nrws;

    constexpr static int max = 10;

    bounded_channel<int> ch(15);

    int sum_p{ 0 };
    int sum_c_1{ 0 };
    int sum_c_2{ 0 };

    auto producer = [&ch, &sum_p]() {
        for (int i = 0; i < max; i++) {
            ch.push(i);
            sum_p += i;
        }
        ch.close();
    };

    auto consumer_1 = [&ch, &sum_c_1]() {
        for (const auto val : ch) { sum_c_1 += val; }
    };

    auto consumer_2 = [&ch, &sum_c_2]() {
        for (const auto val : ch) { sum_c_2 += val; }
    };

    std::thread p(producer);
    std::thread c_1(consumer_1);
    std::thread c_2(consumer_2);

    p.join();

    c_1.join();
    c_2.join();

    EXPECT_EQ((sum_c_1 + sum_c_2), sum_p);
}
