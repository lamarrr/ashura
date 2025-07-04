/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/tuple.h"
#include "ashura/std/types.h"
#include <bit>
#include <limits>
#include <math.h>

namespace ash
{

template <typename T>
constexpr T pow2(T x)
{
  return x * x;
}

template <typename T>
constexpr T pow3(T x)
{
  return x * x * x;
}

template <typename T>
constexpr T pow4(T x)
{
  return x * x * x * x;
}

template <typename T>
constexpr T pow5(T x)
{
  return x * x * x * x * x;
}

template <typename T>
constexpr T pow6(T x)
{
  return x * x * x * x * x * x;
}

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

template <typename T>
constexpr T abs_diff(T a, T b)
{
  if (a < b)
  {
    return b - a;
  }

  return a - b;
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

inline f32 sqrt(f32 x)
{
  return std::sqrt(x);
}

inline f64 sqrt(f64 x)
{
  return std::sqrt(x);
}

constexpr f32 invsqrt(f32 x)
{
  // (enable only on IEEE 754)
  static_assert(std::numeric_limits<f32>::is_iec559);
  f32 const y = std::bit_cast<f32>(0x5F37'59DF - (std::bit_cast<u32>(x) >> 1));
  return y * (1.5F - (x * 0.5F * y * y));
}

/// @brief Calculate log base 2 of an unsigned integer. Undefined behaviour if
/// value is 0
constexpr u32 ulog2(u8 value)
{
  return 7 - std::countl_zero(value);
}

/// @brief Calculate log base 2 of an unsigned integer. Undefined behaviour if
/// value is 0
constexpr u32 ulog2(u16 value)
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
constexpr u32 ulog2(u64 value)
{
  return 63 - std::countl_zero(value);
}

constexpr u32 mip_size(u32 a, u32 level)
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
constexpr T lerp(T const & low, T const & high, T const & t)
{
  return low - t * low + t * high;
}

/// @brief logarithmically interpolate between points `low` and `high` given
/// interpolator `t`
template <typename T>
inline T log_interp(T const & low, T const & high, T const & t)
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
inline T damplerp(T const & low, T const & high, T const & dt,
                  T const & half_life)
{
  return lerp(low, high, 1 - exp2(-half_life * dt));
}

/// find interpolator t, given points a and b, and interpolated value v
template <typename T>
constexpr T unlerp(T const & low, T const & high, T const & v)
{
  return (low == high) ? low : ((v - low) / (high - low));
}

template <typename T>
constexpr T relerp(T const & in_low, T const & in_high, T const & out_low,
                   T const & out_high, T const & v)
{
  return lerp(out_low, out_high, unlerp(in_low, in_high, v));
}

// SEE: https://www.youtube.com/watch?v=jvPPXbo87ds
template <typename T>
constexpr T ease_in(T const & t)
{
  return t * t;
}

template <typename T>
constexpr T ease_out(T const & t)
{
  return 1 - (1 - t) * (1 - t);
}

template <typename T>
constexpr T ease_in_out(T const & t)
{
  return lerp(ease_in(t), ease_out(t), t);
}

template <typename T>
constexpr T bezier(T const & p0, T const & p1, T const & p2, T const & t)
{
  return (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;
}

template <typename T>
constexpr T cubic_bezier(T const & p0, T const & p1, T const & p2, T const & p3,
                         T const & t)
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
constexpr T catmull_rom(T const & p0, T const & p1, T const & p2, T const & p3,
                        T const & t)
{
  return 0.5F *
         ((2 * p1) + (-p0 + p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
          (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t);
}

/// @brief Elastic Easing
/// @param amplitude strength of the elastic effect (default = 1.0)
/// @param period length of the oscillation (default = 0.3)
/// @note Based on Robert Penner's elastic easing
/// (http://robertpenner.com/easing/)
inline f32 elastic(f32 amplitude, f32 period, f32 t)
{
  constexpr f32 TWO_PI = 2.0F * PI;
  f32 const     s      = (period * (1 / TWO_PI)) * std::asin(1 / amplitude);
  f32 const     factor = amplitude * std::pow(2.0F, -10.0F * t) *
                       std::sin((t - s) * (TWO_PI / period)) +
                     1.0F;
  return factor;
}

/// @brief EaseOut Bounce
/// @param strength strength of the bounce effect (default = 1.0)
/// @note Based on Robert Penner's easeOutBounce
/// (http://robertpenner.com/easing/)
constexpr f32 bounce(f32 strength, f32 t)
{
  // Inverse the time to create an ease-out effect
  t = 1.0F - t;

  if (t < (1.0F / 2.75F))
  {
    return 1.0F - (7.5625F * t * t * strength);
  }
  else if (t < (2.0F / 2.75F))
  {
    t -= 1.5F / 2.75F;
    return 1.0F - (7.5625F * t * t * strength + 0.75F);
  }
  else if (t < (2.5F / 2.75F))
  {
    t -= 2.25F / 2.75F;
    return 1.0F - (7.5625F * t * t * strength + 0.9375F);
  }
  else
  {
    t -= 2.625F / 2.75F;
    return 1.0F - (7.5625F * t * t * strength + 0.984375F);
  }
}

/// @brief Spring-based Elastic Easing based on simple harmonic motion with
/// damping
///
/// The default behavior is a tight spring effect, tune the parameters to give
/// a desired effect.
/// @param mass: Oscillator mass (default: 1.0)
/// @param stiffness: Spring constant (default: 20.0)
/// @param damping: Damping coefficient (default: 10.0F)
///
/// @note https://www.ryanjuckett.com/damped-springs/
///
inline f32 spring(f32 mass, f32 stiffness, f32 damping, f32 t)
{
  // Calculate critical damping factors
  f32 const omega0           = std::sqrt(stiffness / mass);
  f32 const critical_damping = 2.0F * std::sqrt(mass * stiffness);
  f32 const damping_ratio    = damping / critical_damping;

  // Underdamped
  if (damping_ratio < 1.0F)
  {
    f32 const omega_d =
      omega0 * std::sqrt(1.0F - damping_ratio * damping_ratio);
    return 1.0F -
           std::exp(-damping_ratio * omega0 * t) *
             (std::cos(omega_d * t) +
              (damping_ratio * omega0 / omega_d) * std::sin(omega_d * t));
  }

  // Overdamped or critically damped
  f32 const alpha = -damping_ratio * omega0;
  f32 const beta  = omega0 * std::sqrt(damping_ratio * damping_ratio - 1.0F);
  return 1.0F - (std::exp(alpha * t) *
                 (std::cosh(beta * t) + (alpha / beta) * std::sinh(beta * t)));
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
inline T grid_snap(T const & a, T const & unit)
{
  return floor((a + unit * 0.5F) / unit) * unit;
}

template <typename T>
constexpr T norm_to_axis(T const & norm)
{
  return norm - 0.5F;
}

template <typename T>
constexpr T axis_to_norm(T const & axis)
{
  return axis + 0.5F;
}

/// @param space available space to align to
/// @param item extent of the item to align
/// @param alignment the alignment to align to [-0.5, +0.5]
/// @return returns the aligned position relative to the space's center
template <typename T>
constexpr T space_align(T const & space, T const & item, T const & alignment)
{
  T const half_space = 0.5F * (space - item);
  return lerp(-half_space, half_space, axis_to_norm(alignment));
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
typedef struct RectI RectI;
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

ASH_BIT_ENUM_OPS(Axes)

struct Vec2
{
  static Vec2 const ZERO;
  static Vec2 const ONE;

  f32 x = 0;
  f32 y = 0;

  static constexpr Vec2 splat(f32 value)
  {
    return Vec2{value, value};
  }

  constexpr f32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const & operator[](usize i) const
  {
    return (&x)[i];
  }
};

inline constexpr Vec2 Vec2::ZERO{0, 0};

inline constexpr Vec2 Vec2::ONE{1, 1};

constexpr bool operator==(Vec2 a, Vec2 b)
{
  return a.x == b.x & a.y == b.y;
}

constexpr bool operator!=(Vec2 a, Vec2 b)
{
  return a.x != b.x | a.y != b.y;
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

constexpr Vec2 & operator+=(Vec2 & a, Vec2 b)
{
  a = a + b;
  return a;
}

constexpr Vec2 & operator-=(Vec2 & a, Vec2 b)
{
  a = a - b;
  return a;
}

constexpr Vec2 & operator*=(Vec2 & a, Vec2 b)
{
  a = a * b;
  return a;
}

constexpr Vec2 & operator/=(Vec2 & a, Vec2 b)
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

  constexpr f32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const & operator[](usize i) const
  {
    return (&x)[i];
  }

  constexpr Vec2 xy() const
  {
    return {x, y};
  }

  constexpr Vec2 yz() const
  {
    return {y, z};
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
  return a.x == b.x & a.y == b.y & a.z == b.z;
}

constexpr bool operator!=(Vec3 a, Vec3 b)
{
  return a.x != b.x | a.y != b.y | a.z != b.z;
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

constexpr Vec3 & operator+=(Vec3 & a, Vec3 b)
{
  a = a + b;
  return a;
}

constexpr Vec3 & operator-=(Vec3 & a, Vec3 b)
{
  a = a - b;
  return a;
}

constexpr Vec3 & operator*=(Vec3 & a, Vec3 b)
{
  a = a * b;
  return a;
}

constexpr Vec3 & operator/=(Vec3 & a, Vec3 b)
{
  a = a / b;
  return a;
}

struct Vec4
{
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;
  f32 w = 0;

  static constexpr Vec4 splat(f32 value)
  {
    return Vec4{value, value, value, value};
  }

  constexpr f32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const & operator[](usize i) const
  {
    return (&x)[i];
  }

  constexpr Vec2 xy() const
  {
    return {x, y};
  }

  constexpr Vec2 xz() const
  {
    return {x, z};
  }

  constexpr Vec2 yz() const
  {
    return {y, z};
  }

  constexpr Vec2 yw() const
  {
    return {y, w};
  }

  constexpr Vec3 xyz() const
  {
    return {x, y, z};
  }

  constexpr Vec3 yzw() const
  {
    return {y, z, w};
  }

  constexpr Vec2 zw() const
  {
    return {z, w};
  }

  constexpr Vec4 with_x(f32 x) const
  {
    return Vec4{.x = x, .y = y, .z = z, .w = w};
  }

  constexpr Vec4 with_y(f32 y) const
  {
    return Vec4{.x = x, .y = y, .z = z, .w = w};
  }

  constexpr Vec4 with_z(f32 w) const
  {
    return Vec4{.x = x, .y = y, .z = z, .w = w};
  }

  constexpr Vec4 with_w(f32 w) const
  {
    return Vec4{.x = x, .y = y, .z = z, .w = w};
  }
};

constexpr Vec4 vec4(Vec3 xyz, f32 w)
{
  return Vec4{xyz.x, xyz.y, xyz.z, w};
}

constexpr Vec4 vec4(Vec2 xy, f32 z, f32 w)
{
  return Vec4{xy.x, xy.y, z, w};
}

constexpr Vec4 vec4(f32 x, Vec3 yzw)
{
  return Vec4{x, yzw.x, yzw.y, yzw.z};
}

constexpr bool operator==(Vec4 a, Vec4 b)
{
  return a.x == b.x & a.y == b.y & a.z == b.z & a.w == b.w;
}

constexpr bool operator!=(Vec4 a, Vec4 b)
{
  return a.x != b.x | a.y != b.y | a.z != b.z | a.w != b.w;
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

constexpr Vec4 & operator+=(Vec4 & a, Vec4 b)
{
  a = a + b;
  return a;
}

constexpr Vec4 & operator-=(Vec4 & a, Vec4 b)
{
  a = a - b;
  return a;
}

constexpr Vec4 & operator*=(Vec4 & a, Vec4 b)
{
  a = a * b;
  return a;
}

constexpr Vec4 & operator/=(Vec4 & a, Vec4 b)
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

  constexpr u8 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u8 const & operator[](usize i) const
  {
    return (&x)[i];
  }

  constexpr Vec4U8 with_x(u8 x) const
  {
    return Vec4U8{.x = x, .y = y, .z = z, .w = w};
  }

  constexpr Vec4U8 with_y(u8 y) const
  {
    return Vec4U8{.x = x, .y = y, .z = z, .w = w};
  }

  constexpr Vec4U8 with_z(u8 w) const
  {
    return Vec4U8{.x = x, .y = y, .z = z, .w = w};
  }

  constexpr Vec4U8 with_w(u8 w) const
  {
    return Vec4U8{.x = x, .y = y, .z = z, .w = w};
  }
};

constexpr bool operator==(Vec4U8 a, Vec4U8 b)
{
  return a.x == b.x & a.y == b.y & a.z == b.z & a.w == b.w;
}

constexpr bool operator!=(Vec4U8 a, Vec4U8 b)
{
  return a.x != b.x | a.y != b.y | a.z != b.z | a.w != b.w;
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

constexpr Vec4U8 & operator+=(Vec4U8 & a, Vec4U8 b)
{
  a = a + b;
  return a;
}

constexpr Vec4U8 & operator-=(Vec4U8 & a, Vec4U8 b)
{
  a = a - b;
  return a;
}

constexpr Vec4U8 & operator*=(Vec4U8 & a, Vec4U8 b)
{
  a = a * b;
  return a;
}

constexpr Vec4U8 & operator/=(Vec4U8 & a, Vec4U8 b)
{
  a = a / b;
  return a;
}

struct Vec2I
{
  i32 x = 0;
  i32 y = 0;

  static constexpr Vec2I splat(i32 value)
  {
    return Vec2I{value, value};
  }

  constexpr i32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr i32 const & operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec2I a, Vec2I b)
{
  return a.x == b.x & a.y == b.y;
}

constexpr bool operator!=(Vec2I a, Vec2I b)
{
  return a.x != b.x | a.y != b.y;
}

constexpr Vec2I operator+(Vec2I a, Vec2I b)
{
  return Vec2I{a.x + b.x, a.y + b.y};
}

constexpr Vec2I operator+(Vec2I a, i32 b)
{
  return Vec2I{a.x + b, a.y + b};
}

constexpr Vec2I operator+(i32 a, Vec2I b)
{
  return Vec2I{a + b.x, a + b.y};
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

constexpr Vec2I & operator+=(Vec2I & a, Vec2I b)
{
  a = a + b;
  return a;
}

constexpr Vec2I & operator-=(Vec2I & a, Vec2I b)
{
  a = a - b;
  return a;
}

constexpr Vec2I & operator*=(Vec2I & a, Vec2I b)
{
  a = a * b;
  return a;
}

constexpr Vec2I & operator/=(Vec2I & a, Vec2I b)
{
  a = a / b;
  return a;
}

struct Vec3I
{
  i32 x = 0;
  i32 y = 0;
  i32 z = 0;

  constexpr i32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr i32 const & operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec3I a, Vec3I b)
{
  return a.x == b.x & a.y == b.y & a.z == b.z;
}

constexpr bool operator!=(Vec3I a, Vec3I b)
{
  return a.x != b.x | a.y != b.y | a.z != b.z;
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

constexpr Vec3I & operator+=(Vec3I & a, Vec3I b)
{
  a = a + b;
  return a;
}

constexpr Vec3I & operator-=(Vec3I & a, Vec3I b)
{
  a = a - b;
  return a;
}

constexpr Vec3I & operator*=(Vec3I & a, Vec3I b)
{
  a = a * b;
  return a;
}

constexpr Vec3I & operator/=(Vec3I & a, Vec3I b)
{
  a = a / b;
  return a;
}

struct Vec4I
{
  i32 x = 0;
  i32 y = 0;
  i32 z = 0;
  i32 w = 0;

  static constexpr Vec4I splat(i32 value)
  {
    return Vec4I{value, value, value, value};
  }

  constexpr i32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr i32 const & operator[](usize i) const
  {
    return (&x)[i];
  }

  constexpr Vec2I xy() const
  {
    return {x, y};
  }

  constexpr Vec2I yz() const
  {
    return {y, z};
  }

  constexpr Vec3I xyz() const
  {
    return {x, y, z};
  }

  constexpr Vec3I yzw() const
  {
    return {y, z, w};
  }
};

constexpr bool operator==(Vec4I a, Vec4I b)
{
  return a.x == b.x & a.y == b.y & a.z == b.z & a.w == b.w;
}

constexpr bool operator!=(Vec4I a, Vec4I b)
{
  return a.x != b.x | a.y != b.y | a.z != b.z | a.w != b.w;
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

constexpr Vec4I & operator+=(Vec4I & a, Vec4I b)
{
  a = a + b;
  return a;
}

constexpr Vec4I & operator-=(Vec4I & a, Vec4I b)
{
  a = a - b;
  return a;
}

constexpr Vec4I & operator*=(Vec4I & a, Vec4I b)
{
  a = a * b;
  return a;
}

constexpr Vec4I & operator/=(Vec4I & a, Vec4I b)
{
  a = a / b;
  return a;
}

struct Vec2U
{
  u32 x = 0;
  u32 y = 0;

  static constexpr Vec2U splat(u32 value)
  {
    return Vec2U{value, value};
  }

  constexpr u32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u32 const & operator[](usize i) const
  {
    return (&x)[i];
  }

  constexpr bool is_visible() const
  {
    return x > 0 && y > 0;
  }
};

constexpr bool operator==(Vec2U a, Vec2U b)
{
  return a.x == b.x & a.y == b.y;
}

constexpr bool operator!=(Vec2U a, Vec2U b)
{
  return a.x != b.x | a.y != b.y;
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

constexpr Vec2U sat_add(Vec2U a, Vec2U b)
{
  return Vec2U{sat_add(a.x, b.x), sat_add(a.y, b.y)};
}

constexpr Vec2U sat_add(Vec2U a, u32 b)
{
  return Vec2U{sat_add(a.x, b), sat_add(a.y, b)};
}

constexpr Vec2U sat_add(u32 a, Vec2U b)
{
  return Vec2U{sat_add(a, b.x), sat_add(a, b.y)};
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

constexpr Vec2U sat_sub(Vec2U a, Vec2U b)
{
  return Vec2U{sat_sub(a.x, b.x), sat_sub(a.y, b.y)};
}

constexpr Vec2U sat_sub(Vec2U a, u32 b)
{
  return Vec2U{sat_sub(a.x, b), sat_sub(a.y, b)};
}

constexpr Vec2U sat_sub(u32 a, Vec2U b)
{
  return Vec2U{sat_sub(a, b.x), sat_sub(a, b.y)};
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

constexpr Vec2U & operator+=(Vec2U & a, Vec2U b)
{
  a = a + b;
  return a;
}

constexpr Vec2U & operator-=(Vec2U & a, Vec2U b)
{
  a = a - b;
  return a;
}

constexpr Vec2U & operator*=(Vec2U & a, Vec2U b)
{
  a = a * b;
  return a;
}

constexpr Vec2U & operator/=(Vec2U & a, Vec2U b)
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

  constexpr u32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u32 const & operator[](usize i) const
  {
    return (&x)[i];
  }

  constexpr Vec2U xy() const
  {
    return {x, y};
  }

  constexpr Vec2U yz() const
  {
    return {y, z};
  }
};

constexpr bool operator==(Vec3U a, Vec3U b)
{
  return a.x == b.x & a.y == b.y & a.z == b.z;
}

constexpr bool operator!=(Vec3U a, Vec3U b)
{
  return a.x != b.x | a.y != b.y | a.z != b.z;
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

constexpr Vec3U & operator+=(Vec3U & a, Vec3U b)
{
  a = a + b;
  return a;
}

constexpr Vec3U & operator-=(Vec3U & a, Vec3U b)
{
  a = a - b;
  return a;
}

constexpr Vec3U & operator*=(Vec3U & a, Vec3U b)
{
  a = a * b;
  return a;
}

constexpr Vec3U & operator/=(Vec3U & a, Vec3U b)
{
  a = a / b;
  return a;
}

constexpr Vec3U vec3u(Vec2U xy, u32 z)
{
  return Vec3U{.x = xy.x, .y = xy.y, .z = z};
}

struct Vec4U
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
  u32 w = 0;

  static constexpr Vec4U splat(u32 value)
  {
    return Vec4U{value, value, value, value};
  }

  constexpr u32 & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr u32 const & operator[](usize i) const
  {
    return (&x)[i];
  }

  constexpr Vec2U xy() const
  {
    return {x, y};
  }

  constexpr Vec2U yz() const
  {
    return {y, z};
  }

  constexpr Vec3U xyz() const
  {
    return {x, y, z};
  }

  constexpr Vec3U yzw() const
  {
    return {y, z, w};
  }
};

constexpr bool operator==(Vec4U a, Vec4U b)
{
  return a.x == b.x & a.y == b.y & a.z == b.z & a.w == b.w;
}

constexpr bool operator!=(Vec4U a, Vec4U b)
{
  return a.x != b.x | a.y != b.y | a.z != b.z | a.w != b.w;
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

constexpr Vec4U & operator+=(Vec4U & a, Vec4U b)
{
  a = a + b;
  return a;
}

constexpr Vec4U & operator-=(Vec4U & a, Vec4U b)
{
  a = a - b;
  return a;
}

constexpr Vec4U & operator*=(Vec4U & a, Vec4U b)
{
  a = a * b;
  return a;
}

constexpr Vec4U & operator/=(Vec4U & a, Vec4U b)
{
  a = a / b;
  return a;
}

constexpr Vec2 clamp_vec(Vec2 v, Vec2 low, Vec2 high)
{
  return Vec2{clamp(v.x, low.x, high.x), clamp(v.y, low.y, high.y)};
}

constexpr Vec2U clamp_vec(Vec2U v, Vec2U low, Vec2U high)
{
  return Vec2U{clamp(v.x, low.x, high.x), clamp(v.y, low.y, high.y)};
}

constexpr Vec2 min_vec(Vec2 v, Vec2 high)
{
  return Vec2{min(v.x, high.x), min(v.y, high.y)};
}

constexpr Vec2U min_vec(Vec2U v, Vec2U high)
{
  return Vec2U{min(v.x, high.x), min(v.y, high.y)};
}

constexpr Vec2 as_vec2(Vec2U a)
{
  return Vec2{(f32) a.x, (f32) a.y};
}

constexpr Vec2 as_vec2(Vec2I a)
{
  return Vec2{(f32) a.x, (f32) a.y};
}

constexpr Vec2I as_vec2i(Vec2U a)
{
  return Vec2I{(i32) a.x, (i32) a.y};
}

constexpr Vec2I as_vec2i(Vec2 a)
{
  return Vec2I{(i32) a.x, (i32) a.y};
}

constexpr Vec2U as_vec2u(Vec2I a)
{
  return Vec2U{(u32) a.x, (u32) a.y};
}

constexpr Vec2U as_vec2u(Vec2 a)
{
  return Vec2U{(u32) a.x, (u32) a.y};
}

constexpr Vec4U8 as_vec4u8(Vec4 a)
{
  return Vec4U8{(u8) a.x, (u8) a.y, (u8) a.z, (u8) a.w};
}

constexpr Vec4 as_vec4(Vec4U8 a)
{
  return Vec4{(f32) a.x, (f32) a.y, (f32) a.z, (f32) a.w};
}

constexpr Vec4 norm(Vec4U8 color)
{
  constexpr f32 SCALE = 1 / 255.0F;
  return as_vec4(color) * Vec4::splat(SCALE);
}

constexpr f32 dot(Vec2 a, Vec2 b)
{
  return (a.x * b.x) + a.y * b.y;
}

constexpr i32 dot(Vec2I a, Vec2I b)
{
  return (a.x * b.x) + a.y * b.y;
}

constexpr f32 dot(Vec3 a, Vec3 b)
{
  return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

constexpr i32 dot(Vec3I a, Vec3I b)
{
  return (a.x * b.x + a.y * b.y) + a.z * b.z;
}

constexpr f32 dot(Vec4 a, Vec4 b)
{
  return (((a.x * b.x) + a.y * b.y) + a.z * b.z) + a.w * b.w;
}

constexpr i32 dot(Vec4I a, Vec4I b)
{
  return ((a.x * b.x + a.y * b.y) + a.z * b.z) + a.w * b.w;
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

constexpr Vec2 normalize(Vec2 a)
{
  return a * invsqrt(dot(a, a));
}

constexpr Vec3 normalize(Vec3 a)
{
  return a * invsqrt(dot(a, a));
}

constexpr Vec4 normalize(Vec4 a)
{
  return a * invsqrt(dot(a, a));
}

struct Mat2
{
  Vec2 rows[2] = {};

  static constexpr Mat2 splat(f32 value)
  {
    return Mat2{
      .rows = {{value, value}, {value, value}}
    };
  }

  static constexpr Mat2 diagonal(f32 value)
  {
    return Mat2{
      .rows = {{value, 0}, {0, value}}
    };
  }

  static constexpr Mat2 identity()
  {
    return diagonal(1);
  }

  constexpr Vec2 & operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec2 const & operator[](usize index) const
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

constexpr bool operator==(Mat2 const & a, Mat2 const & b)
{
  return a[0] == b[0] & a[1] == b[1];
}

constexpr bool operator!=(Mat2 const & a, Mat2 const & b)
{
  return a[0] != b[0] | a[1] != b[1];
}

constexpr Mat2 operator+(Mat2 const & a, Mat2 const & b)
{
  return Mat2{
    .rows = {a[0] + b[0], a[1] + b[1]}
  };
}

constexpr Mat2 operator-(Mat2 const & a, Mat2 const & b)
{
  return Mat2{
    .rows = {a[0] - b[0], a[1] - b[1]}
  };
}

constexpr Vec2 operator*(Mat2 const & a, Vec2 const & b)
{
  return Vec2{dot(a[0], b), dot(a[1], b)};
}

constexpr Mat2 operator*(Mat2 const & a, Mat2 const & b)
{
  return Mat2{
    .rows = {{dot(a[0], b.x()), dot(a[0], b.y())},
             {dot(a[1], b.x()), dot(a[1], b.y())}}
  };
}

constexpr Mat2 operator*(Mat2 const & a, f32 b)
{
  return Mat2{
    .rows = {a[0] * b, a[1] * b}
  };
}

constexpr Mat2 operator*(f32 a, Mat2 const & b)
{
  return b * a;
}

constexpr Mat2 operator/(Mat2 const & a, Mat2 const & b)
{
  return Mat2{
    .rows = {a[0] / b[0], a[1] / b[1]}
  };
}

constexpr Mat2 & operator+=(Mat2 & a, Mat2 const & b)
{
  a = a + b;
  return a;
}

constexpr Mat2 & operator-=(Mat2 & a, Mat2 const & b)
{
  a = a - b;
  return a;
}

constexpr Mat2 & operator*=(Mat2 & a, Mat2 const & b)
{
  a = a * b;
  return a;
}

constexpr Mat2 & operator/=(Mat2 & a, Mat2 const & b)
{
  a = a / b;
  return a;
}

struct Mat3
{
  Vec3 rows[3] = {};

  static constexpr Mat3 splat(f32 value)
  {
    return Mat3{
      .rows = {
               {value, value, value}, {value, value, value}, {value, value, value}}
    };
  }

  static constexpr Mat3 diagonal(f32 value)
  {
    return Mat3{
      .rows = {{value, 0, 0}, {0, value, 0}, {0, 0, value}}
    };
  }

  static constexpr Mat3 identity()
  {
    return diagonal(1);
  }

  constexpr Vec3 & operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec3 const & operator[](usize index) const
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

constexpr bool operator==(Mat3 const & a, Mat3 const & b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

constexpr bool operator!=(Mat3 const & a, Mat3 const & b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

constexpr Mat3 operator+(Mat3 const & a, Mat3 const & b)
{
  return Mat3{
    .rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2]}
  };
}

constexpr Mat3 operator-(Mat3 const & a, Mat3 const & b)
{
  return Mat3{
    .rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2]}
  };
}

constexpr Vec3 operator*(Mat3 const & a, Vec3 const & b)
{
  return Vec3{dot(a[0], b), dot(a[1], b), dot(a[2], b)};
}

constexpr Mat3 operator*(Mat3 const & a, Mat3 const & b)
{
  return Mat3{
    .rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
             {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z())}}
  };
}

constexpr Mat3 operator/(Mat3 const & a, Mat3 const & b)
{
  return Mat3{
    .rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2]}
  };
}

constexpr Mat3 & operator+=(Mat3 & a, Mat3 const & b)
{
  a = a + b;
  return a;
}

constexpr Mat3 & operator-=(Mat3 & a, Mat3 const & b)
{
  a = a - b;
  return a;
}

constexpr Mat3 & operator*=(Mat3 & a, Mat3 const & b)
{
  a = a * b;
  return a;
}

constexpr Mat3 & operator/=(Mat3 & a, Mat3 const & b)
{
  a = a / b;
  return a;
}

struct Affine3
{
  static constexpr Vec3 TRAILING_ROW = Vec3{0, 0, 1};

  static Affine3 const IDENTITY;

  Vec3 rows[2] = {};

  constexpr Vec3 & operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec3 const & operator[](usize index) const
  {
    return rows[index];
  }

  constexpr operator Mat3() const
  {
    return Mat3{
      .rows = {rows[0], rows[1], {0, 0, 1}}
    };
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

inline constexpr Affine3 Affine3::IDENTITY{
  .rows = {{1, 0, 0}, {0, 1, 0}}
};

constexpr bool operator==(Affine3 const & a, Affine3 const & b)
{
  return a[0] == b[0] && a[1] == b[1];
}

constexpr bool operator!=(Affine3 const & a, Affine3 const & b)
{
  return a[0] != b[0] || a[1] != b[1];
}

constexpr Affine3 operator+(Affine3 const & a, Affine3 const & b)
{
  return Affine3{
    .rows = {a[0] + b[0], a[1] + b[1]}
  };
}

constexpr Affine3 operator-(Affine3 const & a, Affine3 const & b)
{
  return Affine3{
    .rows = {a[0] - b[0], a[1] - b[1]}
  };
}

constexpr Vec3 operator*(Affine3 const & a, Vec3 const & b)
{
  return Vec3{dot(a[0], b), dot(a[1], b), dot(Affine3::TRAILING_ROW, b)};
}

constexpr Mat3 operator*(Affine3 const & a, Mat3 const & b)
{
  return Mat3{
    .rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
             {dot(Affine3::TRAILING_ROW, b.x()),
              dot(Affine3::TRAILING_ROW, b.y()),
              dot(Affine3::TRAILING_ROW, b.z())}}
  };
}

constexpr Mat3 operator*(Mat3 const & a, Affine3 const & b)
{
  return Mat3{
    .rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
             {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z())}}
  };
}

constexpr Mat3 operator*(Mat3 const & a, f32 b)
{
  return Mat3{
    .rows = {a[0] * b, a[1] * b, a[2] * b}
  };
}

constexpr Mat3 operator*(f32 a, Mat3 const & b)
{
  return b * a;
}

constexpr Affine3 operator*(Affine3 const & a, Affine3 const & b)
{
  return Affine3{
    .rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())}}
  };
}

constexpr Affine3 operator/(Affine3 const & a, Affine3 const & b)
{
  return Affine3{
    .rows = {a[0] / b[0], a[1] / b[1]}
  };
}

constexpr Affine3 & operator+=(Affine3 & a, Affine3 const & b)
{
  a = a + b;
  return a;
}

constexpr Affine3 & operator-=(Affine3 & a, Affine3 const & b)
{
  a = a - b;
  return a;
}

constexpr Affine3 & operator*=(Affine3 & a, Affine3 const & b)
{
  a = a * b;
  return a;
}

constexpr Affine3 & operator/=(Affine3 & a, Affine3 const & b)
{
  a = a / b;
  return a;
}

struct Mat4
{
  static Mat4 const IDENTITY;

  Vec4 rows[4] = {};

  static constexpr Mat4 splat(f32 value)
  {
    return Mat4{
      .rows = {{value, value, value, value},
               {value, value, value, value},
               {value, value, value, value},
               {value, value, value, value}}
    };
  }

  static constexpr Mat4 diagonal(f32 value)
  {
    return Mat4{
      .rows = {{value, 0, 0, 0},
               {0, value, 0, 0},
               {0, 0, value, 0},
               {0, 0, 0, value}}
    };
  }

  constexpr Vec4 & operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec4 const & operator[](usize index) const
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

inline constexpr Mat4 Mat4::IDENTITY{
  .rows{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}
};

constexpr bool operator==(Mat4 const & a, Mat4 const & b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

constexpr bool operator!=(Mat4 const & a, Mat4 const & b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

constexpr Mat4 operator+(Mat4 const & a, Mat4 const & b)
{
  return Mat4{
    .rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]}
  };
}

constexpr Mat4 operator-(Mat4 const & a, Mat4 const & b)
{
  return Mat4{
    .rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]}
  };
}

constexpr Vec4 operator*(Mat4 const & a, Vec4 const & b)
{
  return Vec4{dot(a[0], b), dot(a[1], b), dot(a[2], b), dot(a[3], b)};
}

constexpr Mat4 operator*(Mat4 const & a, Mat4 const & b)
{
  return Mat4{
    .rows = {
             {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()), dot(a[0], b.w())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()), dot(a[1], b.w())},
             {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()), dot(a[2], b.w())},
             {dot(a[3], b.x()), dot(a[3], b.y()), dot(a[3], b.z()),
       dot(a[3], b.w())}}
  };
}

constexpr Mat4 operator*(Mat4 const & a, f32 b)
{
  return Mat4{
    .rows = {a[0] * b, a[1] * b, a[2] * b, a[3] * b}
  };
}

constexpr Mat4 operator*(f32 a, Mat4 const & b)
{
  return b * a;
}

constexpr Mat4 operator/(Mat4 const & a, Mat4 const & b)
{
  return Mat4{
    .rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2], a[3] / b[3]}
  };
}

constexpr Mat4 & operator+=(Mat4 & a, Mat4 const & b)
{
  a = a + b;
  return a;
}

constexpr Mat4 & operator-=(Mat4 & a, Mat4 const & b)
{
  a = a - b;
  return a;
}

constexpr Mat4 & operator*=(Mat4 & a, Mat4 const & b)
{
  a = a * b;
  return a;
}

constexpr Mat4 & operator/=(Mat4 & a, Mat4 const & b)
{
  a = a / b;
  return a;
}

struct Affine4
{
  static constexpr Vec4 TRAILING_ROW = Vec4{0, 0, 0, 1};

  static Affine4 const IDENTITY;

  Vec4 rows[3] = {};

  constexpr Vec4 & operator[](usize index)
  {
    return rows[index];
  }

  constexpr Vec4 const & operator[](usize index) const
  {
    return rows[index];
  }

  constexpr operator Mat4() const
  {
    return Mat4{
      .rows = {rows[0], rows[1], rows[2], {0, 0, 0, 1}}
    };
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

inline constexpr Affine4 Affine4::IDENTITY{
  .rows = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}}
};

constexpr bool operator==(Affine4 const & a, Affine4 const & b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

constexpr bool operator!=(Affine4 const & a, Affine4 const & b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

constexpr Affine4 operator+(Affine4 const & a, Affine4 const & b)
{
  return Affine4{
    .rows = {a[0] + b[0], a[1] + b[1], a[2] + b[2]}
  };
}

constexpr Affine4 operator-(Affine4 const & a, Affine4 const & b)
{
  return Affine4{
    .rows = {a[0] - b[0], a[1] - b[1], a[2] - b[2]}
  };
}

constexpr Vec4 operator*(Affine4 const & a, Vec4 const & b)
{
  return Vec4{dot(a[0], b), dot(a[1], b), dot(a[2], b),
              dot(Affine4::TRAILING_ROW, b)};
}

constexpr Mat4 operator*(Affine4 const & a, Mat4 const & b)
{
  return Mat4{
    .rows = {
             {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()), dot(a[0], b.w())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()), dot(a[1], b.w())},
             {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()), dot(a[2], b.w())},
             {dot(Affine4::TRAILING_ROW, b.x()), dot(Affine4::TRAILING_ROW, b.y()),
       dot(Affine4::TRAILING_ROW, b.z()), dot(Affine4::TRAILING_ROW, b.w())},
             }
  };
}

constexpr Mat4 operator*(Mat4 const & a, Affine4 const & b)
{
  return Mat4{
    .rows = {
             {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()), dot(a[0], b.w())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()), dot(a[1], b.w())},
             {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()), dot(a[2], b.w())},
             {dot(a[3], b.x()), dot(a[3], b.y()), dot(a[3], b.z()), dot(a[3], b.w())},
             }
  };
}

constexpr Affine4 operator*(Affine4 const & a, Affine4 const & b)
{
  return Affine4{
    .rows = {
             {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()), dot(a[0], b.w())},
             {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()), dot(a[1], b.w())},
             {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()),
       dot(a[2], b.w())}}
  };
}

constexpr Affine4 operator/(Affine4 const & a, Affine4 const & b)
{
  return Affine4{
    .rows = {a[0] / b[0], a[1] / b[1], a[2] / b[2]}
  };
}

constexpr Affine4 & operator+=(Affine4 & a, Affine4 const & b)
{
  a = a + b;
  return a;
}

constexpr Affine4 & operator-=(Affine4 & a, Affine4 const & b)
{
  a = a - b;
  return a;
}

constexpr Affine4 & operator*=(Affine4 & a, Affine4 const & b)
{
  a = a * b;
  return a;
}

constexpr Affine4 & operator/=(Affine4 & a, Affine4 const & b)
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

constexpr Vec2U mip_size(Vec2U a, u32 level)
{
  return Vec2U{mip_size(a.x, level), mip_size(a.y, level)};
}

constexpr Vec3U mip_size(Vec3U a, u32 level)
{
  return Vec3U{mip_size(a.x, level), mip_size(a.y, level),
               mip_size(a.z, level)};
}

constexpr Vec4U mip_size(Vec4U a, u32 level)
{
  return Vec4U{mip_size(a.x, level), mip_size(a.y, level), mip_size(a.z, level),
               mip_size(a.w, level)};
}

constexpr u32 num_mip_levels(Vec2U a)
{
  u32 m = max(a.x, a.y);
  return (m == 0) ? 0 : (ulog2(m) + 1);
}

constexpr u32 num_mip_levels(Vec3U a)
{
  u32 m = max(max(a.x, a.y), a.z);
  return (m == 0) ? 0 : (ulog2(m) + 1);
}

constexpr u32 num_mip_levels(Vec4U a)
{
  u32 m = max(max(max(a.x, a.y), a.z), a.w);
  return (m == 0) ? 0 : (ulog2(m) + 1);
}

constexpr Mat2 transpose(Mat2 const & a)
{
  return Mat2{
    .rows = {a.x(), a.y()}
  };
}

constexpr Mat3 transpose(Mat3 const & a)
{
  return Mat3{
    .rows = {a.x(), a.y(), a.z()}
  };
}

constexpr Mat4 transpose(Mat4 const & a)
{
  return Mat4{
    .rows = {a.x(), a.y(), a.z(), a.w()}
  };
}

constexpr f32 determinant(Mat2 const & a)
{
  return a[0].x * a[1].y - a[1].x * a[0].y;
}

constexpr f32 determinant(Mat3 const & a)
{
  return a[0].x * a[1].y * a[2].z - a[0].x * a[1].z * a[2].y -
         a[0].y * a[1].x * a[2].z + a[0].y * a[1].z * a[2].x +
         a[0].z * a[1].x * a[2].y - a[0].z * a[1].y * a[2].x;
}

constexpr f32 determinant(Mat4 const & a)
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

constexpr Mat2 adjoint(Mat2 const & a)
{
  return Mat2{
    .rows = {{a[1].y, -a[0].y}, {-a[1].x, a[0].x}}
  };
}

constexpr Mat3 adjoint(Mat3 const & a)
{
  return Mat3{
    .rows = {
             {a[1].y * a[2].z - a[1].z * a[2].y, a[0].z * a[2].y - a[0].y * a[2].z,
       a[0].y * a[1].z - a[0].z * a[1].y},
             {a[1].z * a[2].x - a[1].x * a[2].z, a[0].x * a[2].z - a[0].z * a[2].x,
       a[0].z * a[1].x - a[0].x * a[1].z},
             {a[1].x * a[2].y - a[1].y * a[2].x, a[0].y * a[2].x - a[0].x * a[2].y,
       a[0].x * a[1].y - a[0].y * a[1].x}}
  };
}

constexpr Mat4 adjoint(Mat4 const & a)
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
           a[1].y * a[2].x * a[3].w - a[1].x * a[2].w * a[3].y;
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
  r[3].w = a[0].x * a[1].y * a[2].z + a[0].y * a[1].z * a[2].x +
           a[0].z * a[1].x * a[2].y - a[0].z * a[1].y * a[2].x -
           a[0].y * a[1].x * a[2].z - a[0].x * a[1].z * a[2].y;
  return r;
}

constexpr Mat2 inverse(Mat2 a)
{
  return (1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3 inverse(Mat3 const & a)
{
  return (1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat4 inverse(Mat4 const & a)
{
  return (1.0F / determinant(a)) * adjoint(a);
}

constexpr Affine3 translate2d(Vec2 t)
{
  return Affine3{
    .rows = {{1, 0, t.x}, {0, 1, t.y}}
  };
}

constexpr Affine4 translate3d(Vec3 t)
{
  return Affine4{
    .rows = {{1, 0, 0, t.x}, {0, 1, 0, t.y}, {0, 0, 1, t.z}}
  };
}

constexpr Affine3 scale2d(Vec2 s)
{
  return Affine3{
    .rows = {{s.x, 0, 0}, {0, s.y, 0}}
  };
}

constexpr Affine4 scale3d(Vec3 s)
{
  return Affine4{
    .rows = {{s.x, 0, 0, 0}, {0, s.y, 0, 0}, {0, 0, s.z, 0}}
  };
}

inline Affine3 rotate2d(f32 radians)
{
  return Affine3{
    .rows = {{cos(radians), -sin(radians), 0},
             {sin(radians), cos(radians), 0}}
  };
}

inline Affine4 rotate3d_x(f32 radians)
{
  return Affine4{
    .rows = {{1, 0, 0, 0},
             {0, cos(radians), -sin(radians), 0},
             {0, sin(radians), cos(radians), 0}}
  };
}

inline Affine4 rotate3d_y(f32 radians)
{
  return Affine4{
    .rows = {{cos(radians), 0, sin(radians), 0},
             {0, 1, 0, 0},
             {-sin(radians), 0, cos(radians), 0}}
  };
}

inline Affine4 rotate3d_z(f32 radians)
{
  return Affine4{
    .rows = {{cos(radians), -sin(radians), 0, 0},
             {sin(radians), cos(radians), 0, 0},
             {0, 0, 1, 0}}
  };
}

constexpr Vec2 transform(Mat3 const & t, Vec2 value)
{
  Vec3 v = t * Vec3{value.x, value.y, 1};
  return Vec2{v.x, v.y};
}

constexpr Vec2 transform(Affine3 const & t, Vec2 value)
{
  Vec3 v = t * Vec3{value.x, value.y, 1};
  return Vec2{v.x, v.y};
}

constexpr Vec3 transform(Mat4 const & t, Vec3 value)
{
  Vec4 v = t * Vec4{value.x, value.y, value.z, 1};
  return Vec3{v.x, v.y, v.z};
}

constexpr Vec3 transform(Affine4 const & t, Vec3 value)
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

inline constexpr Affine4 transform2d_to_3d(Affine3 const & mat)
{
  return Affine4{
    .rows = {{mat.rows[0][0], mat.rows[0][1], 0, mat.rows[0][2]},
             {mat.rows[1][0], mat.rows[1][1], 0, mat.rows[1][2]},
             {0.0F, 0.0F, 1.0F, 0.0F}}
  };
}

inline constexpr Affine3 transform3d_to_2d(Affine4 const & mat)
{
  return Affine3{
    .rows = {{mat.rows[0][0], mat.rows[0][1], mat.rows[0][3]},
             {mat.rows[1][0], mat.rows[1][1], mat.rows[1][3]}}
  };
}

inline constexpr Mat3 transform3d_to_2d(Mat4 const & mat)
{
  return Mat3{
    .rows = {{mat.rows[0][0], mat.rows[0][1], mat.rows[0][3]},
             {mat.rows[1][0], mat.rows[1][1], mat.rows[1][3]},
             {mat.rows[2][0], mat.rows[2][1], mat.rows[2][3]}}
  };
}

inline constexpr f32 ALIGNMENT_LEFT   = -0.5F;
inline constexpr f32 ALIGNMENT_RIGHT  = 0.5F;
inline constexpr f32 ALIGNMENT_TOP    = -0.5F;
inline constexpr f32 ALIGNMENT_BOTTOM = 0.5F;
inline constexpr f32 ALIGNMENT_CENTER = 0;

inline constexpr Vec2 ALIGNMENT_CENTER_CENTER{0, 0};
inline constexpr Vec2 ALIGNMENT_TOP_LEFT{-0.5F, -0.5F};
inline constexpr Vec2 ALIGNMENT_TOP_CENTER{0, -0.5F};
inline constexpr Vec2 ALIGNMENT_TOP_RIGHT{0.5F, -0.5F};
inline constexpr Vec2 ALIGNMENT_BOTTOM_LEFT{-0.5F, 0.5F};
inline constexpr Vec2 ALIGNMENT_BOTTOM_CENTER{0, 0.5F};
inline constexpr Vec2 ALIGNMENT_BOTTOM_RIGHT{0.5F, 0.5F};
inline constexpr Vec2 ALIGNMENT_LEFT_CENTER{-0.5F, 0};
inline constexpr Vec2 ALIGNMENT_RIGHT_CENTER{0.5F, 0};

typedef Vec2 f32x2;
typedef Vec3 f32x3;
typedef Vec4 f32x4;
typedef Mat2 f32x2x2;
typedef Mat3 f32x3x3;
typedef Mat4 f32x4x4;

constexpr Vec4 opacity(f32 v)
{
  return Vec4{1, 1, 1, v};
}

constexpr Vec4 opacity_premul(f32 v)
{
  return Vec4::splat(v);
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

constexpr Tuple<Vec2, Vec2> intersect(Vec2 a_begin, Vec2 a_end, Vec2 b_begin,
                                      Vec2 b_end)
{
  if (!overlaps(a_begin, a_end, b_begin, b_end))
  {
    return {};
  }

  return {
    Vec2{max(a_begin.x, b_begin.x), max(a_begin.y, b_begin.y)},
    Vec2{min(a_end.x,   b_end.x),   min(a_end.y,   b_end.y)  }
  };
}

struct Rect
{
  Vec2 offset = {};
  Vec2 extent = {};

  static constexpr Rect from_center(Vec2 center, Vec2 extent)
  {
    return Rect{.offset = center - extent * 0.5F, .extent = extent};
  }

  static constexpr Rect range(Vec2 begin, Vec2 end)
  {
    return Rect{.offset = begin, .extent = end - begin};
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
    return extent.x != 0 & extent.y != 0;
  }

  constexpr bool contains(Vec2 point) const
  {
    return contains_point(begin(), end(), point);
  }

  constexpr bool overlaps(Rect const & r) const
  {
    return ash::overlaps(begin(), end(), r.begin(), r.end());
  }

  constexpr Rect intersect(Rect const & r) const
  {
    auto [b, e] = ash::intersect(begin(), end(), r.begin(), r.end());
    return Rect::range(b, e);
  }
};

constexpr bool operator==(Rect const & a, Rect const & b)
{
  return a.offset == b.offset & a.extent == b.extent;
}

constexpr bool operator!=(Rect const & a, Rect const & b)
{
  return a.offset != b.offset || a.extent != b.extent;
}

struct CRect
{
  Vec2 center = {};
  Vec2 extent = {};

  static constexpr auto from_offset(Vec2 offset, Vec2 extent)
  {
    return CRect{.center = offset + extent * 0.5F, .extent = extent};
  }

  static constexpr auto range(Vec2 begin, Vec2 end)
  {
    return CRect{.center = (begin + end) * 0.5F, .extent = (end - begin)};
  }

  static constexpr auto bounding(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3)
  {
    return CRect::range(
      Vec2{min(p0.x, p1.x, p2.x, p3.x), min(p0.y, p1.y, p2.y, p3.y)},
      Vec2{max(p0.x, p1.x, p2.x, p3.x), max(p0.y, p1.y, p2.y, p3.y)});
  }

  constexpr auto begin() const
  {
    return center - (extent * 0.5F);
  }

  constexpr auto end() const
  {
    return center + extent * 0.5F;
  }

  constexpr auto tl() const
  {
    return begin();
  }

  constexpr auto tr() const
  {
    return Vec2{center.x + 0.5F * extent.x, center.y - 0.5F * extent.y};
  }

  constexpr auto bl() const
  {
    return Vec2{center.x - 0.5F * extent.x, center.y + 0.5F * extent.y};
  }

  constexpr auto br() const
  {
    return end();
  }

  constexpr auto bounds() const
  {
    return Tuple{tl(), tr(), bl(), br()};
  }

  constexpr auto area() const
  {
    return extent.x * extent.y;
  }

  constexpr Rect offseted() const;

  constexpr bool is_visible() const
  {
    return extent.x != 0 & extent.y != 0;
  }

  constexpr bool contains(Vec2 point) const
  {
    return contains_point(begin(), end(), point);
  }

  constexpr bool overlaps(CRect const & r) const
  {
    return ash::overlaps(begin(), end(), r.begin(), r.end());
  }

  constexpr auto intersect(CRect const & r) const
  {
    auto [b, e] = ash::intersect(begin(), end(), r.begin(), r.end());
    return CRect::range(b, e);
  }

  constexpr auto unioned(CRect const & r) const
  {
    return CRect::bounding(begin(), end(), r.begin(), r.end());
  }

  constexpr auto transform(Affine3 const & t)
  {
    return CRect::bounding(ash::transform(t, tl()), ash::transform(t, tr()),
                           ash::transform(t, bl()), ash::transform(t, br()));
  }

  constexpr auto transform(Mat3 const & t)
  {
    return CRect::bounding(ash::transform(t, tl()), ash::transform(t, tr()),
                           ash::transform(t, bl()), ash::transform(t, br()));
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

constexpr bool operator==(CRect const & a, CRect const & b)
{
  return a.center == b.center & a.extent == b.extent;
}

constexpr bool operator!=(CRect const & a, CRect const & b)
{
  return a.center != b.center || a.extent != b.extent;
}

struct RectU
{
  Vec2U offset = {};
  Vec2U extent = {};

  static constexpr RectU range(Vec2U begin, Vec2U end)
  {
    return RectU{.offset = begin, .extent = (end - begin)};
  }

  constexpr Vec2U begin() const
  {
    return offset;
  }

  constexpr Vec2U end() const
  {
    return offset + extent;
  }

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0;
  }

  constexpr RectU clamp_to_extent(Vec2U base_extent)
  {
    Vec2U max_offset{sat_sub(base_extent.x, 1U), sat_sub(base_extent.y, 1U)};

    auto end = this->end();

    Vec2U new_begin{min(max_offset.x, offset.x), min(max_offset.y, offset.y)};
    Vec2U new_end{min(base_extent.x, end.x), min(base_extent.y, end.y)};

    return range(new_begin, new_end);
  }
};

constexpr bool operator==(RectU const & a, RectU const & b)
{
  return a.offset == b.offset & a.extent == b.extent;
}

constexpr bool operator!=(RectU const & a, RectU const & b)
{
  return a.offset != b.offset | a.extent != b.extent;
}

struct RectI
{
  Vec2I offset = {};
  Vec2I extent = {};

  constexpr Vec2I begin() const
  {
    return offset;
  }

  constexpr Vec2I end() const
  {
    return offset + extent;
  }
};

constexpr bool operator==(RectI const & a, RectI const & b)
{
  return a.offset == b.offset & a.extent == b.extent;
}

constexpr bool operator!=(RectI const & a, RectI const & b)
{
  return a.offset != b.offset | a.extent != b.extent;
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
    return extent.x != 0 | extent.y != 0 | extent.z != 0;
  }

  constexpr bool contains(Vec3 point) const
  {
    return offset.x <= point.x && offset.y <= point.y && offset.z <= point.z &&
           (offset.x + extent.x) >= point.x &&
           (offset.y + extent.y) >= point.y && (offset.z + extent.z) >= point.z;
    return true;
  }

  constexpr bool overlaps(Box const & b) const
  {
    Vec3 a_begin = offset;
    Vec3 a_end   = offset + extent;
    Vec3 b_begin = b.offset;
    Vec3 b_end   = b.offset + b.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y &&
           a_begin.z <= b_end.z && a_end.z >= b_begin.z;
  }
};

constexpr bool operator==(Box const & a, Box const & b)
{
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(Box const & a, Box const & b)
{
  return a.offset != b.offset || a.extent != b.extent;
}

struct BoxU
{
  Vec3U offset = {};
  Vec3U extent = {};

  constexpr Vec3U begin() const
  {
    return offset;
  }

  constexpr Vec3U end() const
  {
    return offset + extent;
  }
};

constexpr BoxU as_boxu(RectU const & r)
{
  return BoxU{.offset = vec3u(r.offset, 0), .extent = vec3u(r.extent, 1)};
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
    return extent.x != 0 & extent.y != 0 & extent.z != 0;
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

constexpr bool operator==(CBox const & a, CBox const & b)
{
  return a.center == b.center & a.extent == b.extent;
}

constexpr bool operator!=(CBox const & a, CBox const & b)
{
  return a.center != b.center || a.extent != b.extent;
}

/// @brief find the maximum extent that will fit in the provided extent while
/// respecting the provided aspect ratio
constexpr Vec2 with_aspect(Vec2 extent, f32 aspect_ratio)
{
  f32 const  base          = min(extent.x, extent.y);
  Vec2 const width_scaled  = Vec2{base * aspect_ratio, base};
  Vec2 const height_scaled = Vec2{base, base / aspect_ratio};

  if (width_scaled.x <= extent.x)
  {
    return width_scaled;
  }
  else
  {
    return height_scaled;
  }
}

/// @param x_mag The horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @param y_mag The vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @param z_near The distance to the near clipping plane.
/// @param z_far The distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
constexpr Affine4 orthographic(f32 x_mag, f32 y_mag, f32 z_near, f32 z_far)
{
  f32 const z_diff     = z_near - z_far;
  f32 const z_diff_inv = 1 / z_diff;
  return Affine4{
    {{1 / x_mag, 0, 0, 0},
     {0, 1 / y_mag, 0, 0},
     {0, 0, 2 * z_diff_inv, (z_far + z_near) * z_diff_inv}}
  };
}

/// @param aspect_ratio The aspect ratio of the field of view.
/// @param y_fov The vertical field of view in radians. This value
/// SHOULD be less than π.
/// @param z_far The distance to the far clipping plane.
/// @param z_near The distance to the near clipping plane.
inline Mat4 perspective(f32 aspect_ratio, f32 y_fov, f32 z_far, f32 z_near)
{
  f32 const s          = tanf(y_fov * 0.5F);
  f32 const z_diff     = z_near - z_far;
  f32 const z_diff_inv = 1 / z_diff;
  return Mat4{
    {{1 / (aspect_ratio * s), 0, 0, 0},
     {0, 1 / s, 0, 0},
     {0, 0, (z_far + z_near) * z_diff_inv, 2 * z_far * z_near * z_diff_inv},
     {0, 0, -1, 0}}
  };
}

constexpr Mat4 look_at(Vec3 eye, Vec3 center, Vec3 up)
{
  Vec3 const f = normalize(center - eye);
  Vec3 const s = normalize(cross(up, f));
  Vec3 const u = cross(f, s);

  return {
    {{s.x, s.x, s.x, 0},
     {u.y, u.y, u.y, 0},
     {f.z, f.z, f.z, 0},
     {-dot(s, eye), -dot(u, eye), -dot(f, eye), 1}}
  };
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
constexpr bool is_outside_frustum(Mat4 const & mvp, Vec3 offset, Vec3 extent)
{
  constexpr u32 NUM_CORNERS          = 8;
  Vec4 const    corners[NUM_CORNERS] = {
    mvp * vec4(offset, 1),
    mvp * vec4(offset + Vec3{extent.x, 0, 0}, 1),
    mvp * vec4(offset + Vec3{extent.x, extent.y, 0}, 1),
    mvp * vec4(offset + Vec3{0, extent.y, 0}, 1),
    mvp * vec4(offset + Vec3{0, 0, extent.z}, 1),
    mvp * vec4(offset + Vec3{extent.x, 0, extent.z}, 1),
    mvp * vec4(offset + extent, 1),
    mvp * vec4(offset + Vec3{0, extent.y, extent.z}, 1)};

  u32 left   = 0;
  u32 right  = 0;
  u32 top    = 0;
  u32 bottom = 0;
  u32 back   = 0;

  for (u32 i = 0; i < NUM_CORNERS; i++)
  {
    Vec4 const & corner = corners[i];

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

  return left == NUM_CORNERS | right == NUM_CORNERS | top == NUM_CORNERS |
         bottom == NUM_CORNERS | back == NUM_CORNERS;
}

/// @brief Calculate the inverse of a 2d scale and translation-only transformation matrix
constexpr Affine3 translate_scale_inv2d(Affine3 const & t)
{
  auto sx = 1 / t[0].x;
  auto sy = 1 / t[1].y;
  auto tx = t[0].z;
  auto ty = t[1].z;
  return Affine3{
    .rows{{sx, 0, -sx * tx}, {0, sy, -sy * ty}}
  };
}

// [ ] implement for shader?
inline Vec3 refract(Vec3 v, Vec3 n, f32 r)
{
  f32 dot = ash::dot(v, n);
  f32 d   = 1.0F - r * r * (1.0F - dot * dot);

  if (d >= 0.0F)
  {
    d = ash::sqrt(d);
    return r * v - (r * dot + d) * n;
  }

  return Vec3::splat(0);
}

}    // namespace ash
