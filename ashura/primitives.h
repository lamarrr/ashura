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

constexpr Vec2 epsilon_clamp(Vec2 a)
{
  return Vec2{.x = epsilon_clamp(a.x), .y = epsilon_clamp(a.y)};
}

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

  constexpr auto bounds() const
  {
    return std::make_tuple(offset.x, offset.x + extent.x, offset.y,
                           offset.y + extent.y);
  }

  constexpr Vec2 min() const
  {
    return offset;
  }

  constexpr Vec2 max() const
  {
    return offset + extent;
  }

  constexpr bool overlaps(Rect other) const
  {
    auto [x0_min, x0_max, y0_min, y0_max] = bounds();
    auto [x1_min, x1_max, y1_min, y1_max] = other.bounds();

    return x0_min < x1_max && x0_max > x1_min && y1_max > y0_min &&
           y1_min < y0_max;
  }

  constexpr bool overlaps(Quad const &quad) const
  {
    return contains(quad.p0) || contains(quad.p1) || contains(quad.p2) ||
           contains(quad.p3);
  }

  /// @brief NOTE: returns 0-extent rect if there's no intersection
  /// @param other
  /// @return
  constexpr Rect intersect(Rect other) const
  {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    if (!overlaps(other))
    {
      return Rect{.offset = offset, .extent = Vec2{0, 0}};
    }

    Vec2 intersect_offset{std::max(x1_min, x2_min), std::max(y1_min, y2_min)};
    Vec2 intersect_extent{std::min(x1_max, x2_max) - offset.x,
                          std::min(y1_max, y2_max) - offset.y};

    return Rect{.offset = intersect_offset, .extent = intersect_extent};
  }

  constexpr bool contains(Vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y &&
           (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
  }

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0;
  }

  constexpr Vec2 top_left() const
  {
    return offset;
  }

  constexpr Vec2 top_right() const
  {
    return offset + Vec2{extent.x, 0};
  }

  constexpr Vec2 bottom_left() const
  {
    return offset + Vec2{0, extent.y};
  }

  constexpr Vec2 bottom_right() const
  {
    return offset + extent;
  }

  constexpr Quad to_quad() const
  {
    return Quad{.p0 = top_left(),
                .p1 = top_right(),
                .p2 = bottom_right(),
                .p3 = bottom_left()};
  }

  constexpr Rect with_offset(Vec2 new_offset) const
  {
    return Rect{.offset = new_offset, .extent = extent};
  }

  constexpr Rect with_offset(f32 x, f32 y) const
  {
    return Rect{.offset = Vec2{x, y}, .extent = extent};
  }

  constexpr Rect with_extent(Vec2 new_extent) const
  {
    return Rect{.offset = offset, .extent = new_extent};
  }

  constexpr Rect with_extent(f32 w, f32 h) const
  {
    return Rect{.offset = offset, .extent = Vec2{w, h}};
  }

  constexpr Rect with_center(Vec2 center) const
  {
    return Rect{.offset = center - extent / 2, .extent = extent};
  }

  constexpr Rect with_center(f32 cx, f32 cy) const
  {
    return with_center(Vec2{cx, cy});
  }

  constexpr Rect centered() const
  {
    return with_center(offset);
  }
};

struct Vec3
{
  f32 x = 0, y = 0, z = 0, __padding__ = 0;

  static constexpr Vec3 splat(f32 v)
  {
    return Vec3{.x = v, .y = v, .z = v};
  }

  constexpr f32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};



struct Box
{
  Vec3 offset, extent;

  constexpr f32 volume() const
  {
    return extent.x * extent.y * extent.z;
  }

  constexpr Vec3 min() const
  {
    return offset;
  }

  constexpr Vec3 max() const
  {
    return offset + extent;
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

/// column vector
struct Vec4
{
  f32 x = 0, y = 0, z = 0, w = 0;

  static constexpr Vec4 splat(f32 v)
  {
    return Vec4{.x = v, .y = v, .z = v, .w = v};
  }

  constexpr f32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};


constexpr Mat3 operator*(Mat3 a, f32 b)
{
  return Mat3{.rows = {a[0] * b, a[1] * b, a[2] * b}};
}

constexpr Mat3 operator*(f32 a, Mat3 b)
{
  return Mat3{.rows = {a * b[0], a * b[1], a * b[2]}};
}

constexpr Vec3 operator*(Mat3 const &a, Vec3 const &b)
{
}


/// row-major
struct Mat4
{
  Vec4 rows[4];





  constexpr Vec4 &operator[](usize i)
  {
    return rows[i];
  }

  constexpr Vec4 const &operator[](usize i) const
  {
    return rows[i];
  }
};

constexpr Mat4 operator*(Mat4 a, f32 b)
{
  return Mat4{.rows = {a[0] * b, a[1] * b, a[2] * b, a[3] * b}};
}

constexpr Mat4 operator*(f32 a, Mat4 b)
{
  return Mat4{.rows = {a * b[0], a * b[1], a * b[2], a * b[3]}};
}

constexpr Quad transform2d(Mat3 const &a, Rect const &b)
{
  return Quad{.p0 = transform2d(a, b.top_left()),
              .p1 = transform2d(a, b.top_right()),
              .p2 = transform2d(a, b.bottom_right()),
              .p3 = transform2d(a, b.bottom_left())};
}



  constexpr Extent constrain(Extent other) const
  {
    return Extent{.width  = std::min(width, other.width),
                  .height = std::min(height, other.height)};
  }

  constexpr u64 area() const
  {
    return static_cast<u64>(width) * height;
  }

  constexpr u32 max_mip_levels() const
  {
    return log2_floor_u32(std::max(width, height)) + 1;
  }

  constexpr Extent at_mip_level(u32 mip_level) const
  {
    return Extent{.width = width >> mip_level, .height = height >> mip_level};
  }


  rect_contains();


struct URect
{
  constexpr bool contains(URect const &other) const
  {
    return (offset.x <= other.offset.x) &&
           ((offset.x + extent.width) >=
            (other.offset.x + other.extent.width)) &&
           (offset.y <= other.offset.y) &&
           ((offset.y + extent.height) >=
            (other.offset.y + other.extent.height));
  }
};

struct URect3D
{
  constexpr bool contains(URect3D const &other) const
  {
    return (offset.x <= other.offset.x) &&
           ((offset.x + extent.width) >=
            (other.offset.x + other.extent.width)) &&
           (offset.y <= other.offset.y) &&
           ((offset.y + extent.height) >=
            (other.offset.y + other.extent.height)) &&
           (offset.z <= other.offset.z) &&
           ((offset.z + extent.depth) >= (other.offset.z + other.extent.depth));
  }
};





Vec4 normalize_color(Vec4U8);
  constexpr Vec4 to_normalized_vec() const
  {
    return Vec4{
        .x = r / 255.0f, .y = g / 255.0f, .z = b / 255.0f, .w = a / 255.0f};
  }
};

template <>
constexpr Color lerp<Color>(Color const &a, Color const &b, f32 t)
{
  return Color{
      .r = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.r, b.r, t), 0, 255)),
      .g = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.g, b.g, t), 0, 255)),
      .b = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.b, b.b, t), 0, 255)),
      .a = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.a, b.a, t), 0, 255))};
}

struct Version
{
 u8 variant, u8 major = 0, minor = 0, patch = 0;
};

/// @x_mag: The floating-point horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @y_mag: The floating-point vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @z_far: The floating-point distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
/// @z_near: The floating-point distance to the near clipping plane.
struct OrthographicCamera
{
  f32 x_mag  = 0;
  f32 y_mag  = 0;
  f32 z_far  = 0;
  f32 z_near = 0;
};

/// @aspect_ratio: The floating-point aspect ratio of the field of view.
/// @y_fov: The floating-point vertical field of view in radians. This value
/// SHOULD be less than π.
/// @z_far: The floating-point distance to the far clipping plane.
/// @z_near: The floating-point distance to the near clipping plane.
struct PerspectiveCamera
{
  f32 aspect_ratio = 0;
  f32 y_fov        = 0;
  f32 z_far        = 0;
  f32 z_near       = 0;
};
*/
}        // namespace ash
