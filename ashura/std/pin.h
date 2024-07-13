/// SPDX-License-Identifier: MIT
#pragma once

namespace ash
{

template <typename T>
struct Pin
{
  typedef T Type;
  template <typename... Args>
  constexpr Pin(Args &&...args) : value{((Args &&) args)...}
  {
  }
  T value;

  constexpr Pin(Pin const &)            = delete;
  constexpr Pin(Pin &&)                 = delete;
  constexpr Pin &operator=(Pin const &) = delete;
  constexpr Pin &operator=(Pin &&)      = delete;
};

}        // namespace ash