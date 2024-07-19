/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"

namespace ash
{

struct ColorGradient
{
  Vec4 colors[4] = {};

  static constexpr ColorGradient uniform(Vec4 c)
  {
    return ColorGradient{{c, c, c, c}};
  }

  static constexpr ColorGradient uniform(Vec4U8 c)
  {
    return uniform(c.norm());
  }

  static constexpr ColorGradient x(Vec4 x0, Vec4 x1)
  {
    return ColorGradient{{x0, x1, x0, x1}};
  }

  static constexpr ColorGradient x(Vec4U8 x0, Vec4U8 x1)
  {
    return x(x0.norm(), x1.norm());
  }

  static constexpr ColorGradient y(Vec4 y0, Vec4 y1)
  {
    return ColorGradient{{y0, y0, y1, y1}};
  }

  static constexpr ColorGradient y(Vec4U8 y0, Vec4U8 y1)
  {
    return y(y0.norm(), y1.norm());
  }

  constexpr Vec4 const &operator[](usize i) const
  {
    return colors[i];
  }

  constexpr Vec4 &operator[](usize i)
  {
    return colors[i];
  }
};

}        // namespace ash