/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"

namespace ash
{

struct DoubleGrowth
{
  static constexpr usize grow(usize capacity)
  {
    return capacity << 1;
  }
};

struct HalfGrowth
{
  static constexpr usize grow(usize capacity)
  {
    return capacity + (capacity >> 1);
  }
};

using Growth = HalfGrowth;

}    // namespace ash
