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

// Undefined behaviour if value is 0
inline u32 u32log2(u32 value)
{
#if ASH_CFG(COMPILER, MSVC)
  return 31U - __lzcnt(value);
#else
#  if defined(__has_builtin) && __has_builtin(__builtin_clz)
  return 31U - __builtin_clz(value);
#  else
  return 31U - std::count_lzero(value);
#  endif
#endif
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

inline u32 num_mip_levels(u32 a)
{
  return a == 0 ? 0 : u32log2(a);
}

inline u32 num_mip_levels(Vec2U a)
{
  u32 max = ash::max(a.x, a.y);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

inline u32 num_mip_levels(Vec3U a)
{
  u32 max = ash::max(ash::max(a.x, a.y), a.z);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

inline u32 num_mip_levels(Vec4U a)
{
  u32 max = ash::max(ash::max(ash::max(a.x, a.y), a.z), a.w);
  return max == 0 ? 0 : (u32log2(max) + 1);
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
  return Mat2::uniform(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3 inverse(Mat3 const &a)
{
  return Mat3::uniform(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat4 inverse(Mat4 const &a)
{
  return Mat4::uniform(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3 translate2d(Vec2 t)
{
  return Mat3{.rows = {{1, 0, t.x}, {0, 1, t.y}, Mat3Affine::trailing_row}};
}

constexpr Mat3Affine affine_translate2d(Vec2 t)
{
  return Mat3Affine{.rows = {{1, 0, t.x}, {0, 1, t.y}}};
}

constexpr Mat4 translate3d(Vec3 t)
{
  return Mat4{.rows = {{1, 0, 0, t.x},
                       {0, 1, 0, t.y},
                       {0, 0, 1, t.z},
                       Mat4Affine::trailing_row}};
}

constexpr Mat4Affine affine_translate3d(Vec3 t)
{
  return Mat4Affine{.rows = {{1, 0, 0, t.x}, {0, 1, 0, t.y}, {0, 0, 1, t.z}}};
}

constexpr Mat3 scale2d(Vec2 s)
{
  return Mat3{.rows = {{s.x, 0, 0}, {0, s.y, 0}, Mat3Affine::trailing_row}};
}

constexpr Mat4 scale3d(Vec3 s)
{
  return Mat4{.rows = {{s.x, 0, 0, 0},
                       {0, s.y, 0, 0},
                       {0, 0, s.z, 0},
                       Mat4Affine::trailing_row}};
}

constexpr Mat3Affine affine_scale2d(Vec2 s)
{
  return Mat3Affine{.rows = {{s.x, 0, 0}, {0, s.y, 0}}};
}

constexpr Mat4Affine affine_scale3d(Vec3 s)
{
  return Mat4Affine{.rows = {{s.x, 0, 0, 0}, {0, s.y, 0, 0}, {0, 0, s.z, 0}}};
}

inline Mat3 rotate2d(f32 radians)
{
  return Mat3{.rows = {{cosf(radians), -sinf(radians), 0},
                       {sinf(radians), cosf(radians), 0},
                       Mat3Affine::trailing_row}};
}

inline Mat3Affine affine_rotate2d(f32 radians)
{
  return Mat3Affine{.rows = {{cosf(radians), -sinf(radians), 0},
                             {sinf(radians), cosf(radians), 0}}};
}

inline Mat4 rotate3d_x(f32 radians)
{
  return Mat4{.rows = {{1, 0, 0, 0},
                       {0, cosf(radians), -sinf(radians), 0},
                       {0, sinf(radians), cosf(radians), 0},
                       Mat4Affine::trailing_row}};
}

inline Mat4Affine affine_rotate3d_x(f32 radians)
{
  return Mat4Affine{.rows = {{1, 0, 0, 0},
                             {0, cosf(radians), -sinf(radians), 0},
                             {0, sinf(radians), cosf(radians), 0}}};
}

inline Mat4 rotate3d_y(f32 radians)
{
  return Mat4{.rows = {{cosf(radians), 0, sinf(radians), 0},
                       {0, 1, 0, 0},
                       {-sinf(radians), 0, cosf(radians), 0},
                       Mat4Affine::trailing_row}};
}

inline Mat4Affine affine_rotate3d_y(f32 radians)
{
  return Mat4Affine{.rows = {{cosf(radians), 0, sinf(radians), 0},
                             {0, 1, 0, 0},
                             {-sinf(radians), 0, cosf(radians), 0}}};
}

inline Mat4 rotate3d_z(f32 radians)
{
  return Mat4{.rows = {{cosf(radians), -sinf(radians), 0, 0},
                       {sinf(radians), cosf(radians), 0, 0},
                       {0, 0, 1, 0},
                       Mat4Affine::trailing_row}};
}

inline Mat4Affine affine_rotate3d_z(f32 radians)
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

constexpr bool rect_overlaps(Vec2 a_begin, Vec2 a_end, Vec2 b_begin, Vec2 b_end)
{
  return a_begin.x <= b_end.x && a_end.x >= b_begin.x && a_begin.y <= b_end.y &&
         a_end.y >= b_begin.y;
}

constexpr bool rect_contains_point(Vec2 begin, Vec2 end, Vec2 point)
{
  return begin.x <= point.x && begin.y <= point.y && end.x >= point.x &&
         end.y >= point.y;
}

constexpr void rect_intersect(Vec2 a_begin, Vec2 a_end, Vec2 &b_begin,
                              Vec2 &b_end)
{
  if (!rect_overlaps(a_begin, a_end, b_begin, b_end))
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
  Vec2 offset;
  Vec2 extent;

  constexpr Vec2 center() const
  {
    return offset + (extent / 2);
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

  constexpr bool contains(Vec2 point) const
  {
    return rect_contains_point(begin(), end(), point);
  }

  constexpr bool overlaps(Rect const &other) const
  {
    return rect_overlaps(begin(), end(), other.offset, other.end());
  }

  constexpr Rect intersect(Rect const &other) const
  {
    Vec2 b = other.begin();
    Vec2 e = other.end();
    rect_intersect(begin(), end(), b, e);
    return Rect{.offset = b, .extent = e - b};
  }
};

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

// find intepolated value v, given points a and b, and interpolator t
constexpr f32 lerp(f32 low, f32 high, f32 t)
{
  return (1 - t) * low + t * high;
}

/// @brief frame-independent damped lerp
///
/// https://x.com/FreyaHolmer/status/1757836988495847568,
/// https://www.rorydriscoll.com/2016/03/07/frame-rate-independent-damping-using-lerp/
///
/// @param dt time delta
/// @param half_life time to complete half of the whole operation
///
constexpr f32 damplerp(f32 low, f32 high, f32 dt, f32 half_life)
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

/// bezier spline: shapes, fonts, vector graphics. use
/// multiple bezier curves, joined together uniformly by a unit. unit - knot
/// interval index - (knot value/knot interval)
/// take 4 points, advance by 3
void spline_interp();

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
  return floorf((a + unit / 2) / unit) * unit;
}

}        // namespace ash
