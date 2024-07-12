/// SPDX-License-Identifier: MIT
#pragma once

namespace ash
{

template <typename T>
struct Pinned
{
  typedef T Type;
  template <typename... Args>
  constexpr Pinned(Args &&...args) : value{((Args &&) args)...}
  {
  }
  T value;

  constexpr Pinned(Pinned const &)            = delete;
  constexpr Pinned(Pinned &&)                 = delete;
  constexpr Pinned &operator=(Pinned const &) = delete;
  constexpr Pinned &operator=(Pinned &&)      = delete;
};

}        // namespace ash