#pragma once
#include "ashura/std/range.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct Box Box;

struct Box
{
  Vec3 offset;
  Vec3 extent;

  constexpr Vec3 center() const
  {
    return offset + (extent / 2);
  }

  constexpr Vec3 end() const
  {
    return offset + extent;
  }

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

  constexpr bool overlaps(Box const &other) const
  {
    Vec3 a_begin = offset;
    Vec3 a_end   = offset + extent;
    Vec3 b_begin = other.offset;
    Vec3 b_end   = other.offset + other.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y &&
           a_begin.z <= b_end.z && a_end.z >= b_begin.z;
  }
};

}        // namespace ash
