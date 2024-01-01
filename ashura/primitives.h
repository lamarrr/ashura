#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

#include "ashura/math.h"
#include "ashura/types.h"
#include "stx/limits.h"
#include "stx/option.h"

namespace ash
{

constexpr f32 epsilon_clamp(f32 x)
{
  return abs(x) > F32_EPSILON ? x : F32_EPSILON;
}

typedef Vec2U Extent;
typedef Vec2U Offset;

struct Rect
{
  Vec2 offset;
  Vec2 extent;

  constexpr bool contains(Vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y &&
           (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
  }

  constexpr bool overlaps(Rect other) const
  {
    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y;
  }

  constexpr Rect intersect(Rect other) const
  {
    if (!overlaps(other))
    {
      return Rect{offset, {0, 0}};
    }

    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    Vec2 int_begin{op::max(a_begin.x, b_begin.x),
                   op::max(a_begin.y, b_begin.y)};
    Vec2 int_end{op::min(a_end.x, b_end.x), op::min(a_end.y, b_end.y)};

    return Rect{int_begin, int_end - int_begin};
  }
};

/*

struct tri
{
  Vec2 p0, p1, p2;

  constexpr f32 sign() const
  {
    return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
  }

  constexpr bool contains(Vec2 point) const
  {
    f32 sign0 = tri{point, p0, p1}.sign();
    f32 sign1 = tri{point, p1, p2}.sign();
    f32 sign2 = tri{point, p2, p0}.sign();

    bool has_neg = (sign0 < 0) || (sign1 < 0) || (sign2 < 0);
    bool has_pos = (sign0 > 0) || (sign1 > 0) || (sign2 > 0);

    return !(has_neg && has_pos);
  }
};

// each coordinate is an edge of the quad
// TODO(lamarrr): rename this
struct Quad
{
  Vec2 p0, p1, p2, p3;

  constexpr bool contains(Vec2 point) const
  {
    return tri{.p0 = p0, .p1 = p1, .p2 = p2}.contains(point) ||
           tri{.p0 = p0, .p1 = p2, .p2 = p3}.contains(point);
  }
};

struct Rect
{
  Vec2 offset, extent;

  constexpr bool overlaps(Quad const &quad) const
  {
    return contains(quad.p0) || contains(quad.p1) || contains(quad.p2) ||
           contains(quad.p3);
  }

  constexpr Quad to_quad() const
  {
    return Quad{.p0 = top_left(),
                .p1 = top_right(),
                .p2 = bottom_right(),
                .p3 = bottom_left()};
  }

};


struct Box
{
  Vec3 offset, extent;

  constexpr f32 volume() const
  {
    return extent.x * extent.y * extent.z;
  }

  constexpr bool contains(Vec3 point) const
  {
    return offset.x <= point.x && offset.y <= point.y && offset.z <= point.z &&
           (offset.x + extent.x) >= point.x &&
           (offset.y + extent.y) >= point.y && (offset.z + extent.z) >= point.z;
    return true;
  }

  constexpr auto bounds() const
  {
    return std::make_tuple(offset.x, offset.x + extent.x, offset.y,
                           offset.y + extent.y, offset.z, offset.z + extent.z);
  }

  constexpr bool overlaps(Box other) const
  {
    auto [x0_min, x0_max, y0_min, y0_max, z0_min, z0_max] = bounds();
    auto [x1_min, x1_max, y1_min, y1_max, z1_min, z1_max] = other.bounds();

    return x0_min < x1_max && x0_max > x1_min && y1_max > y0_min &&
           y1_min < y0_max && z1_max > z0_min && z1_min < z0_max;
  }
};


constexpr Quad transform2d(Mat3 const &a, Rect const &b)
{
  return Quad{.p0 = transform2d(a, b.top_left()),
              .p1 = transform2d(a, b.top_right()),
              .p2 = transform2d(a, b.bottom_right()),
              .p3 = transform2d(a, b.bottom_left())};
}


*/
}        // namespace ash
