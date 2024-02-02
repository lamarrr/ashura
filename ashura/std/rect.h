#pragma once
#include "ashura/std/algorithms.h"
#include "ashura/std/op.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct Rect Rect;

struct Rect
{
  Vec2 offset;
  Vec2 extent;

  constexpr Vec2 center() const
  {
    return offset + (extent / 2);
  }

  constexpr Vec2 end() const
  {
    return offset + extent;
  }

  constexpr f32 area() const
  {
    return extent.x * extent.y;
  }

  constexpr bool contains(Vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y &&
           (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
  }

  constexpr bool overlaps(Rect const &other) const
  {
    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y;
  }

  constexpr Rect intersect(Rect const &other) const
  {
    if (!overlaps(other))
    {
      return Rect{offset, {0, 0}};
    }

    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    Vec2 int_begin{max(a_begin.x, b_begin.x), max(a_begin.y, b_begin.y)};
    Vec2 int_end{min(a_end.x, b_end.x), min(a_end.y, b_end.y)};

    return Rect{int_begin, int_end - int_begin};
  }
};

}        // namespace ash