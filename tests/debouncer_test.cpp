#include <gtest/gtest.h>

#include <cstddef>
#include <integra/debouncer.hpp>
#include <tuple>
#include <type_traits>

namespace
{

using integra::Debouncer;

TEST(DebouncerTest, SetsAfterThresholdAgreeingSamples)
{
    Debouncer<> debouncer{3U};

    EXPECT_FALSE(debouncer.Update(true));
    EXPECT_FALSE(debouncer.Update(true));
    EXPECT_TRUE(debouncer.Update(true));
    EXPECT_TRUE(debouncer.IsSet());
}

TEST(DebouncerTest, StaysSetWhileTheSamplesKeepAgreeing)
{
    Debouncer<> debouncer{2U};
    std::ignore = debouncer.Update(true);
    std::ignore = debouncer.Update(true);

    EXPECT_TRUE(debouncer.Update(true));
    // Saturates at the threshold rather than climbing past it.
    EXPECT_EQ(debouncer.Count(), 2U);
}

TEST(DebouncerTest, ClearsOnTheFirstDisagreeingSample)
{
    // Asymmetric on purpose: confirm slowly, let go at once.
    Debouncer<> debouncer{3U};
    for (int i = 0; i < 3; ++i)
    {
        std::ignore = debouncer.Update(true);
    }
    ASSERT_TRUE(debouncer.IsSet());

    EXPECT_FALSE(debouncer.Update(false));
    EXPECT_EQ(debouncer.Count(), 2U);
    // One agreeing sample puts it back, since the count only fell by one.
    EXPECT_TRUE(debouncer.Update(true));
}

TEST(DebouncerTest, DoesNotGoBelowZero)
{
    Debouncer<> debouncer{3U};
    std::ignore = debouncer.Update(false);
    std::ignore = debouncer.Update(false);

    EXPECT_EQ(debouncer.Count(), 0U);
    EXPECT_FALSE(debouncer.Update(true));
    EXPECT_EQ(debouncer.Count(), 1U);
}

TEST(DebouncerTest, AppliesItsStepSizes)
{
    // Climbs two at a time, falls three at a time.
    Debouncer<2U, 3U> debouncer{5U};

    EXPECT_FALSE(debouncer.Update(true));
    EXPECT_EQ(debouncer.Count(), 2U);
    EXPECT_FALSE(debouncer.Update(true));
    EXPECT_EQ(debouncer.Count(), 4U);
    // 6 saturates at 5.
    EXPECT_TRUE(debouncer.Update(true));
    EXPECT_EQ(debouncer.Count(), 5U);

    EXPECT_FALSE(debouncer.Update(false));
    EXPECT_EQ(debouncer.Count(), 2U);
    // Falls by at most what is left.
    EXPECT_FALSE(debouncer.Update(false));
    EXPECT_EQ(debouncer.Count(), 0U);
}

TEST(DebouncerTest, TakesAZeroThresholdAsOne)
{
    Debouncer<> debouncer{0U};
    EXPECT_EQ(debouncer.Threshold(), 1U);
    // Not set before it has seen a sample.
    EXPECT_FALSE(debouncer.IsSet());
    EXPECT_TRUE(debouncer.Update(true));
}

TEST(DebouncerTest, KeepsTheCountWithinALoweredThreshold)
{
    Debouncer<> debouncer{5U};
    for (int i = 0; i < 4; ++i)
    {
        std::ignore = debouncer.Update(true);
    }

    debouncer.SetThreshold(2U);
    EXPECT_EQ(debouncer.Count(), 2U);
    EXPECT_TRUE(debouncer.IsSet());

    debouncer.SetThreshold(0U);
    EXPECT_EQ(debouncer.Threshold(), 1U);
    EXPECT_EQ(debouncer.Count(), 1U);
}

TEST(DebouncerTest, KeepsTheCountAcrossARaisedThreshold)
{
    Debouncer<> debouncer{2U};
    std::ignore = debouncer.Update(true);
    std::ignore = debouncer.Update(true);

    debouncer.SetThreshold(4U);
    EXPECT_EQ(debouncer.Count(), 2U);
    EXPECT_FALSE(debouncer.IsSet());
}

TEST(DebouncerTest, ClearStartsOver)
{
    Debouncer<> debouncer{2U};
    std::ignore = debouncer.Update(true);
    std::ignore = debouncer.Update(true);

    debouncer.Clear();
    EXPECT_EQ(debouncer.Count(), 0U);
    EXPECT_FALSE(debouncer.IsSet());
}

TEST(DebouncerTest, TestsAsAConditionButIsNotANumber)
{
    Debouncer<> debouncer{1U};
    std::ignore = debouncer.Update(true);

    EXPECT_TRUE(static_cast<bool>(debouncer));
    if (!debouncer)
    {
        ADD_FAILURE() << "a set debouncer tested false";
    }
    static_assert(!std::is_convertible_v<Debouncer<>, int>);
    static_assert(!std::is_convertible_v<Debouncer<>, bool>);
}

TEST(DebouncerTest, RunsAtCompileTime)
{
    static constexpr bool SET = [] {
        Debouncer<> debouncer{2U};
        std::ignore = debouncer.Update(true);
        return debouncer.Update(true);
    }();
    static_assert(SET);
    SUCCEED();
}

} // namespace
