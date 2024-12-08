/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/enum.gen.h"

namespace ash
{

template <typename... T>
struct IsTriviallyRelocatable<Enum<T...>>
{
  static constexpr bool value = (TriviallyRelocatable<T> && ... && true);
};

}        // namespace ash
