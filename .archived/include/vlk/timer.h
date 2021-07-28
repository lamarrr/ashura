#pragma once

#include <chrono>

// TODO(lamarrr): move to general?

template <typename ClockType = std::chrono::steady_clock>
struct TickTimer {
  using time_point = typename ClockType::time_point;
  using clock_type = ClockType;

  TickTimer() noexcept {}

  auto start() noexcept { last_time_point_ = clock_type::now(); }

  std::chrono::nanoseconds tick() noexcept {
    auto current_time_point = clock_type::now();
    auto tick_duration = current_time_point - last_time_point_;
    last_time_point_ = current_time_point;
    return tick_duration;
  }

 private:
  time_point last_time_point_;
};
