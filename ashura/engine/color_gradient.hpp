/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

struct ColorGradient
{
    f32x4 top_    = {};
    f32x4 bottom_ = {};
    f32   angle_  = 0;
    f32   center_ = 0;

    constexpr ColorGradient() = default;

    constexpr ColorGradient(f32x4 c) : top_{c}, bottom_{c}, angle_{0}, center_{0}
    {
    }

    constexpr ColorGradient(u8x4 c) : ColorGradient{norm(c)}
    {
    }

    constexpr ColorGradient(f32x4 top, f32x4 bottom, f32 angle, f32 center) :
      top_{top},
      bottom_{bottom},
      angle_{angle},
      center_{center}
    {
    }

    constexpr ColorGradient(u8x4 top, u8x4 bottom, f32 angle, f32 center) :
      ColorGradient{norm(top), norm(bottom), angle, center}
    {
    }

    constexpr ColorGradient & to_horizontal()
    {
        angle_ = 0;
        return *this;
    }

    constexpr ColorGradient & to_vertical()
    {
        angle_ = 0.5F * PI;
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

    constexpr f32 angle() const
    {
        return angle_;
    }

    constexpr f32 center() const
    {
        return center_;
    }
};

}    // namespace ash
