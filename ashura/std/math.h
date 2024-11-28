/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include <bit>
#include <math.h>

namespace ash
{

inline f32 sin(f32 v)
{
  return std::sin(v);
}

inline f64 sin(f64 v)
{
  return std::sin(v);
}

inline f32 cos(f32 v)
{
  return std::cos(v);
}

inline f64 cos(f64 v)
{
  return std::cos(v);
}

inline f32 tan(f32 v)
{
  return std::tan(v);
}

inline f64 tan(f64 v)
{
  return std::tan(v);
}

inline f32 exp(f32 v)
{
  return std::exp(v);
}

inline f64 exp(f64 v)
{
  return std::exp(v);
}

inline f32 exp2(f32 v)
{
  return std::exp2(v);
}

inline f64 exp2(f64 v)
{
  return std::exp2(v);
}

inline f32 log(f32 v)
{
  return std::log(v);
}

inline f64 log(f64 v)
{
  return std::log(v);
}

inline f32 floor(f32 v)
{
  return std::floor(v);
}

inline f64 floor(f64 v)
{
  return std::floor(v);
}

template <typename SignedType>
constexpr SignedType abs(SignedType x)
{
  return x > SignedType{} ? x : -x;
}

constexpr bool approx_eq(f32 a, f32 b)
{
  return abs(b - a) <= F32_EPS;
}

constexpr bool approx_eq(f64 a, f64 b)
{
  return abs(b - a) <= F64_EPS;
}

constexpr f32 epsilon_clamp(f32 x)
{
  return abs(x) > F32_EPS ? x : F32_EPS;
}

constexpr f32 to_radians(f32 degree)
{
  return PI * degree * 0.00555555555F;
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

constexpr u32 num_mip_levels(u32 a)
{
  return (a == 0) ? 0 : ulog2(a);
}

/// @brief linearly interpolate between points `low` and `high` given
/// interpolator `t`
/// This is the exact form: (1 - t) * A + T * B, optimized for FMA
template <typename T>
constexpr T lerp(T const &low, T const &high, T const &t)
{
  return low - t * low + t * high;
}

/// @brief logarithmically interpolate between points `low` and `high` given
/// interpolator `t`
template <typename T>
inline T log_interp(T const &low, T const &high, T const &t)
{
  return low * exp(t * log(high / low));
}

/// @brief frame-independent damped lerp
///
/// https://x.com/FreyaHolmer/status/1757836988495847568,
/// https://www.rorydriscoll.com/2016/03/07/frame-rate-independent-damping-using-lerp/
///
/// @param dt time delta
/// @param half_life time to complete half of the whole operation
///
template <typename T>
inline T damplerp(T const &low, T const &high, T const &dt, T const &half_life)
{
  return lerp(low, high, 1 - exp2(-half_life * dt));
}

/// find interpolator t, given points a and b, and interpolated value v
template <typename T>
constexpr T unlerp(T const &low, T const &high, T const &v)
{
  return (v - low) / (high - low);
}

template <typename T>
constexpr T relerp(T const &in_low, T const &in_high, T const &out_low,
                   T const &out_high, T const &v)
{
  return lerp(out_low, out_high, unlerp(in_low, in_high, v));
}

// SEE: https://www.youtube.com/watch?v=jvPPXbo87ds
template <typename T>
constexpr T ease_in(T const &t)
{
  return t * t;
}

template <typename T>
constexpr T ease_out(T const &t)
{
  return 1 - (1 - t) * (1 - t);
}

template <typename T>
constexpr T ease_in_out(T const &t)
{
  return lerp(ease_in(t), ease_out(t), t);
}

template <typename T>
constexpr T bezier(T const &p0, T const &p1, T const &p2, T const &t)
{
  return (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;
}

template <typename T>
constexpr T cubic_bezier(T const &p0, T const &p1, T const &p2, T const &p3,
                         T const &t)
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
template <typename T>
constexpr T catmull_rom(T const &p0, T const &p1, T const &p2, T const &p3,
                        T const &t)
{
  return 0.5F *
         ((2 * p1) + (-p0 + p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
          (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t);
}

constexpr f32 step(f32 a, f32 t)
{
  return (t < a) ? 0.0F : 1.0F;
}

constexpr f32 smoothstep(f32 a, f32 b, f32 t)
{
  t = clamp((t - a) / (b - a), 0.0F, 1.0F);
  return t * t * (3.0F - 2.0F * t);
}

template <typename T>
inline T grid_snap(T const &a, T const &unit)
{
  return floor((a + unit * 0.5F) / unit) * unit;
}

template <typename T>
constexpr T norm_to_axis(T const &norm)
{
  return norm * 2 - 1;
}

template <typename T>
constexpr T axis_to_norm(T const &axis)
{
  return axis * 0.5F + 0.5F;
}

template <typename T>
constexpr T norm_to_space(T const &norm)
{
  return norm - 0.5F;
}

template <typename T>
constexpr T space_to_norm(T const &space)
{
  return space + 0.5F;
}

template <typename T>
constexpr T space_to_axis(T const &space)
{
  return space * 2;
}

template <typename T>
constexpr T axis_to_space(T const &axis)
{
  return axis * 0.5F;
}

/// @param space available space to align to
/// @param item extent of the item to align
/// @param alignment the alignment to align to [-1, +1]
/// @return returns the aligned position relative to the space's center
template <typename T>
constexpr T space_align(T const &space, T const &item, T const &alignment)
{
  T const trailing = (space - item) * 0.5F;
  return lerp(-trailing, trailing, axis_to_norm(alignment));
}

typedef struct Vec2   Vec2;
typedef struct Vec3   Vec3;
typedef struct Vec4   Vec4;
typedef struct Vec4U8 Vec4U8;
typedef struct Vec2I  Vec2I;
typedef struct Vec3I  Vec3I;
typedef struct Vec4I  Vec4I;
typedef struct Vec2U  Vec2U;
typedef struct Vec3U  Vec3U;
typedef struct Vec4U  Vec4U;

typedef struct Rect  Rect;
typedef struct CRect CRect;
typedef struct RectU RectU;
typedef struct Box   Box;
typedef struct CBox  CBox;

enum class Axis : u8
{
  X = 0,
  Y = 1,
  Z = 2,
  W = 3
};

enum class Axes : u8
{
  None = 0x00,
  X    = 0x01,
  Y    = 0x02,
  Z    = 0x04,
  W    = 0x08
};

ASH_DEFINE_ENUM_BIT_OPS(Axes)

struct alignas(8) Vec2
{
  f32 x = 0;
  f32 y = 0;

  static constexpr Vec2 splat(f32 value)
  {
    return Vec2{value, value};
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

constexpr bool operator==(Vec2 a, Vec2 b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Vec2 a, Vec2 b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr Vec2 operator+(Vec2 a, Vec2 b)
{
  return Vec2{a.x + b.x, a.y + b.y};
}

constexpr Vec2 operator+(Vec2 a, f32 b)
{
  return Vec2{a.x + b, a.y + b};
}

constexpr Vec2 operator+(f32 a, Vec2 b)
{
  return Vec2{a + b.x, a + b.y};
}

constexpr Vec2 operator-(Vec2 a)
{
  return Vec2{-a.x, -a.y};
}

constexpr Vec2 operator-(Vec2 a, Vec2 b)
{
  return Vec2{a.x - b.x, a.y - b.y};
}

constexpr Vec2 operator-(Vec2 a, f32 b)
{
  return Vec2{a.x - b, a.y - b};
}

constexpr Vec2 operator-(f32 a, Vec2 b)
{
  return Vec2{a - b.x, a - b.y};
}

constexpr Vec2 operator*(Vec2 a, Vec2 b)
{
  return Vec2{a.x * b.x, a.y * b.y};
}

constexpr Vec2 operator*(Vec2 a, f32 b)
{
  return Vec2{a.x * b, a.y * b};
}

constexpr Vec2 operator*(f32 a, Vec2 b)
{
  return Vec2{a * b.x, a * b.y};
}

constexpr Vec2 operator/(Vec2 a, Vec2 b)
{
  return Vec2{a.x / b.x, a.y / b.y};
}

constexpr Vec2 operator/(Vec2 a, f32 b)
{
  return Vec2{a.x / b, a.y / b};
}

constexpr Vec2 operator/(f32 a, Vec2 b)
{
  return Vec2{a / b.x, a / b.y};
}

constexpr Vec2 &operator+=(Vec2 &a, Vec2 b)
{
  a = a + b;
  return a;
}

constexpr Vec2 &operator-=(Vec2 &a, Vec2 b)
{
  a = a - b;
  return a;
}

constexpr Vec2 &operator*=(Vec2 &a, Vec2 b)
{
  a = a * b;
  return a;
}

constexpr Vec2 &operator/=(Vec2 &a, Vec2 b)
{
  a = a / b;
  return a;
}

struct Vec3
{
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;

  static constexpr Vec3 splat(f32 value)
  {
    return Vec3{value, value, value};
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

constexpr Vec3 vec3(Vec2 xy, f32 z)
{
  return Vec3{xy.x, xy.y, z};
}

constexpr Vec3 vec3(f32 x, Vec2 yz)
{
  return Vec3{x, yz.x, yz.y};
}

constexpr bool operator==(Vec3 a, Vec3 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(Vec3 a, Vec3 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr Vec3 operator+(Vec3 a, Vec3 b)
{
  return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr Vec3 operator+(Vec3 a, f32 b)
{
  return Vec3{a.x + b, a.y + b, a.z + b};
}

constexpr Vec3 operator+(f32 a, Vec3 b)
{
  return Vec3{a + b.x, a + b.y, a + b.z};
}

constexpr Vec3 operator-(Vec3 a)
{
  return Vec3{-a.x, -a.y, -a.z};
}

constexpr Vec3 operator-(Vec3 a, Vec3 b)
{
  return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr Vec3 operator-(Vec3 a, f32 b)
{
  return Vec3{a.x - b, a.y - b, a.z - b};
}

constexpr Vec3 operator-(f32 a, Vec3 b)
{
  return Vec3{a - b.x, a - b.y, a - b.z};
}

constexpr Vec3 operator*(Vec3 a, Vec3 b)
{
  return Vec3{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr Vec3 operator*(Vec3 a, f32 b)
{
  return Vec3{a.x * b, a.y * b, a.z * b};
}

constexpr Vec3 operator*(f32 a, Vec3 b)
{
  return Vec3{a * b.x, a * b.y, a * b.z};
}

constexpr Vec3 operator/(Vec3 a, Vec3 b)
{
  return Vec3{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr Vec3 operator/(Vec3 a, f32 b)
{
  return Vec3{a.x / b, a.y / b, a.z / b};
}

constexpr Vec3 operator/(f32 a, Vec3 b)
{
  return Vec3{a / b.x, a / b.y, a / b.z};
}

constexpr Vec3 &operator+=(Vec3 &a, Vec3 b)
{
  a = a + b;
  return a;
}

constexpr Vec3 &operator-=(Vec3 &a, Vec3 b)
{
  a = a - b;
  return a;
}

constexpr Vec3 &operator*=(Vec3 &a, Vec3 b)
{
  a = a * b;
  return a;
}

constexpr Vec3 &operator/=(Vec3 &a, Vec3 b)
{
  a = a / b;
  return a;
}

struct alignas(16) Vec4
{
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;
  f32 w = 0;

  static constexpr Vec4 splat(f32 value)
  {
    return Vec4{value, value, value, value};
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

constexpr Vec4 vec4(Vec3 xyz, f32 w)
{
  return Vec4{xyz.x, xyz.y, xyz.z, w};
}

constexpr Vec4 vec4(f32 x, Vec3 yzw)
{
  return Vec4{x, yzw.x, yzw.y, yzw.z};
}

constexpr bool operator==(Vec4 a, Vec4 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4 a, Vec4 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4 operator+(Vec4 a, Vec4 b)
{
  return Vec4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr Vec4 operator+(Vec4 a, f32 b)
{
  return Vec4{a.x + b, a.y + b, a.z + b, a.w + b};
}

constexpr Vec4 operator+(f32 a, Vec4 b)
{
  return Vec4{a + b.x, a + b.y, a + b.z, a + b.w};
}

constexpr Vec4 operator-(Vec4 a)
{
  return Vec4{-a.x, -a.y, -a.z, -a.w};
}

constexpr Vec4 operator-(Vec4 a, Vec4 b)
{
  return Vec4{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr Vec4 operator-(Vec4 a, f32 b)
{
  return Vec4{a.x - b, a.y - b, a.z - b, a.w - b};
}

constexpr Vec4 operator-(f32 a, Vec4 b)
{
  return Vec4{a - b.x, a - b.y, a - b.z, a - b.w};
}

constexpr Vec4 operator*(Vec4 a, Vec4 b)
{
  return Vec4{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr Vec4 operator*(Vec4 a, f32 b)
{
  return Vec4{a.x * b, a.y * b, a.z * b, a.w * b};
}

constexpr Vec4 operator*(f32 a, Vec4 b)
{
  return Vec4{a * b.x, a * b.y, a * b.z, a * b.w};
}

constexpr Vec4 operator/(Vec4 a, Vec4 b)
{
  return Vec4{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr Vec4 operator/(Vec4 a, f32 b)
{
  return Vec4{a.x / b, a.y / b, a.z / b, a.w / b};
}

constexpr Vec4 operator/(f32 a, Vec4 b)
{
  return Vec4{a / b.x, a / b.y, a / b.z, a / b.w};
}

constexpr Vec4 &operator+=(Vec4 &a, Vec4 b)
{
  a = a + b;
  return a;
}

constexpr Vec4 &operator-=(Vec4 &a, Vec4 b)
{
  a = a - b;
  return a;
}

constexpr Vec4 &operator*=(Vec4 &a, Vec4 b)
{
  a = a * b;
  return a;
}

constexpr Vec4 &operator/=(Vec4 &a, Vec4 b)
{
  a = a / b;
  return a;
}

struct alignas(4) Vec4U8
{
  u8 x = 0;
  u8 y = 0;
  u8 z = 0;
  u8 w = 0;

  static constexpr Vec4U8 splat(u8 value)
  {
    return Vec4U8{value, value, value, value};
  }

  constexpr Vec4 norm() const
  {
    return Vec4{
        .x = x / 255.0F, .y = y / 255.0F, .z = z / 255.0F, .w = w / 255.0F};
  }

  constexpr u8 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u8 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec4U8 a, Vec4U8 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4U8 a, Vec4U8 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4U8 operator+(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x + b.x), static_cast<u8>(a.y + b.y),
                static_cast<u8>(a.z + b.z), static_cast<u8>(a.w + b.w)};
}

constexpr Vec4U8 operator-(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x - b.x), static_cast<u8>(a.y - b.y),
                static_cast<u8>(a.z - b.z), static_cast<u8>(a.w - b.w)};
}

constexpr Vec4U8 operator*(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x * b.x), static_cast<u8>(a.y * b.y),
                static_cast<u8>(a.z * b.z), static_cast<u8>(a.w * b.w)};
}

constexpr Vec4U8 operator/(Vec4U8 a, Vec4U8 b)
{
  return Vec4U8{static_cast<u8>(a.x / b.x), static_cast<u8>(a.y / b.y),
                static_cast<u8>(a.z / b.z), static_cast<u8>(a.w / b.w)};
}

constexpr Vec4U8 &operator+=(Vec4U8 &a, Vec4U8 b)
{
  a = a + b;
  return a;
}

constexpr Vec4U8 &operator-=(Vec4U8 &a, Vec4U8 b)
{
  a = a - b;
  return a;
}

constexpr Vec4U8 &operator*=(Vec4U8 &a, Vec4U8 b)
{
  a = a * b;
  return a;
}

constexpr Vec4U8 &operator/=(Vec4U8 &a, Vec4U8 b)
{
  a = a / b;
  return a;
}

constexpr Vec4U8 as_vec4u8(Vec4 a)
{
  return Vec4U8{(u8) a.x, (u8) a.y, (u8) a.z, (u8) a.w};
}

constexpr Vec4 as_vec4(Vec4U8 a)
{
  return Vec4{(f32) a.x, (f32) a.y, (f32) a.z, (f32) a.w};
}

struct alignas(8) Vec2I
{
  i32 x = 0;
  i32 y = 0;

  static constexpr Vec2I splat(i32 value)
  {
    return Vec2I{value, value};
  }

  constexpr i32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr i32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec2I a, Vec2I b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Vec2I a, Vec2I b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr Vec2I operator+(Vec2I a, Vec2I b)
{
  return Vec2I{a.x + b.x, a.y + b.y};
}

constexpr Vec2I operator-(Vec2I a)
{
  return Vec2I{-a.x, -a.y};
}

constexpr Vec2I operator-(Vec2I a, Vec2I b)
{
  return Vec2I{a.x - b.x, a.y - b.y};
}

constexpr Vec2I operator*(Vec2I a, Vec2I b)
{
  return Vec2I{a.x * b.x, a.y * b.y};
}

constexpr Vec2I operator*(i32 a, Vec2I b)
{
  return Vec2I{a * b.x, a * b.y};
}

constexpr Vec2I operator*(Vec2I a, i32 b)
{
  return Vec2I{a.x * b, a.y * b};
}

constexpr Vec2I operator/(Vec2I a, Vec2I b)
{
  return Vec2I{a.x / b.x, a.y / b.y};
}

constexpr Vec2I &operator+=(Vec2I &a, Vec2I b)
{
  a = a + b;
  return a;
}

constexpr Vec2I &operator-=(Vec2I &a, Vec2I b)
{
  a = a - b;
  return a;
}

constexpr Vec2I &operator*=(Vec2I &a, Vec2I b)
{
  a = a * b;
  return a;
}

constexpr Vec2I &operator/=(Vec2I &a, Vec2I b)
{
  a = a / b;
  return a;
}

struct Vec3I
{
  i32 x = 0;
  i32 y = 0;
  i32 z = 0;

  constexpr i32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr i32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec3I a, Vec3I b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(Vec3I a, Vec3I b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr Vec3I operator+(Vec3I a, Vec3I b)
{
  return Vec3I{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr Vec3I operator-(Vec3I a, Vec3I b)
{
  return Vec3I{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr Vec3I operator*(Vec3I a, Vec3I b)
{
  return Vec3I{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr Vec3I operator/(Vec3I a, Vec3I b)
{
  return Vec3I{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr Vec3I &operator+=(Vec3I &a, Vec3I b)
{
  a = a + b;
  return a;
}

constexpr Vec3I &operator-=(Vec3I &a, Vec3I b)
{
  a = a - b;
  return a;
}

constexpr Vec3I &operator*=(Vec3I &a, Vec3I b)
{
  a = a * b;
  return a;
}

constexpr Vec3I &operator/=(Vec3I &a, Vec3I b)
{
  a = a / b;
  return a;
}

struct alignas(16) Vec4I
{
  i32 x = 0;
  i32 y = 0;
  i32 z = 0;
  i32 w = 0;

  static constexpr Vec4I splat(i32 value)
  {
    return Vec4I{value, value, value, value};
  }

  constexpr i32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr i32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec4I a, Vec4I b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4I a, Vec4I b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4I operator+(Vec4I a, Vec4I b)
{
  return Vec4I{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr Vec4I operator-(Vec4I a, Vec4I b)
{
  return Vec4I{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr Vec4I operator*(Vec4I a, Vec4I b)
{
  return Vec4I{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr Vec4I operator/(Vec4I a, Vec4I b)
{
  return Vec4I{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr Vec4I &operator+=(Vec4I &a, Vec4I b)
{
  a = a + b;
  return a;
}

constexpr Vec4I &operator-=(Vec4I &a, Vec4I b)
{
  a = a - b;
  return a;
}

constexpr Vec4I &operator*=(Vec4I &a, Vec4I b)
{
  a = a * b;
  return a;
}

constexpr Vec4I &operator/=(Vec4I &a, Vec4I b)
{
  a = a / b;
  return a;
}

struct alignas(8) Vec2U
{
  u32 x = 0;
  u32 y = 0;

  static constexpr Vec2U splat(u32 value)
  {
    return Vec2U{value, value};
  }

  constexpr u32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr Vec2 as_vec2(Vec2U a)
{
  return Vec2{(f32) a.x, (f32) a.y};
}

constexpr bool operator==(Vec2U a, Vec2U b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Vec2U a, Vec2U b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr Vec2U operator+(Vec2U a, Vec2U b)
{
  return Vec2U{a.x + b.x, a.y + b.y};
}

constexpr Vec2U operator+(Vec2U a, u32 b)
{
  return Vec2U{a.x + b, a.y + b};
}

constexpr Vec2U operator+(u32 a, Vec2U b)
{
  return Vec2U{a + b.x, a + b.y};
}

constexpr Vec2U operator-(Vec2U a, Vec2U b)
{
  return Vec2U{a.x - b.x, a.y - b.y};
}

constexpr Vec2U operator-(Vec2U a, u32 b)
{
  return Vec2U{a.x - b, a.y - b};
}

constexpr Vec2U operator-(u32 a, Vec2U b)
{
  return Vec2U{a - b.x, a - b.y};
}

constexpr Vec2U operator*(Vec2U a, Vec2U b)
{
  return Vec2U{a.x * b.x, a.y * b.y};
}

constexpr Vec2U operator*(Vec2U a, u32 b)
{
  return Vec2U{a.x * b, a.y * b};
}

constexpr Vec2U operator*(u32 a, Vec2U b)
{
  return Vec2U{a * b.x, a * b.y};
}

constexpr Vec2U operator/(Vec2U a, Vec2U b)
{
  return Vec2U{a.x / b.x, a.y / b.y};
}

constexpr Vec2U operator/(Vec2U a, u32 b)
{
  return Vec2U{a.x / b, a.y / b};
}

constexpr Vec2U operator/(u32 a, Vec2U b)
{
  return Vec2U{a / b.x, a / b.y};
}

constexpr Vec2U &operator+=(Vec2U &a, Vec2U b)
{
  a = a + b;
  return a;
}

constexpr Vec2U &operator-=(Vec2U &a, Vec2U b)
{
  a = a - b;
  return a;
}

constexpr Vec2U &operator*=(Vec2U &a, Vec2U b)
{
  a = a * b;
  return a;
}

constexpr Vec2U &operator/=(Vec2U &a, Vec2U b)
{
  a = a / b;
  return a;
}

struct Vec3U
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;

  static constexpr Vec3U splat(u32 value)
  {
    return Vec3U{value, value, value};
  }

  constexpr u32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec3U a, Vec3U b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(Vec3U a, Vec3U b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr Vec3U operator+(Vec3U a, Vec3U b)
{
  return Vec3U{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr Vec3U operator-(Vec3U a, Vec3U b)
{
  return Vec3U{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr Vec3U operator*(Vec3U a, Vec3U b)
{
  return Vec3U{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr Vec3U operator/(Vec3U a, Vec3U b)
{
  return Vec3U{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr Vec3U &operator+=(Vec3U &a, Vec3U b)
{
  a = a + b;
  return a;
}

constexpr Vec3U &operator-=(Vec3U &a, Vec3U b)
{
  a = a - b;
  return a;
}

constexpr Vec3U &operator*=(Vec3U &a, Vec3U b)
{
  a = a * b;
  return a;
}

constexpr Vec3U &operator/=(Vec3U &a, Vec3U b)
{
  a = a / b;
  return a;
}

struct alignas(16) Vec4U
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
  u32 w = 0;

  static constexpr Vec4U splat(u32 value)
  {
    return Vec4U{value, value, value, value};
  }

  constexpr u32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec4U a, Vec4U b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4U a, Vec4U b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4U operator+(Vec4U a, Vec4U b)
{
  return Vec4U{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr Vec4U operator-(Vec4U a, Vec4U b)
{
  return Vec4U{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr Vec4U operator*(Vec4U a, Vec4U b)
{
  return Vec4U{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr Vec4U operator/(Vec4U a, Vec4U b)
{
  return Vec4U{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr Vec4U &operator+=(Vec4U &a, Vec4U b)
{
  a = a + b;
  return a;
}

constexpr Vec4U &operator-=(Vec4U &a, Vec4U b)
{
  a = a - b;
  return a;
}

constexpr Vec4U &operator*=(Vec4U &a, Vec4U b)
{
  a = a * b;
  return a;
}

constexpr Vec4U &operator/=(Vec4U &a, Vec4U b)
{
  a = a / b;
  return a;
}

constexpr f32 dot(Vec2 a, Vec2 b)
{
  return a.x * b.x + a.y * b.y;
}

constexpr i32 dot(Vec2I a, Vec2I b)
{
  return a.x * b.x + a.y * b.y;
}

constexpr f32 dot(Vec3 a, Vec3 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr i32 dot(Vec3I a, Vec3I b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr f32 dot(Vec4 a, Vec4 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

constexpr i32 dot(Vec4I a, Vec4I b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

constexpr f32 cross(Vec2 a, Vec2 b)
{
  return a.x * b.y - b.x * a.y;
}

constexpr i32 cross(Vec2I a, Vec2I b)
{
  return a.x * b.y - b.x * a.y;
}

constexpr Vec3 cross(Vec3 a, Vec3 b)
{
  return Vec3{a.y * b.z - a.z * b.y, -(a.x * b.z - a.z * b.x),
              a.x * b.y - a.y * b.x};
}

constexpr Vec3I cross(Vec3I a, Vec3I b)
{
  return Vec3I{a.y * b.z - a.z * b.y, -(a.x * b.z - a.z * b.x),
               a.x * b.y - a.y * b.x};
}

constexpr f32 inverse_sqrt(f32 num)
{
  // (enable only on IEEE 754)
  static_assert(std::numeric_limits<f32>::is_iec559);
  f32 const y = std::bit_cast<f32>(0x5F3759DF - (std::bit_cast<u32>(num) >> 1));
  return y * (1.5F - (num * 0.5F * y * y));
}

constexpr Vec2 normalize(Vec2 a)
{
  return a * inverse_sqrt(dot(a, a));
}

constexpr Vec3 normalize(Vec3 a)
{
  return a * inverse_sqrt(dot(a, a));
}

constexpr Vec4 normalize(Vec4 a)
{
  return a * inverse_sqrt(dot(a, a));
}

struct Mat2
{
  Vec2 rows[2] = {};

  static constexpr Mat2 splat(f32 value)
  {
    return Mat2{.rows = {{value, value}, {value, value}}};
  }

  static constexpr Mat2 diagonal(f32 value)
  {
    return Mat2{.rows = {{value, 0}, {0, value}}};
  }

  static constexpr Mat2 identity()
  {
    return diagonal(1);
  }

  constexpr Vec2 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec2 const &operator[](usize index) const
  {
    return rows[index];
  }

  constexpr Vec2 x() const
  {
    return Vec2{rows[0].x, rows[1].x};
  }

  constexpr Vec2 y() const
  {
    return Vec2{rows[0].y, rows[1].y};
  }
};

constexpr bool operator==(Mat2 const &a, Mat2 const &b)
{
  return a[0] == b[0] && a[1] == b[1];
}

constexpr bool operator!=(Mat2 const &a, Mat2 const &b)
{
  return a[0] != b[0] || a[1] != b[1];
}

constexpr Mat2 operator+(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {a[0] + b[0], a[1] + b[1]}};
}

constexpr Mat2 operator-(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {a[0] - b[0], a[1] - b[1]}};
}

constexpr Vec2 operator*(Mat2 const &a, Vec2 const &b)
{
  return Vec2{dot(a[0], b), dot(a[1], b)};
}

constexpr Mat2 operator*(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {{dot(a[0], b.x()), dot(a[0], b.y())},
                       {dot(a[1], b.x()), dot(a[1], b.y())}}};
}

constexpr Mat2 operator/(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {a[0] / b[0], a[1] / b[1]}};
}

constexpr Mat2 &operator+=(Mat2 &a, Mat2 const &b)
{
  a = a + b;
  return a;
}

constexpr Mat2 &operator-=(Mat2 &a, Mat2 const &b)
{
  a = a - b;
  return a;
}

constexpr Mat2 &operator*=(Mat2 &a, Mat2 const &b)
{
  a = a * b;
  return a;
}

constexpr Mat2 &operator/=(Mat2 &a, Mat2 const &b)
{
  a = a / b;
  return a;
}

struct Mat3
{
  Vec3 rows[3] = {};

  static constexpr Mat3 splat(f32 value)
  {
    return Mat3{.rows = {{value, value, value},
                         {value, value, value},
                         {value, value, value}}};
  }

  static constexpr Mat3 diagonal(f32 value)
  {
    return Mat3{.rows = {{value, 0, 0}, {0, value, 0}, {0, 0, value}}};
  }

  static constexpr Mat3 identity()
  {
    return diagonal(1);
  }

  constexpr Vec3 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec3 const &operator[](usize index) const
  {
    return rows[index];
  }

  constexpr Vec3 x() const
  {
    return Vec3{rows[0].x, rows[1].x, rows[2].x};
  }

  constexpr Vec3 y() const
  {
    return Vec3{rows[0].y, rows[1].y, rows[2].y};
  }

  constexpr Vec3 z() const
  {
    return Vec3{rows[0].z, rows[1].z, rows[2].z};
  }
};

constexpr bool operator==(Mat3 const &a, Mat3 const &b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

constexpr bool operator!=(Mat3 const &a, Mat3 const &b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

constexpr Mat3 operator+(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2]}};
}

constexpr Mat3 operator-(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2]}};
}

constexpr Vec3 operator*(Mat3 const &a, Vec3 const &b)
{
  return Vec3{dot(a[0], b), dot(a[1], b), dot(a[2], b)};
}

constexpr Mat3 operator*(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
                  {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z())},
              }};
}

constexpr Mat3 operator/(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2]}};
}

constexpr Mat3 &operator+=(Mat3 &a, Mat3 const &b)
{
  a = a + b;
  return a;
}

constexpr Mat3 &operator-=(Mat3 &a, Mat3 const &b)
{
  a = a - b;
  return a;
}

constexpr Mat3 &operator*=(Mat3 &a, Mat3 const &b)
{
  a = a * b;
  return a;
}

constexpr Mat3 &operator/=(Mat3 &a, Mat3 const &b)
{
  a = a / b;
  return a;
}

struct Mat3Affine
{
  static constexpr Vec3 trailing_row = Vec3{0, 0, 1};
  Vec3                  rows[2]      = {};

  constexpr Vec3 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec3 const &operator[](usize index) const
  {
    return rows[index];
  }

  constexpr operator Mat3() const
  {
    return Mat3{.rows = {rows[0], rows[1], {0, 0, 1}}};
  }

  static constexpr Mat3Affine identity()
  {
    return Mat3Affine{.rows = {{1, 0, 0}, {0, 1, 0}}};
  }

  constexpr Vec3 x() const
  {
    return Vec3{rows[0].x, rows[1].x, 0};
  }

  constexpr Vec3 y() const
  {
    return Vec3{rows[0].y, rows[1].y, 0};
  }

  constexpr Vec3 z() const
  {
    return Vec3{rows[0].z, rows[1].z, 1};
  }
};

constexpr bool operator==(Mat3Affine const &a, Mat3Affine const &b)
{
  return a[0] == b[0] && a[1] == b[1];
}

constexpr bool operator!=(Mat3Affine const &a, Mat3Affine const &b)
{
  return a[0] != b[0] || a[1] != b[1];
}

constexpr Mat3Affine operator+(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{.rows = {a[0] + b[0], a[1] + b[1]}};
}

constexpr Mat3Affine operator-(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{.rows = {a[0] - b[0], a[1] - b[1]}};
}

constexpr Vec3 operator*(Mat3Affine const &a, Vec3 const &b)
{
  return Vec3{dot(a[0], b), dot(a[1], b), dot(Mat3Affine::trailing_row, b)};
}

constexpr Mat3 operator*(Mat3Affine const &a, Mat3 const &b)
{
  return Mat3{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
                  {dot(Mat3Affine::trailing_row, b.x()),
                   dot(Mat3Affine::trailing_row, b.y()),
                   dot(Mat3Affine::trailing_row, b.z())},
              }};
}

constexpr Mat3 operator*(Mat3 const &a, Mat3Affine const &b)
{
  return Mat3{.rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
                       {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
                       {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z())}}};
}

constexpr Mat3Affine operator*(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{
      .rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
               {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())}}};
}

constexpr Mat3Affine operator/(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{.rows = {a[0] / b[0], a[1] / b[1]}};
}

constexpr Mat3Affine &operator+=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a + b;
  return a;
}

constexpr Mat3Affine &operator-=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a - b;
  return a;
}

constexpr Mat3Affine &operator*=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a * b;
  return a;
}

constexpr Mat3Affine &operator/=(Mat3Affine &a, Mat3Affine const &b)
{
  a = a / b;
  return a;
}

struct Mat4
{
  Vec4 rows[4] = {};

  static constexpr Mat4 splat(f32 value)
  {
    return Mat4{.rows = {{value, value, value, value},
                         {value, value, value, value},
                         {value, value, value, value},
                         {value, value, value, value}}};
  }

  static constexpr Mat4 diagonal(f32 value)
  {
    return Mat4{.rows = {{value, 0, 0, 0},
                         {0, value, 0, 0},
                         {0, 0, value, 0},
                         {0, 0, 0, value}}};
  }

  static constexpr Mat4 identity()
  {
    return diagonal(1);
  }

  constexpr Vec4 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec4 const &operator[](usize index) const
  {
    return rows[index];
  }

  constexpr Vec4 x() const
  {
    return Vec4{rows[0].x, rows[1].x, rows[2].x, rows[3].x};
  }

  constexpr Vec4 y() const
  {
    return Vec4{rows[0].y, rows[1].y, rows[2].y, rows[3].y};
  }

  constexpr Vec4 z() const
  {
    return Vec4{rows[0].z, rows[1].z, rows[2].z, rows[3].z};
  }

  constexpr Vec4 w() const
  {
    return Vec4{rows[0].w, rows[1].w, rows[2].w, rows[3].w};
  }
};

constexpr bool operator==(Mat4 const &a, Mat4 const &b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

constexpr bool operator!=(Mat4 const &a, Mat4 const &b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

constexpr Mat4 operator+(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]}};
}

constexpr Mat4 operator-(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]}};
}

constexpr Vec4 operator*(Mat4 const &a, Vec4 const &b)
{
  return Vec4{dot(a[0], b), dot(a[1], b), dot(a[2], b), dot(a[3], b)};
}

constexpr Mat4 operator*(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()),
                        dot(a[0], b.w())},
                       {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()),
                        dot(a[1], b.w())},
                       {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()),
                        dot(a[2], b.w())},
                       {dot(a[3], b.x()), dot(a[3], b.y()), dot(a[3], b.z()),
                        dot(a[3], b.w())}}};
}

constexpr Mat4 operator/(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2], a[3] / b[3]}};
}

constexpr Mat4 &operator+=(Mat4 &a, Mat4 const &b)
{
  a = a + b;
  return a;
}

constexpr Mat4 &operator-=(Mat4 &a, Mat4 const &b)
{
  a = a - b;
  return a;
}

constexpr Mat4 &operator*=(Mat4 &a, Mat4 const &b)
{
  a = a * b;
  return a;
}

constexpr Mat4 &operator/=(Mat4 &a, Mat4 const &b)
{
  a = a / b;
  return a;
}

struct Mat4Affine
{
  static constexpr Vec4 trailing_row = Vec4{0, 0, 0, 1};
  Vec4                  rows[3]      = {};

  constexpr Vec4 &operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec4 const &operator[](usize index) const
  {
    return rows[index];
  }

  constexpr operator Mat4() const
  {
    return Mat4{.rows = {rows[0], rows[1], rows[2], {0, 0, 0, 1}}};
  }

  static constexpr Mat4Affine identity()
  {
    return Mat4Affine{.rows = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}}};
  }

  constexpr Vec4 x() const
  {
    return Vec4{rows[0].x, rows[1].x, rows[2].x, 0};
  }

  constexpr Vec4 y() const
  {
    return Vec4{rows[0].y, rows[1].y, rows[2].y, 0};
  }

  constexpr Vec4 z() const
  {
    return Vec4{rows[0].z, rows[1].z, rows[2].z, 0};
  }

  constexpr Vec4 w() const
  {
    return Vec4{rows[0].w, rows[1].w, rows[2].w, 1};
  }
};

constexpr bool operator==(Mat4Affine const &a, Mat4Affine const &b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

constexpr bool operator!=(Mat4Affine const &a, Mat4Affine const &b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

constexpr Mat4Affine operator+(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2]}};
}

constexpr Mat4Affine operator-(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2]}};
}

constexpr Vec4 operator*(Mat4Affine const &a, Vec4 const &b)
{
  return Vec4{dot(a[0], b), dot(a[1], b), dot(a[2], b),
              dot(Mat4Affine::trailing_row, b)};
}

constexpr Mat4 operator*(Mat4Affine const &a, Mat4 const &b)
{
  return Mat4{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()),
                   dot(a[0], b.w())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()),
                   dot(a[1], b.w())},
                  {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()),
                   dot(a[2], b.w())},
                  {dot(Mat4Affine::trailing_row, b.x()),
                   dot(Mat4Affine::trailing_row, b.y()),
                   dot(Mat4Affine::trailing_row, b.z()),
                   dot(Mat4Affine::trailing_row, b.w())},
              }};
}

constexpr Mat4 operator*(Mat4 const &a, Mat4Affine const &b)
{
  return Mat4{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()),
                   dot(a[0], b.w())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()),
                   dot(a[1], b.w())},
                  {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()),
                   dot(a[2], b.w())},
                  {dot(a[3], b.x()), dot(a[3], b.y()), dot(a[3], b.z()),
                   dot(a[3], b.w())},
              }};
}

constexpr Mat4Affine operator*(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {{dot(a[0], b.x()), dot(a[0], b.y()),
                              dot(a[0], b.z()), dot(a[0], b.w())},
                             {dot(a[1], b.x()), dot(a[1], b.y()),
                              dot(a[1], b.z()), dot(a[1], b.w())},
                             {dot(a[2], b.x()), dot(a[2], b.y()),
                              dot(a[2], b.z()), dot(a[2], b.w())}}};
}

constexpr Mat4Affine operator/(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2]}};
}

constexpr Mat4Affine &operator+=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a + b;
  return a;
}

constexpr Mat4Affine &operator-=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a - b;
  return a;
}

constexpr Mat4Affine &operator*=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a * b;
  return a;
}

constexpr Mat4Affine &operator/=(Mat4Affine &a, Mat4Affine const &b)
{
  a = a / b;
  return a;
}

inline f32 length(Vec2 a)
{
  return sqrtf(dot(a, a));
}

inline f32 length(Vec3 a)
{
  return std::sqrt(dot(a, a));
}

inline f32 length(Vec4 a)
{
  return std::sqrt(dot(a, a));
}

constexpr Vec2U mip_down(Vec2U a, u32 level)
{
  return Vec2U{mip_down(a.x, level), mip_down(a.y, level)};
}

constexpr Vec3U mip_down(Vec3U a, u32 level)
{
  return Vec3U{mip_down(a.x, level), mip_down(a.y, level),
               mip_down(a.z, level)};
}

constexpr Vec4U mip_down(Vec4U a, u32 level)
{
  return Vec4U{mip_down(a.x, level), mip_down(a.y, level), mip_down(a.z, level),
               mip_down(a.w, level)};
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
  return Mat3Affine{.rows = {{cos(radians), -sin(radians), 0},
                             {sin(radians), cos(radians), 0}}};
}

inline Mat4Affine rotate3d_x(f32 radians)
{
  return Mat4Affine{.rows = {{1, 0, 0, 0},
                             {0, cos(radians), -sin(radians), 0},
                             {0, sin(radians), cos(radians), 0}}};
}

inline Mat4Affine rotate3d_y(f32 radians)
{
  return Mat4Affine{.rows = {{cos(radians), 0, sin(radians), 0},
                             {0, 1, 0, 0},
                             {-sin(radians), 0, cos(radians), 0}}};
}

inline Mat4Affine rotate3d_z(f32 radians)
{
  return Mat4Affine{.rows = {{cos(radians), -sin(radians), 0, 0},
                             {sin(radians), cos(radians), 0, 0},
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

inline Vec2 sin(Vec2 v)
{
  return Vec2{sin(v.x), sin(v.y)};
}

inline Vec3 sin(Vec3 v)
{
  return Vec3{sin(v.x), sin(v.y), sin(v.z)};
}

inline Vec4 sin(Vec4 v)
{
  return Vec4{sin(v.x), sin(v.y), sin(v.z), sin(v.w)};
}

inline Vec2 cos(Vec2 v)
{
  return Vec2{cos(v.x), cos(v.y)};
}

inline Vec3 cos(Vec3 v)
{
  return Vec3{cos(v.x), cos(v.y), cos(v.z)};
}

inline Vec4 cos(Vec4 v)
{
  return Vec4{cos(v.x), cos(v.y), cos(v.z), cos(v.w)};
}

inline Vec2 tan(Vec2 v)
{
  return Vec2{tan(v.x), tan(v.y)};
}

inline Vec3 tan(Vec3 v)
{
  return Vec3{tan(v.x), tan(v.y), tan(v.z)};
}

inline Vec4 tan(Vec4 v)
{
  return Vec4{tan(v.x), tan(v.y), tan(v.z), tan(v.w)};
}

inline Vec2 exp(Vec2 v)
{
  return Vec2{exp(v.x), exp(v.y)};
}

inline Vec3 exp(Vec3 v)
{
  return Vec3{exp(v.x), exp(v.y), exp(v.z)};
}

inline Vec4 exp(Vec4 v)
{
  return Vec4{exp(v.x), exp(v.y), exp(v.z), exp(v.w)};
}

inline Vec2 exp2(Vec2 v)
{
  return Vec2{exp2(v.x), exp2(v.y)};
}

inline Vec3 exp2(Vec3 v)
{
  return Vec3{exp2(v.x), exp2(v.y), exp2(v.z)};
}

inline Vec4 exp2(Vec4 v)
{
  return Vec4{exp2(v.x), exp2(v.y), exp2(v.z), exp2(v.w)};
}

inline Vec2 log(Vec2 v)
{
  return Vec2{log(v.x), log(v.y)};
}

inline Vec3 log(Vec3 v)
{
  return Vec3{log(v.x), log(v.y), log(v.z)};
}

inline Vec4 log(Vec4 v)
{
  return Vec4{log(v.x), log(v.y), log(v.z), log(v.w)};
}

inline Vec2 floor(Vec2 v)
{
  return Vec2{floor(v.x), floor(v.y)};
}

inline Vec3 floor(Vec3 v)
{
  return Vec3{floor(v.x), floor(v.y), floor(v.z)};
}

inline Vec4 floor(Vec4 v)
{
  return Vec4{floor(v.x), floor(v.y), floor(v.z), floor(v.w)};
}

inline Vec2 rotor(f32 a)
{
  return Vec2{cos(a), sin(a)};
}

constexpr Vec2 ALIGNMENT_CENTER{0, 0};
constexpr Vec2 ALIGNMENT_TOP_LEFT{-1, -1};
constexpr Vec2 ALIGNMENT_TOP_RIGHT{1, -1};
constexpr Vec2 ALIGNMENT_BOTTOM_LEFT{-1, 1};
constexpr Vec2 ALIGNMENT_BOTTOM_RIGHT{1, 1};

constexpr Vec4 opacity(f32 v)
{
  return Vec4{1, 1, 1, v};
}

constexpr Vec4 opacity_premultiplied(f32 v)
{
  return Vec4::splat(v);
}

struct Rect
{
  Vec2 offset = {};
  Vec2 extent = {};

  static constexpr Rect from_center(Vec2 center, Vec2 extent)
  {
    return Rect{.offset = center - extent * 0.5F, .extent = extent};
  }

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

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0;
  }
};

constexpr bool operator==(Rect const &a, Rect const &b)
{
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(Rect const &a, Rect const &b)
{
  return a.offset != b.offset || a.extent != b.extent;
}

struct CRect
{
  Vec2 center = {};
  Vec2 extent = {};

  static constexpr CRect from_offset(Vec2 offset, Vec2 extent)
  {
    return CRect{.center = offset + extent * 0.5F, .extent = extent};
  }

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

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0;
  }
};

constexpr CRect Rect::centered() const
{
  return CRect::from_offset(offset, extent);
}

constexpr Rect CRect::offseted() const
{
  return Rect::from_center(center, extent);
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

  static constexpr Box from_center(Vec3 center, Vec3 extent)
  {
    return Box{.offset = center - extent * 0.5F, .extent = extent};
  }

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

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0 && extent.z != 0;
  }
};

constexpr bool operator==(Box const &a, Box const &b)
{
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(Box const &a, Box const &b)
{
  return a.offset != b.offset || a.extent != b.extent;
}

struct CBox
{
  Vec3 center = {};
  Vec3 extent = {};

  static constexpr CBox from_offset(Vec3 offset, Vec3 extent)
  {
    return CBox{.center = offset + extent * 0.5F, .extent = extent};
  }

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

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0 && extent.z != 0;
  }
};

constexpr CBox Box::centered() const
{
  return CBox::from_offset(offset, extent);
}

constexpr Box CBox::offseted() const
{
  return Box::from_center(center, extent);
}

constexpr bool operator==(CBox const &a, CBox const &b)
{
  return a.center == b.center && a.extent == b.extent;
}

constexpr bool operator!=(CBox const &a, CBox const &b)
{
  return a.center != b.center || a.extent != b.extent;
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

constexpr bool contains(Rect const &rect, Vec2 point)
{
  return contains_point(rect.begin(), rect.end(), point);
}

constexpr bool contains(CRect const &rect, Vec2 point)
{
  return contains_point(rect.begin(), rect.end(), point);
}

constexpr bool overlaps(Rect const &a, Rect const &b)
{
  return ash::overlaps(a.begin(), a.end(), b.begin(), b.end());
}

constexpr bool overlaps(CRect const &a, CRect const &b)
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

constexpr CRect intersect(CRect const &a, CRect const &b)
{
  Vec2 begin = b.begin();
  Vec2 end   = b.end();
  intersect(a.begin(), a.end(), begin, end);
  return CRect::from_offset(begin, end - begin);
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
                            Span<Box const> aabb, BitSpan<u64> is_visible)
{
  for (u32 i = 0; i < aabb.size32(); i++)
  {
    is_visible.set(i, !is_outside_frustum(mvp * global_transform[i],
                                          aabb[i].offset, aabb[i].extent));
  }
}

}        // namespace ash
