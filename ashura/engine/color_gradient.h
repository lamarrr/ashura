/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

struct ColorGradient
{
    f32x4 top_    = {};
    f32x4 bottom_ = {};
    f32x2 rotor_  = {1, 0};
    f32   center_ = 0;

    constexpr ColorGradient() = default;

    constexpr ColorGradient(f32x4 c) :
      top_{c},
      bottom_{c},
      rotor_{1, 0},
      center_{0}
    {
    }

    constexpr ColorGradient(u8x4 c) : ColorGradient{norm(c)}
    {
    }

    constexpr ColorGradient(f32x4 top, f32x4 bottom, f32 angle, f32 center) :
      top_{top},
      bottom_{bottom},
      rotor_{ash::rotor(angle)},
      center_{center}
    {
    }

    constexpr ColorGradient(u8x4 top, u8x4 bottom, f32 angle, f32 center) :
      ColorGradient{norm(top), norm(bottom), angle, center}
    {
    }

    constexpr ColorGradient & to_horizontal()
    {
        rotor_ = {0, 1};
        return *this;
    }

    constexpr ColorGradient & to_vertical()
    {
        rotor_ = {1, 0};
        return *this;
    }

    constexpr bool is_transparent() const
    {
        return top_.a() == 0 && bottom_.a() == 0;
    }

    constexpr f32x4 top() const
    {
        return top_;
    }

    constexpr f32x4 bottom() const
    {
        return bottom_;
    }

    constexpr f32x2 rotor() const
    {
        return rotor_;
    }

    constexpr f32 center() const
    {
        return center_;
    }
};

}    // namespace ash
