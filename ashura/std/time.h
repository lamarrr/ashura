/// SPDX-License-Identifier: MIT
#pragma once
#include <chrono>

namespace ash
{

using std::chrono::days;
using std::chrono::hours;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::months;
using std::chrono::nanoseconds;
using std::chrono::seconds;
using std::chrono::weeks;
using std::chrono::years;

using std::chrono::high_resolution_clock;
using std::chrono::steady_clock;

using clock = steady_clock;

using time_point = std::chrono::steady_clock::time_point;

using namespace std::chrono_literals;

using std::chrono::duration_cast;

}    // namespace ash
