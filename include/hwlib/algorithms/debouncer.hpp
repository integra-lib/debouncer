#pragma once

#include <algorithm>
#include <cstddef>

namespace hwlib::algorithms
{

/// A sample-counting debouncer: a saturating counter that climbs by INC on every
/// true sample and falls by DEC on every false one, and reads as set while it sits
/// at the threshold.
///
/// It is not symmetric, and that is the point of it rather than an oversight: it
/// takes `threshold / INC` agreeing samples to set, and with DEC equal to INC a
/// single disagreeing sample to clear. It confirms a condition before acting on it
/// — a level held for long enough, a reading over a limit several times running —
/// and lets go the moment the condition fails. A debouncer that should also resist
/// clearing wants hysteresis, which this is not.
///
/// It counts samples, not time. For a button on a pin, where the settle time is
/// what matters, button-event is the component to reach for.
template<std::size_t INC = 1, std::size_t DEC = 1>
    requires(INC != 0U && DEC != 0U)
class Debouncer
{
public:
    /// A threshold of zero is taken as one: a debouncer that is set before it has
    /// seen a single sample would not be debouncing anything.
    constexpr explicit Debouncer(std::size_t threshold = 1U) noexcept
        : m_threshold{ClampThreshold(threshold)}
    {}

    /// Feeds one sample and returns whether the debouncer is set afterwards.
    /// Deliberately not [[nodiscard]]: feeding the sample is the point of the call,
    /// and a caller that reads the state later through IsSet() has no use for it.
    constexpr bool Update(bool sample) noexcept
    {
        if (sample)
        {
            m_count = std::min(m_count + INC, m_threshold);
        }
        else
        {
            m_count -= std::min(DEC, m_count);
        }
        return IsSet();
    }

    [[nodiscard]] constexpr bool IsSet() const noexcept
    {
        return m_count == m_threshold;
    }

    /// Explicit, so a debouncer tests as a condition but does not quietly become a
    /// number: the original's implicit conversion let `int n = debouncer;` compile.
    constexpr explicit operator bool() const noexcept
    {
        return IsSet();
    }

    constexpr void Clear() noexcept
    {
        m_count = 0U;
    }

    /// Changes the threshold without losing the samples already counted, beyond what
    /// a lower threshold can hold.
    constexpr void SetThreshold(std::size_t threshold) noexcept
    {
        m_threshold = ClampThreshold(threshold);
        m_count     = std::min(m_count, m_threshold);
    }

    [[nodiscard]] constexpr std::size_t Count() const noexcept
    {
        return m_count;
    }

    [[nodiscard]] constexpr std::size_t Threshold() const noexcept
    {
        return m_threshold;
    }

private:
    [[nodiscard]] static constexpr std::size_t ClampThreshold(std::size_t threshold) noexcept
    {
        return threshold == 0U ? 1U : threshold;
    }

    std::size_t m_count{0U};
    std::size_t m_threshold;
};

} // namespace hwlib::algorithms
