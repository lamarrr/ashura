/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.hpp"

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

}    // namespace ash
