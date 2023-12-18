#pragma once
#include <chrono>

namespace ash
{
// monotonic system clock
using Clock               = std::chrono::steady_clock;
using HighResolutionClock = std::chrono::high_resolution_clock;
using Timepoint           = Clock::time_point;
using Nanoseconds         = std::chrono::nanoseconds;
using Milliseconds        = std::chrono::milliseconds;
using Seconds             = std::chrono::seconds;

}        // namespace ash