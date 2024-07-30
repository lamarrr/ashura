/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include <bit>
#include <math.h>

#if ASH_CFG(COMPILER, MSVC)
#  include <intrin.h>
#endif

namespace ash
{

typedef struct Rect  Rect;
typedef struct CRect CRect;
typedef struct RectU RectU;
typedef struct Box   Box;
typedef struct CBox  CBox;

inline f32 length(Vec2 a)
{
  return sqrtf(dot(a, a));
}

inline f32 length(Vec3 a)
{
  return sqrtf(dot(a, a));
}

inline f32 length(Vec4 a)
{
  return sqrtf(dot(a, a));
}

template <typename SignedType>
constexpr SignedType abs(SignedType x)
{
  return x > SignedType{} ? x : -x;
}

constexpr bool approx_equal(f32 a, f32 b)
{
  return abs(b - a) <= F32_EPSILON;
}

constexpr bool approx_equal(f64 a, f64 b)
{
  return abs(b - a) <= F64_EPSILON;
}

constexpr f32 epsilon_clamp(f32 x)
{
  return abs(x) > F32_EPSILON ? x : F32_EPSILON;
}

constexpr f32 to_radians(f32 degree)
{
  return PI * degree * 0.00555555555f;
}

constexpr f64 to_radians(f64 degree)
{
  return PI * degree * 0.00555555555;
}

/// @brief Calculate log base 2 of an unsigned integer. Undefined behaviour if
/// value is 0
constexpr u8 ulog2(u8 value)
{
  return 7 - std::countl_zero(value);
}

/// @brief Calculate log base 2 of an unsigned integer. Undefined behaviour if
/// value is 0
constexpr u16 ulog2(u16 value)
{
  return 15 - std::countl_zero(value);
}

/// @brief Calculate log base 2 of an unsigned integer. Undefined behaviour if
/// value is 0
constexpr u32 ulog2(u32 value)
{
  return 31 - std::countl_zero(value);
}

/// @brief Calculate log base 2 of an unsigned integer. Undefined behaviour if
/// value is 0
constexpr u64 ulog2(u64 value)
{
  return 63 - std::countl_zero(value);
}

constexpr u32 mip_down(u32 a, u32 level)
{
  return max(a >> level, 1U);
}

constexpr Vec2U mip_down(Vec2U a, u32 level)
{
  return Vec2U{max(a.x >> level, 1U), max(a.y >> level, 1U)};
}

constexpr Vec3U mip_down(Vec3U a, u32 level)
{
  return Vec3U{max(a.x >> level, 1U), max(a.y >> level, 1U),
               max(a.z >> level, 1U)};
}

constexpr Vec4U mip_down(Vec4U a, u32 level)
{
  return Vec4U{max(a.x >> level, 1U), max(a.y >> level, 1U),
               max(a.z >> level, 1U), max(a.w >> level, 1U)};
}

constexpr u32 num_mip_levels(u32 a)
{
  return (a == 0) ? 0 : ulog2(a);
}

constexpr u32 num_mip_levels(Vec2U a)
{
  u32 max = ash::max(a.x, a.y);
  return (max == 0) ? 0 : (ulog2(max) + 1);
}

constexpr u32 num_mip_levels(Vec3U a)
{
  u32 max = ash::max(ash::max(a.x, a.y), a.z);
  return (max == 0) ? 0 : (ulog2(max) + 1);
}

constexpr u32 num_mip_levels(Vec4U a)
{
  u32 max = ash::max(ash::max(ash::max(a.x, a.y), a.z), a.w);
  return (max == 0) ? 0 : (ulog2(max) + 1);
}

constexpr Mat2 transpose(Mat2 const &a)
{
  return Mat2{.rows = {a.x(), a.y()}};
}

constexpr Mat3 transpose(Mat3 const &a)
{
  return Mat3{.rows = {a.x(), a.y(), a.z()}};
}

constexpr Mat4 transpose(Mat4 const &a)
{
  return Mat4{.rows = {a.x(), a.y(), a.z(), a.w()}};
}

constexpr f32 determinant(Mat2 const &a)
{
  return a[0].x * a[1].y - a[1].x * a[0].y;
}

constexpr f32 determinant(Mat3 const &a)
{
  return a[0].x * a[1].y * a[2].z - a[0].x * a[1].z * a[2].y -
         a[0].y * a[1].x * a[2].z + a[0].y * a[1].z * a[2].x +
         a[0].z * a[1].x * a[2].y - a[0].z * a[1].y * a[2].x;
}

constexpr f32 determinant(Mat4 const &a)
{
  return a[0].x * (a[1].y * a[2].z * a[3].w + a[1].z * a[2].w * a[3].y +
                   a[1].w * a[2].y * a[3].z - a[1].w * a[2].z * a[3].y -
                   a[1].z * a[2].y * a[3].w - a[1].y * a[2].w * a[3].z) -
         a[1].x * (a[0].y * a[2].z * a[3].w + a[0].z * a[2].w * a[3].y +
                   a[0].w * a[2].y * a[3].z - a[0].w * a[2].z * a[3].y -
                   a[0].z * a[2].y * a[3].w - a[0].y * a[2].w * a[3].z) +
         a[2].x * (a[0].y * a[1].z * a[3].w + a[0].z * a[1].w * a[3].y +
                   a[0].w * a[1].y * a[3].z - a[0].w * a[1].z * a[3].y -
                   a[0].z * a[1].y * a[3].w - a[0].y * a[1].w * a[3].z) -
         a[3].x * (a[0].y * a[1].z * a[2].w + a[0].z * a[1].w * a[2].y +
                   a[0].w * a[1].y * a[2].z - a[0].w * a[1].z * a[2].y -
                   a[0].z * a[1].y * a[2].w - a[0].y * a[1].w * a[2].z);
}

constexpr Mat2 adjoint(Mat2 const &a)
{
  return Mat2{.rows = {{a[1].y, -a[0].y}, {-a[1].x, a[0].x}}};
}

constexpr Mat3 adjoint(Mat3 const &a)
{
  return Mat3{
      .rows = {
          {a[1].y * a[2].z - a[1].z * a[2].y, a[0].z * a[2].y - a[0].y * a[2].z,
           a[0].y * a[1].z - a[0].z * a[1].y},
          {a[1].z * a[2].x - a[1].x * a[2].z, a[0].x * a[2].z - a[0].z * a[2].x,
           a[0].z * a[1].x - a[0].x * a[1].z},
          {a[1].x * a[2].y - a[1].y * a[2].x, a[0].y * a[2].x - a[0].x * a[2].y,
           a[0].x * a[1].y - a[0].y * a[1].x}}};
}

constexpr Mat4 adjoint(Mat4 const &a)
{
  Mat4 r;
  r[0].x = a[1].y * a[2].z * a[3].w + a[1].z * a[2].w * a[3].y +
           a[1].w * a[2].y * a[3].z - a[1].w * a[2].z * a[3].y -
           a[1].z * a[2].y * a[3].w - a[1].y * a[2].w * a[3].z;
  r[0].y = -a[0].y * a[2].z * a[3].w - a[0].z * a[2].w * a[3].y -
           a[0].w * a[2].y * a[3].z + a[0].w * a[2].z * a[3].y +
           a[0].z * a[2].y * a[3].w + a[0].y * a[2].w * a[3].z;
  r[0].z = a[0].y * a[1].z * a[3].w + a[0].z * a[1].w * a[3].y +
           a[0].w * a[1].y * a[3].z - a[0].w * a[1].z * a[3].y -
           a[0].z * a[1].y * a[3].w - a[0].y * a[1].w * a[3].z;
  r[0].w = -a[0].y * a[1].z * a[2].w - a[0].z * a[1].w * a[2].y -
           a[0].w * a[1].y * a[2].z + a[0].w * a[1].z * a[2].y +
           a[0].z * a[1].y * a[2].w + a[0].y * a[1].w * a[2].z;
  r[1].x = -a[1].x * a[2].z * a[3].w - a[1].z * a[2].w * a[3].x -
           a[1].w * a[2].x * a[3].z + a[1].w * a[2].z * a[3].x +
           a[1].z * a[2].x * a[3].w + a[1].x * a[2].w * a[3].z;
  r[1].y = a[0].x * a[2].z * a[3].w + a[0].z * a[2].w * a[3].x +
           a[0].w * a[2].x * a[3].z - a[0].w * a[2].z * a[3].x -
           a[0].z * a[2].x * a[3].w - a[0].x * a[2].w * a[3].z;
  r[1].z = -a[0].x * a[1].z * a[3].w - a[0].z * a[1].w * a[3].x -
           a[0].w * a[1].x * a[3].z + a[0].w * a[1].z * a[3].x +
           a[0].z * a[1].x * a[3].w + a[0].x * a[1].w * a[3].z;
  r[1].w = a[0].x * a[1].z * a[2].w + a[0].z * a[1].w * a[2].x +
           a[0].w * a[1].x * a[2].z - a[0].w * a[1].z * a[2].x -
           a[0].z * a[1].x * a[2].w - a[0].x * a[1].w * a[2].z;
  r[2].x = a[1].x * a[2].y * a[3].w + a[1].y * a[2].w * a[3].x +
           a[1].w * a[2].x * a[3].y - a[1].w * a[2].y * a[3].x -
           a[1].y * a[2].y * a[3].w - a[1].x * a[2].w * a[3].y;
  r[2].y = -a[0].x * a[2].y * a[3].w - a[0].y * a[2].w * a[3].x -
           a[0].w * a[2].x * a[3].y + a[0].w * a[2].y * a[3].x +
           a[0].y * a[2].x * a[3].w + a[0].x * a[2].w * a[3].y;
  r[2].z = a[0].x * a[1].y * a[3].w + a[0].y * a[1].w * a[3].x +
           a[0].w * a[1].x * a[3].y - a[0].w * a[1].y * a[3].x -
           a[0].y * a[1].x * a[3].w - a[0].x * a[1].w * a[3].y;
  r[2].w = -a[0].x * a[1].y * a[2].w - a[0].y * a[1].w * a[2].x -
           a[0].w * a[1].x * a[2].y + a[0].w * a[1].y * a[2].x +
           a[0].y * a[1].x * a[2].w + a[0].x * a[1].w * a[2].y;
  r[3].x = -a[1].x * a[2].y * a[3].z - a[1].y * a[2].z * a[3].x -
           a[1].z * a[2].x * a[3].y + a[1].z * a[2].y * a[3].x +
           a[1].y * a[2].x * a[3].z + a[1].x * a[2].z * a[3].y;
  r[3].y = a[0].x * a[2].y * a[3].z + a[0].y * a[2].z * a[3].x +
           a[0].z * a[2].x * a[3].y - a[0].z * a[2].y * a[3].x -
           a[0].y * a[2].x * a[3].z - a[0].x * a[2].z * a[3].y;
  r[3].z = -a[0].x * a[1].y * a[3].z - a[0].y * a[1].z * a[3].x -
           a[0].z * a[1].x * a[3].y + a[0].z * a[1].y * a[3].x +
           a[0].y * a[1].x * a[3].z + a[0].x * a[1].z * a[3].y;
  r[3].w = a[0].x * a[1].y * a[2].z + a[0].y * a[1].z * a[2].y +
           a[0].z * a[1].x * a[2].y - a[0].z * a[1].y * a[2].x -
           a[0].y * a[1].x * a[2].z - a[0].x * a[1].z * a[2].y;
  return r;
}

constexpr Mat2 inverse(Mat2 a)
{
  return Mat2::splat(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3 inverse(Mat3 const &a)
{
  return Mat3::splat(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat4 inverse(Mat4 const &a)
{
  return Mat4::splat(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3Affine translate2d(Vec2 t)
{
  return Mat3Affine{.rows = {{1, 0, t.x}, {0, 1, t.y}}};
}

constexpr Mat4Affine translate3d(Vec3 t)
{
  return Mat4Affine{.rows = {{1, 0, 0, t.x}, {0, 1, 0, t.y}, {0, 0, 1, t.z}}};
}

constexpr Mat3Affine scale2d(Vec2 s)
{
  return Mat3Affine{.rows = {{s.x, 0, 0}, {0, s.y, 0}}};
}

constexpr Mat4Affine scale3d(Vec3 s)
{
  return Mat4Affine{.rows = {{s.x, 0, 0, 0}, {0, s.y, 0, 0}, {0, 0, s.z, 0}}};
}

inline Mat3Affine rotate2d(f32 radians)
{
  return Mat3Affine{.rows = {{cosf(radians), -sinf(radians), 0},
                             {sinf(radians), cosf(radians), 0}}};
}

inline Mat4Affine rotate3d_x(f32 radians)
{
  return Mat4Affine{.rows = {{1, 0, 0, 0},
                             {0, cosf(radians), -sinf(radians), 0},
                             {0, sinf(radians), cosf(radians), 0}}};
}

inline Mat4Affine rotate3d_y(f32 radians)
{
  return Mat4Affine{.rows = {{cosf(radians), 0, sinf(radians), 0},
                             {0, 1, 0, 0},
                             {-sinf(radians), 0, cosf(radians), 0}}};
}

inline Mat4Affine rotate3d_z(f32 radians)
{
  return Mat4Affine{.rows = {{cosf(radians), -sinf(radians), 0, 0},
                             {sinf(radians), cosf(radians), 0, 0},
                             {0, 0, 1, 0}}};
}

constexpr Vec2 transform(Mat3 const &t, Vec2 value)
{
  Vec3 v = t * Vec3{value.x, value.y, 1};
  return Vec2{v.x, v.y};
}

constexpr Vec2 transform(Mat3Affine const &t, Vec2 value)
{
  Vec3 v = t * Vec3{value.x, value.y, 1};
  return Vec2{v.x, v.y};
}

constexpr Vec3 transform(Mat4 const &t, Vec3 value)
{
  Vec4 v = t * Vec4{value.x, value.y, value.z, 1};
  return Vec3{v.x, v.y, v.z};
}

constexpr Vec3 transform(Mat4Affine const &t, Vec3 value)
{
  Vec4 v = t * Vec4{value.x, value.y, 1};
  return Vec3{v.x, v.y, v.z};
}

constexpr bool overlaps(Vec2 a_begin, Vec2 a_end, Vec2 b_begin, Vec2 b_end)
{
  return a_begin.x <= b_end.x && a_end.x >= b_begin.x && a_begin.y <= b_end.y &&
         a_end.y >= b_begin.y;
}

constexpr bool contains_point(Vec2 begin, Vec2 end, Vec2 point)
{
  return begin.x <= point.x && begin.y <= point.y && end.x >= point.x &&
         end.y >= point.y;
}

constexpr void intersect(Vec2 a_begin, Vec2 a_end, Vec2 &b_begin, Vec2 &b_end)
{
  if (!overlaps(a_begin, a_end, b_begin, b_end))
  {
    b_begin = {};
    b_end   = {};
    return;
  }

  b_begin = Vec2{max(a_begin.x, b_begin.x), max(a_begin.y, b_begin.y)};
  b_end   = Vec2{min(a_end.x, b_end.x), min(a_end.y, b_end.y)};
}

struct Rect
{
  Vec2 offset = {};
  Vec2 extent = {};

  constexpr Vec2 center() const
  {
    return offset + (extent * 0.5F);
  }

  constexpr Vec2 begin() const
  {
    return offset;
  }

  constexpr Vec2 end() const
  {
    return offset + extent;
  }

  constexpr f32 area() const
  {
    return extent.x * extent.y;
  }

  constexpr CRect centered() const;
};

struct CRect
{
  Vec2 center = {};
  Vec2 extent = {};

  constexpr Vec2 begin() const
  {
    return center - (extent * 0.5F);
  }

  constexpr Vec2 end() const
  {
    return center + extent * 0.5F;
  }

  constexpr f32 area() const
  {
    return extent.x * extent.y;
  }

  constexpr Rect offseted() const;
};

constexpr CRect Rect::centered() const
{
  return CRect{.center = offset + extent * 0.5F, .extent = extent};
}

constexpr Rect CRect::offseted() const
{
  return Rect{.offset = center - extent * 0.5F, .extent = extent};
}

constexpr bool operator==(CRect const &a, CRect const &b)
{
  return a.center == b.center && a.extent == b.extent;
}

constexpr bool operator!=(CRect const &a, CRect const &b)
{
  return a.center != b.center || a.extent != b.extent;
}

struct RectU
{
  Vec2U offset = {};
  Vec2U extent = {};

  constexpr Vec2U end() const
  {
    return offset + extent;
  }
};

constexpr bool operator==(RectU const &a, RectU const &b)
{
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(RectU const &a, RectU const &b)
{
  return a.offset != b.offset || a.extent != b.extent;
}

struct Box
{
  Vec3 offset = {};
  Vec3 extent = {};

  constexpr Vec3 center() const
  {
    return offset + (extent * 0.5F);
  }

  constexpr Vec3 end() const
  {
    return offset + extent;
  }

  constexpr f32 volume() const
  {
    return extent.x * extent.y * extent.z;
  }

  constexpr CBox centered() const;
};

struct CBox
{
  Vec3 center = {};
  Vec3 extent = {};

  constexpr Vec3 begin() const
  {
    return center - extent * 0.5F;
  }

  constexpr Vec3 end() const
  {
    return center + extent * 0.5F;
  }

  constexpr f32 volume() const
  {
    return extent.x * extent.y * extent.z;
  }

  constexpr Box offseted() const;
};

constexpr CBox Box::centered() const
{
  return CBox{.center = offset + extent * 0.5F, .extent = extent};
}

constexpr Box CBox::offseted() const
{
  return Box{.offset = center - extent * 0.5F, .extent = extent};
}

constexpr bool contains(Rect const &rect, Vec2 point)
{
  return contains_point(rect.begin(), rect.end(), point);
}

constexpr bool overlaps(Rect const &a, Rect const &b)
{
  return ash::overlaps(a.begin(), a.end(), b.begin(), b.end());
}

constexpr Rect intersect(Rect const &a, Rect const &b)
{
  Vec2 begin = b.begin();
  Vec2 end   = b.end();
  intersect(a.begin(), a.end(), begin, end);
  return Rect{.offset = begin, .extent = end - begin};
}

constexpr bool contains(Box const &box, Vec3 point)
{
  return box.offset.x <= point.x && box.offset.y <= point.y &&
         box.offset.z <= point.z && (box.offset.x + box.extent.x) >= point.x &&
         (box.offset.y + box.extent.y) >= point.y &&
         (box.offset.z + box.extent.z) >= point.z;
  return true;
}

constexpr bool overlaps(Box const &a, Box const &b)
{
  Vec3 a_begin = a.offset;
  Vec3 a_end   = a.offset + a.extent;
  Vec3 b_begin = b.offset;
  Vec3 b_end   = b.offset + b.extent;
  return a_begin.x <= b_end.x && a_end.x >= b_begin.x && a_begin.y <= b_end.y &&
         a_end.y >= b_begin.y && a_begin.z <= b_end.z && a_end.z >= b_begin.z;
}

inline Vec2 rotor(f32 a)
{
  return Vec2{cosf(a), sinf(a)};
}

/// @brief linearly interpolate between points `low` and `high` given
/// interpolator `t`
constexpr f32 lerp(f32 low, f32 high, f32 t)
{
  return (1 - t) * low + t * high;
}

/// @brief logarithmically interpolate between points `low` and `high` given
/// interpolator `t`
inline f32 log_interp(f32 low, f32 high, f32 t)
{
  return low * expf(t * logf(high / low));
}

/// @brief frame-independent damped lerp
///
/// https://x.com/FreyaHolmer/status/1757836988495847568,
/// https://www.rorydriscoll.com/2016/03/07/frame-rate-independent-damping-using-lerp/
///
/// @param dt time delta
/// @param half_life time to complete half of the whole operation
///
inline f32 damplerp(f32 low, f32 high, f32 dt, f32 half_life)
{
  return lerp(low, high, 1 - exp2f(-half_life * dt));
}

/// find interpolator t, given points a and b, and interpolated value v
constexpr f32 unlerp(f32 low, f32 high, f32 v)
{
  return (v - low) / (high - low);
}

constexpr f32 relerp(f32 in_low, f32 in_high, f32 out_low, f32 out_high, f32 &v)
{
  return lerp(out_low, out_high, unlerp(in_low, in_high, v));
}

// SEE: https://www.youtube.com/watch?v=jvPPXbo87ds
constexpr f32 linear(f32 t)
{
  return t;
}

constexpr f32 ease_in(f32 t)
{
  return t * t;
}

constexpr f32 ease_out(f32 t)
{
  return 1 - (1 - t) * (1 - t);
}

constexpr f32 ease_in_out(f32 t)
{
  return lerp(ease_in(t), ease_out(t), t);
}

constexpr f32 bezier(f32 p0, f32 p1, f32 p2, f32 t)
{
  return (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;
}

constexpr f32 cubic_bezier(f32 p0, f32 p1, f32 p2, f32 p3, f32 t)
{
  return (1 - t) * (1 - t) * (1 - t) * p0 + 3 * (1 - t) * (1 - t) * t * p1 +
         3 * (1 - t) * t * t * p2 + t * t * t * p3;
}

/// https://www.youtube.com/watch?v=jvPPXbo87ds&t=1033s - The Continuity of
/// Splines by Freya Holmer
///
/// has automatic tangent. use for animation and path smoothing
/// ne of the features of the Catmull-Rom spline is that the specified curve
/// will pass through all of the control points.
constexpr f32 catmull_rom(f32 p0, f32 p1, f32 p2, f32 p3, f32 t)
{
  return 0.5f *
         ((2 * p1) + (-p0 + p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
          (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t);
}

constexpr f32 step(f32 a, f32 t)
{
  return t < a ? 0.0f : 1.0f;
}

constexpr f32 smoothstep(f32 a, f32 b, f32 t)
{
  t = clamp((t - a) / (b - a), 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}

inline f32 grid_snap(f32 a, f32 unit)
{
  return floorf((a + unit * 0.5F) / unit) * unit;
}

/// @brief get the aligned center relative to a fixed amount of space
/// @param space the space to align to
/// @param alignment the alignment to align to [-1, +1]
/// @return
constexpr f32 space_align(f32 space, f32 content, f32 alignment)
{
  f32 const trailing = space - content;
  f32       padding  = (alignment * 0.5F + 0.5F) * trailing;
  return padding + content / 2;
}

constexpr Vec2 space_align(Vec2 space, Vec2 content, Vec2 alignment)
{
  return Vec2{space_align(space.x, content.x, alignment.x),
              space_align(space.y, content.y, alignment.y)};
}

constexpr f32 norm_to_axis(f32 norm)
{
  return norm * 2 - 1;
}

constexpr f32 axis_to_norm(f32 axis)
{
  return axis * 0.5F + 0.5F;
}

constexpr Vec2 norm_to_axis(Vec2 norm)
{
  return norm * 2 - 1;
}

constexpr Vec2 axis_to_norm(Vec2 axis)
{
  return axis * 0.5F + 0.5F;
}

constexpr Vec3 norm_to_axis(Vec3 norm)
{
  return norm * 2 - 1;
}

constexpr Vec3 axis_to_norm(Vec3 axis)
{
  return axis * 0.5F + 0.5F;
}

constexpr Vec4 norm_to_axis(Vec4 norm)
{
  return norm * 2 - 1;
}

constexpr Vec4 axis_to_norm(Vec4 axis)
{
  return axis * 0.5F + 0.5F;
}

constexpr Vec4 opacity(f32 v)
{
  return Vec4{1, 1, 1, v};
}

/// @param x_mag The horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @param y_mag The vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @param z_near The distance to the near clipping plane.
/// @param z_far The distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
constexpr Mat4Affine orthographic(f32 x_mag, f32 y_mag, f32 z_near, f32 z_far)
{
  f32 const z_diff = z_near - z_far;
  return Mat4Affine{{{1 / x_mag, 0, 0, 0},
                     {0, 1 / y_mag, 0, 0},
                     {0, 0, 2 / z_diff, (z_far + z_near) / z_diff}}};
}

/// @param aspect_ratio The aspect ratio of the field of view.
/// @param y_fov The vertical field of view in radians. This value
/// SHOULD be less than π.
/// @param z_far The distance to the far clipping plane.
/// @param z_near The distance to the near clipping plane.
inline Mat4 perspective(f32 aspect_ratio, f32 y_fov, f32 z_far, f32 z_near)
{
  f32 const s      = tanf(y_fov * 0.5F);
  f32 const z_diff = z_near - z_far;
  return Mat4{{{1 / (aspect_ratio * s), 0, 0, 0},
               {0, 1 / s, 0, 0},
               {0, 0, (z_far + z_near) / z_diff, (2 * z_far * z_near) / z_diff},
               {0, 0, -1, 0}}};
}

constexpr Mat4 look_at(Vec3 eye, Vec3 center, Vec3 up)
{
  Vec3 const f = normalize(center - eye);
  Vec3 const s = normalize(cross(up, f));
  Vec3 const u = cross(f, s);

  return {{{s.x, s.x, s.x, 0},
           {u.y, u.y, u.y, 0},
           {f.z, f.z, f.z, 0},
           {-dot(s, eye), -dot(u, eye), -dot(f, eye), 1}}};
}

/// @brief Given an object-clip space (mvp) matrix, determine if the object is
/// within the camera's area of view.
/// https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/b92d559bd083f44df9f8f42a6ad149c1584ae94c/src/common/Misc/Misc.cpp#L265
/// https://bruop.github.io/frustum_culling/
///
/// exploits the fact that in clip-space all vertices in the view frustum will
/// obey:
///
/// -w <= x <= w
/// -w <= y <= w
///  0 <= z <= w
///
constexpr bool is_outside_frustum(Mat4 const &mvp, Vec3 offset, Vec3 extent)
{
  constexpr u8 NUM_CORNERS          = 8;
  Vec4 const   corners[NUM_CORNERS] = {
      mvp * vec4(offset, 1),
      mvp * vec4(offset + Vec3{extent.x, 0, 0}, 1),
      mvp * vec4(offset + Vec3{extent.x, extent.y, 0}, 1),
      mvp * vec4(offset + Vec3{0, extent.y, 0}, 1),
      mvp * vec4(offset + Vec3{0, 0, extent.z}, 1),
      mvp * vec4(offset + Vec3{extent.x, 0, extent.z}, 1),
      mvp * vec4(offset + extent, 1),
      mvp * vec4(offset + Vec3{0, extent.y, extent.z}, 1)};
  u8 left   = 0;
  u8 right  = 0;
  u8 top    = 0;
  u8 bottom = 0;
  u8 back   = 0;

  for (u8 i = 0; i < NUM_CORNERS; i++)
  {
    Vec4 const &corner = corners[i];

    if (corner.x < -corner.w)
    {
      left++;
    }

    if (corner.x > corner.w)
    {
      right++;
    }

    if (corner.y < -corner.w)
    {
      bottom++;
    }

    if (corner.y > corner.w)
    {
      top++;
    }

    if (corner.z < 0)
    {
      back++;
    }
  }

  return left == NUM_CORNERS || right == NUM_CORNERS || top == NUM_CORNERS ||
         bottom == NUM_CORNERS || back == NUM_CORNERS;
}

constexpr void frustum_cull(Mat4 const &mvp, Span<Mat4 const> global_transform,
                            Span<Box const> aabb, Span<u64> is_visible)
{
  for (u32 i = 0; i < aabb.size32(); i++)
  {
    if (is_outside_frustum(mvp * global_transform[i], aabb[i].offset,
                           aabb[i].extent))
    {
      clear_bit(is_visible, i);
    }
    else
    {
      set_bit(is_visible, i);
    }
  }
}

}        // namespace ash
