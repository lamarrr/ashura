#include "ashura/math.h"
#include "ashura/types.h"

namespace ash
{

/// Simple Layout Constraint Model
/// @bias: adding or subtracting from the source size, i.e. value should be
/// source size - 20px
/// @scale: scales the source size, i.e. value should be 0.5 of source size
/// @min: clamps the source size, i.e. value should be at least 20px
/// @max: clamps the source size, i.e. value should be at most 100px
/// @minr: clamps the source size relatively. i.e. value should be at least 0.5
/// of source size
/// @maxr: clamps the source size relatively. i.e. value should be at most 0.5
/// of source size
struct Constraint
{
  f32 bias  = 0;
  f32 scale = 0;
  f32 min   = F32_MIN;
  f32 max   = F32_MAX;
  f32 minr  = 0;
  f32 maxr  = 1;

  static constexpr Constraint relative(f32 scale)
  {
    return Constraint{
        .bias = 0, .scale = scale, .min = F32_MIN, .max = F32_MAX};
  }

  static constexpr Constraint absolute(f32 value)
  {
    return Constraint{
        .bias = value, .scale = 0, .min = F32_MIN, .max = F32_MAX};
  }

  constexpr Constraint with_min(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = v,
                      .max   = max,
                      .minr  = minr,
                      .maxr  = maxr};
  }

  constexpr Constraint with_max(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = min,
                      .max   = v,
                      .minr  = minr,
                      .maxr  = maxr};
  }

  constexpr Constraint with_minr(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = min,
                      .max   = max,
                      .minr  = v,
                      .maxr  = maxr};
  }

  constexpr Constraint with_maxr(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = min,
                      .max   = max,
                      .minr  = minr,
                      .maxr  = v};
  }

  constexpr f32 resolve(f32 value) const
  {
    return op::clamp(op::clamp(bias + value * scale, min, max), minr * value,
                     maxr * value);
  }
};

struct Constraint2D
{
  Constraint x, y;

  static constexpr Constraint2D relative(f32 x, f32 y)
  {
    return Constraint2D{.x = Constraint::relative(x),
                        .y = Constraint::relative(y)};
  }

  static constexpr Constraint2D relative(Vec2 xy)
  {
    return relative(xy.x, xy.y);
  }

  static constexpr Constraint2D absolute(f32 x, f32 y)
  {
    return Constraint2D{.x = Constraint::absolute(x),
                        .y = Constraint::absolute(y)};
  }

  static constexpr Constraint2D absolute(Vec2 xy)
  {
    return absolute(xy.x, xy.y);
  }

  constexpr Constraint2D with_min(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_min(nx), .y = y.with_min(ny)};
  }

  constexpr Constraint2D with_max(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_max(nx), .y = y.with_max(ny)};
  }

  constexpr Constraint2D with_minr(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_minr(nx), .y = y.with_minr(ny)};
  }

  constexpr Constraint2D with_maxr(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_maxr(nx), .y = y.with_maxr(ny)};
  }

  constexpr Vec2 resolve(f32 xsrc, f32 ysrc) const
  {
    return Vec2{x.resolve(xsrc), y.resolve(ysrc)};
  }

  constexpr Vec2 resolve(Vec2 src) const
  {
    return resolve(src.x, src.y);
  }
};

}        // namespace ash
