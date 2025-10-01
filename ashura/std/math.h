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
template <Unsigned T>
constexpr T log2(T value)
{
  return (sizeof(T) * 8 - 1) - std::countl_zero(value);
}

constexpr u32 mip(u32 a, u32 level)
{
  return max(a >> level, 1U);
}

constexpr auto mips(Unsigned auto a)
{
  return (a == 0) ? 0 : log2(a);
}

/// @brief Linearly interpolate between points `low` and `high` given
/// interpolator `t`
/// This is the exact form: (1 - t) * A + T * B, optimized for FMA
template <typename T>
constexpr T lerp(T const & low, T const & high, T const & t)
{
  return low - t * low + t * high;
}

/// @brief Logarithmically interpolate between points `low` and `high` given
/// interpolator `t`
template <typename T>
inline T log_interp(T const & low, T const & high, T const & t)
{
  return low * exp(t * log(high / low));
}

/// @brief Frame-independent damped lerp
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

namespace math
{

template <typename T, usize N>
requires (N > 0)
struct vec
{
  using Type      = T;
  using View      = Span<T>;
  using ConstView = Span<T const>;
  using Iter      = SpanIter<T>;
  using ConstIter = SpanIter<T const>;

  static constexpr usize SIZE = N;

  Type v[N] = {};

  constexpr bool is_empty() const
  {
    return size() == 0;
  }

  constexpr Type * data()
  {
    return v;
  }

  constexpr Type const * data() const
  {
    return v;
  }

  static constexpr usize size()
  {
    return SIZE;
  }

  static constexpr usize capacity()
  {
    return size();
  }

  static constexpr usize size_bytes()
  {
    return sizeof(T) * size();
  }

  constexpr T const * pbegin() const
  {
    return data();
  }

  constexpr T const * pend() const
  {
    return data() + size();
  }

  constexpr T * pbegin()
  {
    return data();
  }

  constexpr T * pend()
  {
    return data() + size();
  }

  constexpr auto begin()
  {
    return Iter{.iter_ = pbegin(), .end_ = pend()};
  }

  constexpr auto begin() const
  {
    return ConstIter{.iter_ = pbegin(), .end_ = pend()};
  }

  constexpr auto end()
  {
    return IterEnd{};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr auto view() const
  {
    return ConstView{data(), size()};
  }

  constexpr auto view()
  {
    return View{data(), size()};
  }

  static constexpr vec splat(Type v) requires (N == 2)
  {
    return {v, v};
  }

  static constexpr vec splat(Type v) requires (N == 3)
  {
    return {v, v, v};
  }

  static constexpr vec splat(Type v) requires (N == 4)
  {
    return {v, v, v, v};
  }

  static constexpr vec splat(Type v) requires (N > 4)
  {
    vec r;
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = v;
    }
    return r;
  }

  static constexpr vec zero()
  {
    return splat(0);
  }

  static constexpr vec one()
  {
    return splat(1);
  }

  constexpr T const & x() const
  {
    return v[0];
  }

  constexpr T & x()
  {
    return v[0];
  }

  constexpr T const & y() const requires (N > 1)
  {
    return v[1];
  }

  constexpr T & y() requires (N > 1)
  {
    return v[1];
  }

  constexpr T const & z() const requires (N > 2)
  {
    return v[2];
  }

  constexpr T & z() requires (N > 2)
  {
    return v[2];
  }

  constexpr T const & w() const requires (N > 3)
  {
    return v[3];
  }

  constexpr T & w() requires (N > 3)
  {
    return v[3];
  }

  constexpr vec x(Type x) const
  {
    vec r = *this;
    r.x() = x;
    return r;
  }

  constexpr vec y(Type y) const requires (N > 1)
  {
    vec r = *this;
    r.y() = y;
    return r;
  }

  constexpr vec z(Type z) const requires (N > 2)
  {
    vec r = *this;
    r.z() = z;
    return r;
  }

  constexpr vec w(Type w) const requires (N > 3)
  {
    vec r = *this;
    r.w() = w;
    return r;
  }

  constexpr Type & i()
  {
    return v[0];
  }

  constexpr Type const & i() const
  {
    return v[0];
  }

  constexpr Type & j() requires (N > 1)
  {
    return v[1];
  }

  constexpr Type const & j() const requires (N > 1)
  {
    return v[1];
  }

  constexpr Type & k() requires (N > 2)
  {
    return v[2];
  }

  constexpr Type const & k() const requires (N > 2)
  {
    return v[2];
  }

  constexpr T const & r() const
  {
    return x();
  }

  constexpr T & r()
  {
    return x();
  }

  constexpr T const & g() const requires (N > 1)
  {
    return y();
  }

  constexpr T & g() requires (N > 1)
  {
    return y();
  }

  constexpr T const & b() const requires (N > 2)
  {
    return z();
  }

  constexpr T & b() requires (N > 2)
  {
    return z();
  }

  constexpr T const & a() const requires (N > 3)
  {
    return w();
  }

  constexpr T & a() requires (N > 3)
  {
    return w();
  }

  constexpr Type & operator[](usize i)
  {
    return v[i];
  }

  constexpr Type const & operator[](usize i) const
  {
    return v[i];
  }

  constexpr vec<Type, 2> xy() const requires (N > 2)
  {
    return {x(), y()};
  }

  constexpr vec<Type, 2> yz() const requires (N > 2)
  {
    return {y(), z()};
  }

  constexpr vec<Type, 2> xz() const requires (N > 2)
  {
    return {x(), z()};
  }

  constexpr vec<Type, 2> zw() const requires (N > 3)
  {
    return {z(), w()};
  }

  constexpr vec<Type, 2> wx() const requires (N > 3)
  {
    return {w(), x()};
  }

  constexpr vec<Type, 2> wy() const requires (N > 3)
  {
    return {w(), y()};
  }

  constexpr vec<Type, 3> xyz() const requires (N > 3)
  {
    return {x(), y(), z()};
  }

  constexpr vec<Type, 3> xzy() const requires (N > 3)
  {
    return {x(), z(), y()};
  }

  constexpr vec<Type, 3> yxz() const requires (N > 3)
  {
    return {y(), x(), z()};
  }

  constexpr vec<Type, 3> yzx() const requires (N > 3)
  {
    return {y(), z(), x()};
  }

  constexpr vec<Type, 3> zxy() const requires (N > 3)
  {
    return {z(), x(), y()};
  }

  constexpr vec<Type, 3> zyx() const requires (N > 3)
  {
    return {z(), y(), x()};
  }

  constexpr bool operator==(vec b) const
  {
    bool all_eq = true;

#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      all_eq &= (v[i] == b.v[i]);
    }

    return all_eq;
  }

  constexpr bool operator==(Type b) const
  {
    return *this == splat(b);
  }

  constexpr bool operator!=(vec b) const
  {
    bool any_neq = false;

#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      any_neq |= (v[i] != b.v[i]);
    }

    return any_neq;
  }

  constexpr bool operator!=(Type b) const
  {
    return *this != splat(b);
  }

  constexpr bool is_zero() const
  {
    return *this == splat(0);
  }

  constexpr bool is_one() const
  {
    return *this == splat(1);
  }

  constexpr bool any_zero() const
  {
    bool any_zero = false;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      any_zero |= (v[i] == 0);
    }
    return any_zero;
  }

  constexpr vec operator+(vec b) const
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] + b.v[i];
    }
    return c;
  }

  constexpr vec operator+(Type b) const
  {
    return *this + splat(b);
  }

  constexpr vec & operator+=(vec b)
  {
    *this = *this + b;
    return *this;
  }

  constexpr vec & operator+=(Type b)
  {
    *this = *this + b;
    return *this;
  }

  constexpr vec operator-() const requires (Signed<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = -v[i];
    }
    return c;
  }

  constexpr vec operator-(vec b) const
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] - b.v[i];
    }
    return c;
  }

  constexpr vec operator-(Type b) const
  {
    return *this - splat(b);
  }

  constexpr vec & operator-=(vec b)
  {
    *this = *this - b;
    return *this;
  }

  constexpr vec & operator-=(Type b)
  {
    *this = *this - b;
    return *this;
  }

  constexpr vec operator*(vec b) const
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] * b.v[i];
    }
    return c;
  }

  constexpr vec operator*(Type b) const
  {
    return *this * splat(b);
  }

  constexpr vec & operator*=(vec b)
  {
    *this = *this * b;
    return *this;
  }

  constexpr vec & operator*=(Type b)
  {
    *this = *this * b;
    return *this;
  }

  constexpr vec operator/(vec b) const
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] / b.v[i];
    }
    return c;
  }

  constexpr vec operator/(Type b) const
  {
    return *this / splat(b);
  }

  constexpr vec & operator/=(vec b)
  {
    *this = *this / b;
    return *this;
  }

  constexpr vec & operator/=(Type b)
  {
    *this = *this / b;
    return *this;
  }

  constexpr vec operator>>(vec b) const requires (Unsigned<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] >> b.v[i];
    }
    return c;
  }

  constexpr vec & operator>>=(vec b) requires (Unsigned<T>)
  {
    *this = *this >> b;
    return *this;
  }

  constexpr vec operator>>(Type b) const requires (Unsigned<T>)
  {
    return *this >> splat(b);
  }

  constexpr vec & operator>>=(Type b) requires (Unsigned<T>)
  {
    *this = *this >> b;
    return *this;
  }

  constexpr vec operator<<(vec b) const requires (Unsigned<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] << b.v[i];
    }
    return c;
  }

  constexpr vec & operator<<=(vec b) requires (Unsigned<T>)
  {
    *this = *this << b;
    return *this;
  }

  constexpr vec operator<<(Type b) const requires (Unsigned<T>)
  {
    return *this << splat(b);
  }

  constexpr vec & operator<<=(Type b) requires (Unsigned<T>)
  {
    *this = *this << b;
    return *this;
  }

  constexpr vec operator&(vec b) const requires (Unsigned<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] & b.v[i];
    }
    return c;
  }

  constexpr vec & operator&=(vec b) requires (Unsigned<T>)
  {
    *this = *this & b;
    return *this;
  }

  constexpr vec operator&(Type b) const requires (Unsigned<T>)
  {
    return *this & splat(b);
  }

  constexpr vec & operator&=(Type b) requires (Unsigned<T>)
  {
    *this = *this & b;
    return *this;
  }

  constexpr vec operator|(vec b) const requires (Unsigned<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = v[i] | b.v[i];
    }
    return c;
  }

  constexpr vec & operator|=(vec b) requires (Unsigned<T>)
  {
    *this = *this | b;
    return *this;
  }

  constexpr vec operator|(Type b) const requires (Unsigned<T>)
  {
    return *this | splat(b);
  }

  constexpr vec & operator|=(Type b) requires (Unsigned<T>)
  {
    *this = *this | b;
    return *this;
  }

  constexpr vec sat_add(vec b) const requires (Integral<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = ash::sat_add(v[i], b.v[i]);
    }
    return c;
  }

  constexpr vec sat_add(Type b) const requires (Integral<T>)
  {
    return sat_add(splat(b));
  }

  constexpr vec sat_sub(vec b) const requires (Integral<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = ash::sat_sub(v[i], b.v[i]);
    }
    return c;
  }

  constexpr vec sat_sub(Type b) const requires (Integral<T>)
  {
    return sat_sub(splat(b));
  }

  constexpr vec sat_mul(vec b) const requires (Integral<T>)
  {
    vec c;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      c.v[i] = ash::sat_mul(v[i], b.v[i]);
    }
    return c;
  }

  constexpr vec sat_mul(Type b) const requires (Integral<T>)
  {
    return sat_mul(splat(b));
  }

  constexpr Type sum() const
  {
    Type sum = 0;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      sum += v[i];
    }
    return sum;
  }

  template <typename U>
  constexpr U sum() const
  {
    U sum = 0;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      sum += static_cast<U>(v[i]);
    }
    return sum;
  }

  constexpr Type product() const
  {
    Type product = 1;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      product *= v[i];
    }
    return product;
  }

  template <typename U>
  constexpr U product() const
  {
    U product = 1;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      product *= static_cast<U>(v[i]);
    }
    return product;
  }

  constexpr Type sat_sum() const requires (Integral<T>)
  {
    Type sum = 0;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      sum = ash::sat_add(sum, v[i]);
    }
    return sum;
  }

  constexpr Type sat_product() const requires (Integral<T>)
  {
    Type product = 1;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      product = ash::sat_mul(product, v[i]);
    }
    return product;
  }

  constexpr Type min() const
  {
    Type min_value = v[0];
#pragma unroll
    for (usize i = 1; i < N; i++)
    {
      min_value = ash::min(min_value, v[i]);
    }
    return min_value;
  }

  constexpr Type max() const
  {
    Type max_value = v[0];
#pragma unroll
    for (usize i = 1; i < N; i++)
    {
      max_value = ash::max(max_value, v[i]);
    }
    return max_value;
  }

  constexpr Type dot(vec b) const
  {
    Type dot_product = v[0] * b.v[0];
#pragma unroll
    for (usize i = 1; i < N; i++)
    {
      dot_product += v[i] * b.v[i];
    }
    return dot_product;
  }

  constexpr vec cross(vec b) const requires (Signed<T> && (N == 2 || N == 3))
  {
    if constexpr (N == 2)
    {
      return {x() * b.y() - y() * b.x(), y() * b.x() - x() * b.y()};
    }
    else if constexpr (N == 3)
    {
      return {y() * b.z() - z() * b.y(), z() * b.x() - x() * b.z(),
              x() * b.y() - y() * b.x()};
    }
  }

  constexpr f32 scalar_cross(vec b) const requires (Signed<T> && N == 2)
  {
    return x() * b.y() - y() * b.x();
  }

  constexpr Type length() const requires (FloatingPoint<T>)
  {
    return sqrt(dot(*this));
  }

  constexpr vec clamp(vec min, vec max) const
  {
    vec r;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = ash::clamp(v[i], min.v[i], max.v[i]);
    }
    return r;
  }

  constexpr vec clamp(Type min, Type max) const
  {
    return clamp(splat(min), splat(max));
  }

  constexpr vec min(vec b) const
  {
    vec r;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = ash::min(v[i], b.v[i]);
    }
    return r;
  }

  constexpr vec min(Type b) const
  {
    return min(splat(b));
  }

  constexpr vec max(vec b) const
  {
    vec r;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = ash::max(v[i], b.v[i]);
    }
    return r;
  }

  constexpr vec max(Type b) const
  {
    return max(splat(b));
  }

  constexpr vec abs() const requires (Signed<T>)
  {
    vec r;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = ash::abs(v[i]);
    }
    return r;
  }

  template <typename To>
  explicit constexpr operator vec<To, SIZE>() const
  {
    vec<To, SIZE> r;
#pragma unroll
    for (usize i = 0; i < SIZE; i++)
    {
      r.v[i] = static_cast<To>(v[i]);
    }
    return r;
  }

  template <typename To>
  constexpr vec<To, SIZE> to() const
  {
    return static_cast<vec<To, SIZE>>(*this);
  }

  constexpr Type mips() const requires (Unsigned<T>)
  {
    return ash::mips(max());
  }

  constexpr vec mip(usize level) const requires (Unsigned<T>)
  {
    vec r;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = ash::mip(v[i], level);
    }
    return r;
  }

  constexpr vec<T, N + 1> append(Type n) const
  {
    vec<T, N + 1> r;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = v[i];
    }
    r.v[N] = n;
    return r;
  }

  template <usize N2>
  constexpr vec<T, N + N2> append(vec<T, N2> n) const
  {
    vec<T, N + N2> r;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i] = v[i];
    }
#pragma unroll
    for (usize i = 0; i < N2; i++)
    {
      r.v[i + N] = n.v[i];
    }
    return r;
  }

  constexpr vec<T, N + 1> prepend(Type n) const
  {
    vec<T, N + 1> r;
    r.v[0] = n;
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i + 1] = v[i];
    }
    return r;
  }

  template <usize N2>
  constexpr vec<T, N + N2> prepend(vec<T, N2> n) const
  {
    vec<T, N + N2> r;
    r.v[0] = n.v[0];
#pragma unroll
    for (usize i = 0; i < N; i++)
    {
      r.v[i + 1] = v[i];
    }
#pragma unroll
    for (usize i = 0; i < N2 - 1; i++)
    {
      r.v[i + N + 1] = n.v[i + 1];
    }
    return r;
  }
};

template <typename T, usize N>
constexpr vec<T, N> operator+(typename vec<T, N>::Type a, vec<T, N> b)
{
  return b + a;
}

template <typename T, usize N>
constexpr vec<T, N> operator-(typename vec<T, N>::Type a, vec<T, N> b)
{
  vec<T, N> r;
#pragma unroll
  for (usize i = 0; i < N; i++)
  {
    r.v[i] = a - b.v[i];
  }
  return r;
}

template <typename T, usize N>
constexpr vec<T, N> operator*(typename vec<T, N>::Type a, vec<T, N> b)
{
  return b * a;
}

template <typename T, usize N>
constexpr vec<T, N> operator/(typename vec<T, N>::Type a, vec<T, N> b)
{
  vec<T, N> r;
#pragma unroll
  for (usize i = 0; i < N; i++)
  {
    r.v[i] = a / b.v[i];
  }
  return r;
}

template <typename T, usize R, usize C>
requires (R > 0 && C > 0)
struct mat
{
  using Type = T;

  static constexpr usize NUM_COLS = R;
  static constexpr usize NUM_ROWS = C;

  using Row     = vec<T, NUM_COLS>;
  using Col     = vec<T, NUM_ROWS>;
  using Storage = Row[NUM_ROWS];

  Storage v = {};

  static constexpr mat splat(T v)
  {
    mat r;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      r.v[i] = Row::splat(v);
    }
    return r;
  }

  static constexpr mat diagonal(T v)
  {
    mat r;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
#pragma unroll
      for (usize j = 0; j < NUM_COLS; j++)
      {
        r.v[i][j] = (i == j) ? v : 0;
      }
    }
    return r;
  }

  static constexpr mat identity()
  {
    return diagonal(1);
  }

  static constexpr mat zero()
  {
    return splat(0);
  }

  constexpr Row & row(usize row)
  {
    return v[row];
  }

  constexpr Row const & row(usize row) const
  {
    return v[row];
  }

  constexpr Row & operator[](usize row)
  {
    return this->row(row);
  }

  constexpr Row const & operator[](usize row) const
  {
    return this->row(row);
  }

  constexpr Col col(usize column)
  {
    Col c;

#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c[i] = v[i][column];
    }
    return c;
  }

  constexpr Col col(usize column) const
  {
    Col c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c[i] = v[i][column];
    }
    return c;
  }

  constexpr Col x() const
  {
    return col(0);
  }

  constexpr Col y() const requires (C > 1)
  {
    return col(1);
  }

  constexpr Col z() const requires (C > 2)
  {
    return col(2);
  }

  constexpr Col w() const requires (C > 3)
  {
    return col(3);
  }

  constexpr Col i() const
  {
    return col(0);
  }

  constexpr Col j() const requires (C > 1)
  {
    return col(1);
  }

  constexpr Col k() const requires (C > 2)
  {
    return col(2);
  }

  constexpr Col r() const
  {
    return x();
  }

  constexpr Col g() const requires (C > 1)
  {
    return y();
  }

  constexpr Col b() const requires (C > 2)
  {
    return z();
  }

  constexpr Col a() const requires (C > 3)
  {
    return w();
  }

  constexpr Type & operator[](usize row, usize column)
  {
    return v[row][column];
  }

  constexpr Type const & operator[](usize row, usize column) const
  {
    return v[row][column];
  }

  constexpr bool operator==(mat const & b) const
  {
    bool all_eq = true;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      all_eq &= (v[i] == b.v[i]);
    }
    return all_eq;
  }

  constexpr bool operator!=(mat const & b) const
  {
    bool any_neq = false;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      any_neq |= (v[i] != b.v[i]);
    }
    return any_neq;
  }

  constexpr mat operator+(mat const & b) const
  {
    mat c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c.v[i] = v[i] + b.v[i];
    }
    return c;
  }

  constexpr mat operator+(Type b) const
  {
    mat c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c.v[i] = v[i] + b;
    }
    return c;
  }

  constexpr mat & operator+=(mat const & b)
  {
    *this = *this + b;
    return *this;
  }

  constexpr mat operator-(mat const & b) const
  {
    mat c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c.v[i] = v[i] - b.v[i];
    }
    return c;
  }

  constexpr mat operator-(Type b)
  {
    mat c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c.v[i] = v[i] - b;
    }
    return c;
  }

  constexpr mat & operator-=(mat const & b)
  {
    *this = *this - b;
    return *this;
  }

  constexpr mat operator*(Type b) const
  {
    mat c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c.v[i] = v[i] * b;
    }
    return c;
  }

  constexpr mat & operator*=(Type b)
  {
    *this = *this * b;
    return *this;
  }

  constexpr mat operator/(Type b)
  {
    mat c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c.v[i] = v[i] / b;
    }
    return c;
  }

  constexpr mat & operator/=(Type b)
  {
    *this = *this / b;
    return *this;
  }

  constexpr mat operator*(mat const & b) const requires (R == C)
  {
    mat c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      for (usize j = 0; j < NUM_COLS; j++)
      {
        c.v[i][j] = row(i).dot(b.col(j));
      }
    }
    return c;
  }

  constexpr Row operator*(Col b) const requires (R == C)
  {
    Row c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c.v[i] = row(i).dot(b);
    }
    return c;
  }

  constexpr mat transpose() const requires (R == C)
  {
    mat r;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      for (usize j = 0; j < NUM_COLS; j++)
      {
        r.v[j][i] = v[i][j];
      }
    }
    return r;
  }
};

template <typename T, usize R, usize C>
constexpr mat<T, R, C> operator+(typename mat<T, R, C>::Type a,
                                 mat<T, R, C> const &        b)
{
  return b + a;
}

template <typename T, usize R, usize C>
constexpr mat<T, R, C> operator-(typename mat<T, R, C>::Type a,
                                 mat<T, R, C> const &        b)
{
  mat<T, R, C> r;
#pragma unroll
  for (usize i = 0; i < R; i++)
  {
    for (usize j = 0; j < C; j++)
    {
      r.v[i][j] = a - b.v[i][j];
    }
  }
  return r;
}

template <typename T, usize R, usize C>
constexpr mat<T, R, C> operator*(typename mat<T, R, C>::Type a,
                                 mat<T, R, C> const &        b)
{
  return b * a;
}

template <typename T, usize R, usize C>
constexpr mat<T, R, C> operator/(typename mat<T, R, C>::Type a,
                                 mat<T, R, C> const &        b)
{
  mat<T, R, C> r;
#pragma unroll
  for (usize i = 0; i < R; i++)
  {
    for (usize j = 0; j < C; j++)
    {
      r.v[i][j] = a / b.v[i][j];
    }
  }
  return r;
}

template <typename T, usize R>
requires (R >= 3 && R <= 4)
struct affine
{
  using Type = T;

  static constexpr usize NUM_COLS = R;
  static constexpr usize NUM_ROWS = R - 1;

  using Row     = vec<T, NUM_COLS>;
  using Col     = vec<T, NUM_ROWS>;
  using Storage = Row[NUM_ROWS];

  Storage v = {};

  static constexpr Row trailing()
  {
    Row r;
    r[NUM_COLS - 1] = 1;
    return r;
  }

  static constexpr affine splat(T v)
  {
    affine r;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      r.v[i] = Row::splat(v);
    }
    return r;
  }

  static constexpr affine diagonal(T v)
  {
    affine r;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
#pragma unroll
      for (usize j = 0; j < NUM_COLS; j++)
      {
        r.v[i][j] = (i == j) ? v : 0;
      }
    }
    return r;
  }

  static constexpr affine identity()
  {
    return diagonal(1);
  }

  static constexpr affine zero()
  {
    return splat(0);
  }

  constexpr Row & row(usize row)
  {
    return v[row];
  }

  constexpr Row const & row(usize row) const
  {
    return v[row];
  }

  constexpr Row & operator[](usize row)
  {
    return this->row(row);
  }

  constexpr Row const & operator[](usize row) const
  {
    return this->row(row);
  }

  constexpr Col col(usize column) const
  {
    Col c;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      c[i] = v[i][column];
    }
    return c;
  }

  constexpr Col x() const
  {
    return col(0);
  }

  constexpr Col y() const
  {
    return col(1);
  }

  constexpr Col z() const
  {
    return col(2);
  }

  constexpr Col w() const requires (R > 3)
  {
    return col(3);
  }

  constexpr Col i() const
  {
    return x();
  }

  constexpr Col j() const
  {
    return y();
  }

  constexpr Col k() const
  {
    return z();
  }

  constexpr Row x_ext() const
  {
    return col(0).append(0);
  }

  constexpr Row y_ext() const
  {
    return col(1).append(0);
  }

  constexpr Row z_ext() const
  {
    return col(2).append(R == 3 ? 1 : 0);
  }

  constexpr Row w_ext() const requires (R > 3)
  {
    return col(3).append(1);
  }

  constexpr Row i_ext() const
  {
    return x_ext();
  }

  constexpr Row j_ext() const
  {
    return y_ext();
  }

  constexpr Row k_ext() const
  {
    return z_ext();
  }

  constexpr bool operator==(affine const & b) const
  {
    bool all_eq = true;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      all_eq &= (v[i] == b.v[i]);
    }
    return all_eq;
  }

  constexpr bool operator!=(affine const & b) const
  {
    bool any_neq = false;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      any_neq |= (v[i] != b.v[i]);
    }
    return any_neq;
  }

  constexpr affine operator*(affine const & b) const
  {
    auto   m = to_mat() * b.to_mat();
    affine r;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS - 1; i++)
    {
      r[i] = m[i];
    }
    return r;
  }

  constexpr mat<T, R, R> operator*(mat<T, R, R> const & b) const
  {
    return to_mat() * b;
  }

  constexpr Row operator*(Row b) const
  {
    return to_mat() * b;
  }

  explicit constexpr operator mat<T, R, R>() const
  {
    mat<T, R, R> r;
#pragma unroll
    for (usize i = 0; i < NUM_ROWS; i++)
    {
      r.v[i] = v[i];
    }
    r.v[NUM_ROWS] = trailing();
    return r;
  }

  constexpr mat<T, R, R> to_mat() const
  {
    return static_cast<mat<T, R, R>>(*this);
  }
};

template <typename T, usize R>
constexpr mat<T, R, R> operator*(mat<T, R, R> const & a, affine<T, R> const & b)
{
  return a * b.to_mat();
}

}    // namespace math

using i8x1 = math::vec<i8, 1>;
using i8x2 = math::vec<i8, 2>;
using i8x3 = math::vec<i8, 3>;
using i8x4 = math::vec<i8, 4>;

using u8x1 = math::vec<u8, 1>;
using u8x2 = math::vec<u8, 2>;
using u8x3 = math::vec<u8, 3>;
using u8x4 = math::vec<u8, 4>;

using i16x1 = math::vec<i16, 1>;
using i16x2 = math::vec<i16, 2>;
using i16x3 = math::vec<i16, 3>;
using i16x4 = math::vec<i16, 4>;

using u16x1 = math::vec<u16, 1>;
using u16x2 = math::vec<u16, 2>;
using u16x3 = math::vec<u16, 3>;
using u16x4 = math::vec<u16, 4>;

using i32x1 = math::vec<i32, 1>;
using i32x2 = math::vec<i32, 2>;
using i32x3 = math::vec<i32, 3>;
using i32x4 = math::vec<i32, 4>;

using u32x1 = math::vec<u32, 1>;
using u32x2 = math::vec<u32, 2>;
using u32x3 = math::vec<u32, 3>;
using u32x4 = math::vec<u32, 4>;

using i64x1 = math::vec<i64, 1>;
using i64x2 = math::vec<i64, 2>;
using i64x3 = math::vec<i64, 3>;
using i64x4 = math::vec<i64, 4>;

using u64x1 = math::vec<u64, 1>;
using u64x2 = math::vec<u64, 2>;
using u64x3 = math::vec<u64, 3>;
using u64x4 = math::vec<u64, 4>;

using f8x1 = math::vec<f8, 1>;
using f8x2 = math::vec<f8, 2>;
using f8x3 = math::vec<f8, 3>;
using f8x4 = math::vec<f8, 4>;

using f16x1 = math::vec<f16, 1>;
using f16x2 = math::vec<f16, 2>;
using f16x3 = math::vec<f16, 3>;
using f16x4 = math::vec<f16, 4>;

using f32x1 = math::vec<f32, 1>;
using f32x2 = math::vec<f32, 2>;
using f32x3 = math::vec<f32, 3>;
using f32x4 = math::vec<f32, 4>;

using f64x1 = math::vec<f64, 1>;
using f64x2 = math::vec<f64, 2>;
using f64x3 = math::vec<f64, 3>;
using f64x4 = math::vec<f64, 4>;

using f16x1x1 = math::mat<f16, 1, 1>;
using f16x2x2 = math::mat<f16, 2, 2>;
using f16x3x3 = math::mat<f16, 3, 3>;
using f16x4x4 = math::mat<f16, 4, 4>;

using f32x1x1 = math::mat<f32, 1, 1>;
using f32x2x2 = math::mat<f32, 2, 2>;
using f32x3x3 = math::mat<f32, 3, 3>;
using f32x4x4 = math::mat<f32, 4, 4>;

using f64x1x1 = math::mat<f64, 1, 1>;
using f64x2x2 = math::mat<f64, 2, 2>;
using f64x3x3 = math::mat<f64, 3, 3>;
using f64x4x4 = math::mat<f64, 4, 4>;

using affinef16x3 = math::affine<f16, 3>;
using affinef16x4 = math::affine<f16, 4>;

using affinef32x3 = math::affine<f32, 3>;
using affinef32x4 = math::affine<f32, 4>;

using affinef64x3 = math::affine<f64, 3>;
using affinef64x4 = math::affine<f64, 4>;

constexpr f32x4 norm(u8x4 color)
{
  constexpr f32 SCALE = 1 / 255.0F;
  return static_cast<f32x4>(color) * f32x4::splat(SCALE);
}

constexpr f32x2 normalize(f32x2 a)
{
  return a * invsqrt(a.dot(a));
}

constexpr f32x3 normalize(f32x3 a)
{
  return a * invsqrt(a.dot(a));
}

constexpr f32x4 normalize(f32x4 a)
{
  return a * invsqrt(a.dot(a));
}

constexpr f32 determinant(f32x2x2 const & a)
{
  return a[0].x() * a[1].y() - a[1].x() * a[0].y();
}

constexpr f32 determinant(f32x3x3 const & a)
{
  return a[0].x() * a[1].y() * a[2].z() - a[0].x() * a[1].z() * a[2].y() -
         a[0].y() * a[1].x() * a[2].z() + a[0].y() * a[1].z() * a[2].x() +
         a[0].z() * a[1].x() * a[2].y() - a[0].z() * a[1].y() * a[2].x();
}

constexpr f32 determinant(f32x4x4 const & a)
{
  return a[0].x() *
           (a[1].y() * a[2].z() * a[3].w() + a[1].z() * a[2].w() * a[3].y() +
            a[1].w() * a[2].y() * a[3].z() - a[1].w() * a[2].z() * a[3].y() -
            a[1].z() * a[2].y() * a[3].w() - a[1].y() * a[2].w() * a[3].z()) -
         a[1].x() *
           (a[0].y() * a[2].z() * a[3].w() + a[0].z() * a[2].w() * a[3].y() +
            a[0].w() * a[2].y() * a[3].z() - a[0].w() * a[2].z() * a[3].y() -
            a[0].z() * a[2].y() * a[3].w() - a[0].y() * a[2].w() * a[3].z()) +
         a[2].x() *
           (a[0].y() * a[1].z() * a[3].w() + a[0].z() * a[1].w() * a[3].y() +
            a[0].w() * a[1].y() * a[3].z() - a[0].w() * a[1].z() * a[3].y() -
            a[0].z() * a[1].y() * a[3].w() - a[0].y() * a[1].w() * a[3].z()) -
         a[3].x() *
           (a[0].y() * a[1].z() * a[2].w() + a[0].z() * a[1].w() * a[2].y() +
            a[0].w() * a[1].y() * a[2].z() - a[0].w() * a[1].z() * a[2].y() -
            a[0].z() * a[1].y() * a[2].w() - a[0].y() * a[1].w() * a[2].z());
}

constexpr f32x2x2 adjoint(f32x2x2 const & a)
{
  return f32x2x2{
    {{a[1].y(), -a[0].y()}, {-a[1].x(), a[0].x()}}
  };
}

constexpr f32x3x3 adjoint(f32x3x3 const & a)
{
  return f32x3x3{
    {{a[1].y() * a[2].z() - a[1].z() * a[2].y(),
      a[0].z() * a[2].y() - a[0].y() * a[2].z(),
      a[0].y() * a[1].z() - a[0].z() * a[1].y()},
     {a[1].z() * a[2].x() - a[1].x() * a[2].z(),
      a[0].x() * a[2].z() - a[0].z() * a[2].x(),
      a[0].z() * a[1].x() - a[0].x() * a[1].z()},
     {a[1].x() * a[2].y() - a[1].y() * a[2].x(),
      a[0].y() * a[2].x() - a[0].x() * a[2].y(),
      a[0].x() * a[1].y() - a[0].y() * a[1].x()}}
  };
}

constexpr f32x4x4 adjoint(f32x4x4 const & a)
{
  f32x4x4 r;
  r[0].x() = a[1].y() * a[2].z() * a[3].w() + a[1].z() * a[2].w() * a[3].y() +
             a[1].w() * a[2].y() * a[3].z() - a[1].w() * a[2].z() * a[3].y() -
             a[1].z() * a[2].y() * a[3].w() - a[1].y() * a[2].w() * a[3].z();
  r[0].y() = -a[0].y() * a[2].z() * a[3].w() - a[0].z() * a[2].w() * a[3].y() -
             a[0].w() * a[2].y() * a[3].z() + a[0].w() * a[2].z() * a[3].y() +
             a[0].z() * a[2].y() * a[3].w() + a[0].y() * a[2].w() * a[3].z();
  r[0].z() = a[0].y() * a[1].z() * a[3].w() + a[0].z() * a[1].w() * a[3].y() +
             a[0].w() * a[1].y() * a[3].z() - a[0].w() * a[1].z() * a[3].y() -
             a[0].z() * a[1].y() * a[3].w() - a[0].y() * a[1].w() * a[3].z();
  r[0].w() = -a[0].y() * a[1].z() * a[2].w() - a[0].z() * a[1].w() * a[2].y() -
             a[0].w() * a[1].y() * a[2].z() + a[0].w() * a[1].z() * a[2].y() +
             a[0].z() * a[1].y() * a[2].w() + a[0].y() * a[1].w() * a[2].z();
  r[1].x() = -a[1].x() * a[2].z() * a[3].w() - a[1].z() * a[2].w() * a[3].x() -
             a[1].w() * a[2].x() * a[3].z() + a[1].w() * a[2].z() * a[3].x() +
             a[1].z() * a[2].x() * a[3].w() + a[1].x() * a[2].w() * a[3].z();
  r[1].y() = a[0].x() * a[2].z() * a[3].w() + a[0].z() * a[2].w() * a[3].x() +
             a[0].w() * a[2].x() * a[3].z() - a[0].w() * a[2].z() * a[3].x() -
             a[0].z() * a[2].x() * a[3].w() - a[0].x() * a[2].w() * a[3].z();
  r[1].z() = -a[0].x() * a[1].z() * a[3].w() - a[0].z() * a[1].w() * a[3].x() -
             a[0].w() * a[1].x() * a[3].z() + a[0].w() * a[1].z() * a[3].x() +
             a[0].z() * a[1].x() * a[3].w() + a[0].x() * a[1].w() * a[3].z();
  r[1].w() = a[0].x() * a[1].z() * a[2].w() + a[0].z() * a[1].w() * a[2].x() +
             a[0].w() * a[1].x() * a[2].z() - a[0].w() * a[1].z() * a[2].x() -
             a[0].z() * a[1].x() * a[2].w() - a[0].x() * a[1].w() * a[2].z();
  r[2].x() = a[1].x() * a[2].y() * a[3].w() + a[1].y() * a[2].w() * a[3].x() +
             a[1].w() * a[2].x() * a[3].y() - a[1].w() * a[2].y() * a[3].x() -
             a[1].y() * a[2].x() * a[3].w() - a[1].x() * a[2].w() * a[3].y();
  r[2].y() = -a[0].x() * a[2].y() * a[3].w() - a[0].y() * a[2].w() * a[3].x() -
             a[0].w() * a[2].x() * a[3].y() + a[0].w() * a[2].y() * a[3].x() +
             a[0].y() * a[2].x() * a[3].w() + a[0].x() * a[2].w() * a[3].y();
  r[2].z() = a[0].x() * a[1].y() * a[3].w() + a[0].y() * a[1].w() * a[3].x() +
             a[0].w() * a[1].x() * a[3].y() - a[0].w() * a[1].y() * a[3].x() -
             a[0].y() * a[1].x() * a[3].w() - a[0].x() * a[1].w() * a[3].y();
  r[2].w() = -a[0].x() * a[1].y() * a[2].w() - a[0].y() * a[1].w() * a[2].x() -
             a[0].w() * a[1].x() * a[2].y() + a[0].w() * a[1].y() * a[2].x() +
             a[0].y() * a[1].x() * a[2].w() + a[0].x() * a[1].w() * a[2].y();
  r[3].x() = -a[1].x() * a[2].y() * a[3].z() - a[1].y() * a[2].z() * a[3].x() -
             a[1].z() * a[2].x() * a[3].y() + a[1].z() * a[2].y() * a[3].x() +
             a[1].y() * a[2].x() * a[3].z() + a[1].x() * a[2].z() * a[3].y();
  r[3].y() = a[0].x() * a[2].y() * a[3].z() + a[0].y() * a[2].z() * a[3].x() +
             a[0].z() * a[2].x() * a[3].y() - a[0].z() * a[2].y() * a[3].x() -
             a[0].y() * a[2].x() * a[3].z() - a[0].x() * a[2].z() * a[3].y();
  r[3].z() = -a[0].x() * a[1].y() * a[3].z() - a[0].y() * a[1].z() * a[3].x() -
             a[0].z() * a[1].x() * a[3].y() + a[0].z() * a[1].y() * a[3].x() +
             a[0].y() * a[1].x() * a[3].z() + a[0].x() * a[1].z() * a[3].y();
  r[3].w() = a[0].x() * a[1].y() * a[2].z() + a[0].y() * a[1].z() * a[2].x() +
             a[0].z() * a[1].x() * a[2].y() - a[0].z() * a[1].y() * a[2].x() -
             a[0].y() * a[1].x() * a[2].z() - a[0].x() * a[1].z() * a[2].y();
  return r;
}

constexpr f32x2x2 inverse(f32x2x2 a)
{
  return (1.0F / determinant(a)) * adjoint(a);
}

constexpr f32x3x3 inverse(f32x3x3 const & a)
{
  return (1.0F / determinant(a)) * adjoint(a);
}

constexpr f32x4x4 inverse(f32x4x4 const & a)
{
  return (1.0F / determinant(a)) * adjoint(a);
}

constexpr affinef32x3 translate2d(f32x2 t)
{
  return affinef32x3{
    {{1, 0, t.x()}, {0, 1, t.y()}}
  };
}

constexpr affinef32x4 translate3d(f32x3 t)
{
  return affinef32x4{
    {{1, 0, 0, t.x()}, {0, 1, 0, t.y()}, {0, 0, 1, t.z()}}
  };
}

constexpr affinef32x3 scale2d(f32x2 s)
{
  return affinef32x3{
    {{s.x(), 0, 0}, {0, s.y(), 0}}
  };
}

constexpr affinef32x4 scale3d(f32x3 s)
{
  return affinef32x4{
    {{s.x(), 0, 0, 0}, {0, s.y(), 0, 0}, {0, 0, s.z(), 0}}
  };
}

inline affinef32x3 rotate2d(f32 radians)
{
  return affinef32x3{
    {{cos(radians), -sin(radians), 0}, {sin(radians), cos(radians), 0}}
  };
}

inline affinef32x4 rotate3d_x(f32 radians)
{
  return affinef32x4{
    {{1, 0, 0, 0},
     {0, cos(radians), -sin(radians), 0},
     {0, sin(radians), cos(radians), 0}}
  };
}

inline affinef32x4 rotate3d_y(f32 radians)
{
  return affinef32x4{
    {{cos(radians), 0, sin(radians), 0},
     {0, 1, 0, 0},
     {-sin(radians), 0, cos(radians), 0}}
  };
}

inline affinef32x4 rotate3d_z(f32 radians)
{
  return affinef32x4{
    {{cos(radians), -sin(radians), 0, 0},
     {sin(radians), cos(radians), 0, 0},
     {0, 0, 1, 0}}
  };
}

constexpr f32x2 transform(f32x3x3 const & t, f32x2 value)
{
  return (t * (f32x3{value.x(), value.y(), 1})).xy();
}

constexpr f32x2 transform(affinef32x3 const & t, f32x2 value)
{
  return (t * (f32x3{value.x(), value.y(), 1})).xy();
}

constexpr f32x3 transform(f32x4x4 const & t, f32x3 value)
{
  return (t * (f32x4{value.x(), value.y(), value.z(), 1})).xyz();
}

constexpr f32x3 transform(affinef32x4 const & t, f32x3 value)
{
  return (t * (f32x4{value.x(), value.y(), 1})).xyz();
}

inline f32x2 sin(f32x2 v)
{
  return f32x2{sin(v.x()), sin(v.y())};
}

inline f32x3 sin(f32x3 v)
{
  return f32x3{sin(v.x()), sin(v.y()), sin(v.z())};
}

inline f32x4 sin(f32x4 v)
{
  return f32x4{sin(v.x()), sin(v.y()), sin(v.z()), sin(v.w())};
}

inline f32x2 cos(f32x2 v)
{
  return f32x2{cos(v.x()), cos(v.y())};
}

inline f32x3 cos(f32x3 v)
{
  return f32x3{cos(v.x()), cos(v.y()), cos(v.z())};
}

inline f32x4 cos(f32x4 v)
{
  return f32x4{cos(v.x()), cos(v.y()), cos(v.z()), cos(v.w())};
}

inline f32x2 tan(f32x2 v)
{
  return f32x2{tan(v.x()), tan(v.y())};
}

inline f32x3 tan(f32x3 v)
{
  return f32x3{tan(v.x()), tan(v.y()), tan(v.z())};
}

inline f32x4 tan(f32x4 v)
{
  return f32x4{tan(v.x()), tan(v.y()), tan(v.z()), tan(v.w())};
}

inline f32x2 exp(f32x2 v)
{
  return f32x2{exp(v.x()), exp(v.y())};
}

inline f32x3 exp(f32x3 v)
{
  return f32x3{exp(v.x()), exp(v.y()), exp(v.z())};
}

inline f32x4 exp(f32x4 v)
{
  return f32x4{exp(v.x()), exp(v.y()), exp(v.z()), exp(v.w())};
}

inline f32x2 exp2(f32x2 v)
{
  return f32x2{exp2(v.x()), exp2(v.y())};
}

inline f32x3 exp2(f32x3 v)
{
  return f32x3{exp2(v.x()), exp2(v.y()), exp2(v.z())};
}

inline f32x4 exp2(f32x4 v)
{
  return f32x4{exp2(v.x()), exp2(v.y()), exp2(v.z()), exp2(v.w())};
}

inline f32x2 log(f32x2 v)
{
  return f32x2{log(v.x()), log(v.y())};
}

inline f32x3 log(f32x3 v)
{
  return f32x3{log(v.x()), log(v.y()), log(v.z())};
}

inline f32x4 log(f32x4 v)
{
  return f32x4{log(v.x()), log(v.y()), log(v.z()), log(v.w())};
}

inline f32x2 floor(f32x2 v)
{
  return f32x2{floor(v.x()), floor(v.y())};
}

inline f32x3 floor(f32x3 v)
{
  return f32x3{floor(v.x()), floor(v.y()), floor(v.z())};
}

inline f32x4 floor(f32x4 v)
{
  return f32x4{floor(v.x()), floor(v.y()), floor(v.z()), floor(v.w())};
}

inline f32x2 rotor(f32 a)
{
  return f32x2{cos(a), sin(a)};
}

inline constexpr affinef32x4 transform2d_to_3d(affinef32x3 const & mat)
{
  return affinef32x4{
    {{mat.v[0][0], mat.v[0][1], 0, mat.v[0][2]},
     {mat.v[1][0], mat.v[1][1], 0, mat.v[1][2]},
     {0.0F, 0.0F, 1.0F, 0.0F}}
  };
}

inline constexpr affinef32x3 transform3d_to_2d(affinef32x4 const & mat)
{
  return affinef32x3{
    {{mat.v[0][0], mat.v[0][1], mat.v[0][3]},
     {mat.v[1][0], mat.v[1][1], mat.v[1][3]}}
  };
}

inline constexpr f32x3x3 transform3d_to_2d(f32x4x4 const & mat)
{
  return f32x3x3{
    {{mat.v[0][0], mat.v[0][1], mat.v[0][3]},
     {mat.v[1][0], mat.v[1][1], mat.v[1][3]},
     {mat.v[2][0], mat.v[2][1], mat.v[2][3]}}
  };
}

inline constexpr f32 ALIGNMENT_LEFT   = -0.5F;
inline constexpr f32 ALIGNMENT_RIGHT  = 0.5F;
inline constexpr f32 ALIGNMENT_TOP    = -0.5F;
inline constexpr f32 ALIGNMENT_BOTTOM = 0.5F;
inline constexpr f32 ALIGNMENT_CENTER = 0;

inline constexpr f32x2 ALIGNMENT_CENTER_CENTER{0, 0};
inline constexpr f32x2 ALIGNMENT_TOP_LEFT{-0.5F, -0.5F};
inline constexpr f32x2 ALIGNMENT_TOP_CENTER{0, -0.5F};
inline constexpr f32x2 ALIGNMENT_TOP_RIGHT{0.5F, -0.5F};
inline constexpr f32x2 ALIGNMENT_BOTTOM_LEFT{-0.5F, 0.5F};
inline constexpr f32x2 ALIGNMENT_BOTTOM_CENTER{0, 0.5F};
inline constexpr f32x2 ALIGNMENT_BOTTOM_RIGHT{0.5F, 0.5F};
inline constexpr f32x2 ALIGNMENT_LEFT_CENTER{-0.5F, 0};
inline constexpr f32x2 ALIGNMENT_RIGHT_CENTER{0.5F, 0};

constexpr f32x4 opacity(f32 v)
{
  return f32x4{1, 1, 1, v};
}

constexpr f32x4 opacity_premul(f32 v)
{
  return f32x4::splat(v);
}

constexpr bool overlaps(f32x2 a_begin, f32x2 a_end, f32x2 b_begin, f32x2 b_end)
{
  return a_begin.x() <= b_end.x() && a_end.x() >= b_begin.x() &&
         a_begin.y() <= b_end.y() && a_end.y() >= b_begin.y();
}

constexpr bool contains_point(f32x2 begin, f32x2 end, f32x2 point)
{
  return begin.x() <= point.x() && begin.y() <= point.y() &&
         end.x() >= point.x() && end.y() >= point.y();
}

constexpr Tuple<f32x2, f32x2> intersect(f32x2 a_begin, f32x2 a_end,
                                        f32x2 b_begin, f32x2 b_end)
{
  if (!overlaps(a_begin, a_end, b_begin, b_end))
  {
    return {};
  }

  return {
    f32x2{max(a_begin.x(), b_begin.x()), max(a_begin.y(), b_begin.y())},
    f32x2{min(a_end.x(),   b_end.x()),   min(a_end.y(),   b_end.y())  }
  };
}

struct CRect;

struct Rect
{
  f32x2 offset = {};
  f32x2 extent = {};

  static constexpr Rect from_center(f32x2 center, f32x2 extent)
  {
    return Rect{.offset = center - extent * 0.5F, .extent = extent};
  }

  static constexpr Rect range(f32x2 begin, f32x2 end)
  {
    return Rect{.offset = begin, .extent = end - begin};
  }

  constexpr f32x2 center() const
  {
    return offset + (extent * 0.5F);
  }

  constexpr f32x2 begin() const
  {
    return offset;
  }

  constexpr f32x2 end() const
  {
    return offset + extent;
  }

  constexpr f32 area() const
  {
    return extent.product();
  }

  constexpr CRect centered() const;

  constexpr bool is_visible() const
  {
    return !extent.any_zero();
  }

  constexpr bool contains(f32x2 point) const
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
  f32x2 center = {};
  f32x2 extent = {};

  static constexpr auto from_offset(f32x2 offset, f32x2 extent)
  {
    return CRect{.center = offset + extent * 0.5F, .extent = extent};
  }

  static constexpr auto range(f32x2 begin, f32x2 end)
  {
    return CRect{.center = (begin + end) * 0.5F, .extent = (end - begin)};
  }

  static constexpr auto bounding(f32x2 p0, f32x2 p1, f32x2 p2, f32x2 p3)
  {
    return CRect::range(f32x2{min(p0.x(), p1.x(), p2.x(), p3.x()),
                              min(p0.y(), p1.y(), p2.y(), p3.y())},
                        f32x2{max(p0.x(), p1.x(), p2.x(), p3.x()),
                              max(p0.y(), p1.y(), p2.y(), p3.y())});
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
    return f32x2{center.x() + 0.5F * extent.x(),
                 center.y() - 0.5F * extent.y()};
  }

  constexpr auto bl() const
  {
    return f32x2{center.x() - 0.5F * extent.x(),
                 center.y() + 0.5F * extent.y()};
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
    return extent.x() * extent.y();
  }

  constexpr Rect offseted() const;

  constexpr bool is_visible() const
  {
    return extent.x() != 0 & extent.y() != 0;
  }

  constexpr bool contains(f32x2 point) const
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

  constexpr auto transform(affinef32x3 const & t)
  {
    return CRect::bounding(ash::transform(t, tl()), ash::transform(t, tr()),
                           ash::transform(t, bl()), ash::transform(t, br()));
  }

  constexpr auto transform(f32x3x3 const & t)
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
  u32x2 offset = {};
  u32x2 extent = {};

  static constexpr RectU range(u32x2 begin, u32x2 end)
  {
    return RectU{.offset = begin, .extent = (end - begin)};
  }

  constexpr u32x2 begin() const
  {
    return offset;
  }

  constexpr u32x2 end() const
  {
    return offset + extent;
  }

  constexpr bool is_visible() const
  {
    return !extent.any_zero();
  }

  constexpr RectU clamp_to_extent(u32x2 base_extent)
  {
    auto max_offset = base_extent.sat_sub({1U, 1U});
    auto end        = this->end();
    auto new_begin  = offset.min(max_offset);
    auto new_end    = base_extent.min(end);

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
  i32x2 offset = {};
  i32x2 extent = {};

  constexpr i32x2 begin() const
  {
    return offset;
  }

  constexpr i32x2 end() const
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

struct CBox;

struct Box
{
  f32x3 offset = {};
  f32x3 extent = {};

  static constexpr Box from_center(f32x3 center, f32x3 extent)
  {
    return Box{.offset = center - extent * 0.5F, .extent = extent};
  }

  constexpr f32x3 center() const
  {
    return offset + (extent * 0.5F);
  }

  constexpr f32x3 end() const
  {
    return offset + extent;
  }

  constexpr f32 volume() const
  {
    return extent.x() * extent.y() * extent.z();
  }

  constexpr CBox centered() const;

  constexpr bool is_visible() const
  {
    return extent.x() != 0 | extent.y() != 0 | extent.z() != 0;
  }

  constexpr bool contains(f32x3 point) const
  {
    return offset.x() <= point.x() && offset.y() <= point.y() &&
           offset.z() <= point.z() && (offset.x() + extent.x()) >= point.x() &&
           (offset.y() + extent.y()) >= point.y() &&
           (offset.z() + extent.z()) >= point.z();
    return true;
  }

  constexpr bool overlaps(Box const & b) const
  {
    f32x3 a_begin = offset;
    f32x3 a_end   = offset + extent;
    f32x3 b_begin = b.offset;
    f32x3 b_end   = b.offset + b.extent;
    return a_begin.x() <= b_end.x() && a_end.x() >= b_begin.x() &&
           a_begin.y() <= b_end.y() && a_end.y() >= b_begin.y() &&
           a_begin.z() <= b_end.z() && a_end.z() >= b_begin.z();
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
  u32x3 offset = {};
  u32x3 extent = {};

  constexpr u32x3 begin() const
  {
    return offset;
  }

  constexpr u32x3 end() const
  {
    return offset + extent;
  }
};

constexpr BoxU as_boxu(RectU const & r)
{
  return BoxU{.offset = r.offset.append(0), .extent = r.extent.append(1)};
}

struct CBox
{
  f32x3 center = {};
  f32x3 extent = {};

  static constexpr CBox from_offset(f32x3 offset, f32x3 extent)
  {
    return CBox{.center = offset + extent * 0.5F, .extent = extent};
  }

  constexpr f32x3 begin() const
  {
    return center - extent * 0.5F;
  }

  constexpr f32x3 end() const
  {
    return center + extent * 0.5F;
  }

  constexpr f32 volume() const
  {
    return extent.x() * extent.y() * extent.z();
  }

  constexpr Box offseted() const;

  constexpr bool is_visible() const
  {
    return extent.x() != 0 & extent.y() != 0 & extent.z() != 0;
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

/// @brief Find the maximum extent that will fit in the provided extent while
/// respecting the provided aspect ratio
constexpr f32x2 with_aspect(f32x2 extent, f32 aspect_ratio)
{
  f32 const   base          = min(extent.x(), extent.y());
  f32x2 const width_scaled  = f32x2{base * aspect_ratio, base};
  f32x2 const height_scaled = f32x2{base, base / aspect_ratio};

  if (width_scaled.x() <= extent.x())
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
constexpr affinef32x4 orthographic(f32 x_mag, f32 y_mag, f32 z_near, f32 z_far)
{
  f32 const z_diff     = z_near - z_far;
  f32 const z_diff_inv = 1 / z_diff;
  return affinef32x4{
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
inline f32x4x4 perspective(f32 aspect_ratio, f32 y_fov, f32 z_far, f32 z_near)
{
  f32 const s          = tanf(y_fov * 0.5F);
  f32 const z_diff     = z_near - z_far;
  f32 const z_diff_inv = 1 / z_diff;
  return f32x4x4{
    {{1 / (aspect_ratio * s), 0, 0, 0},
     {0, 1 / s, 0, 0},
     {0, 0, (z_far + z_near) * z_diff_inv, 2 * z_far * z_near * z_diff_inv},
     {0, 0, -1, 0}}
  };
}

constexpr f32x4x4 look_at(f32x3 eye, f32x3 center, f32x3 up)
{
  f32x3 const f = normalize(center - eye);
  f32x3 const s = normalize(up.cross(f));
  f32x3 const u = f.cross(s);

  return {
    {{s.x(), s.x(), s.x(), 0},
     {u.y(), u.y(), u.y(), 0},
     {f.z(), f.z(), f.z(), 0},
     {-s.dot(eye), -u.dot(eye), -f.dot(eye), 1}}
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
constexpr bool is_outside_frustum(f32x4x4 const & mvp, f32x3 offset,
                                  f32x3 extent)
{
  constexpr u32 NUM_CORNERS          = 8;
  f32x4 const   corners[NUM_CORNERS] = {
    mvp * offset.append(1),
    mvp * (offset + f32x3{extent.x(), 0, 0}).append(1),
    mvp * (offset + f32x3{extent.x(), extent.y(), 0}).append(1),
    mvp * (offset + f32x3{0, extent.y(), 0}).append(1),
    mvp * (offset + f32x3{0, 0, extent.z()}).append(1),
    mvp * (offset + f32x3{extent.x(), 0, extent.z()}).append(1),
    mvp * (offset + extent).append(1),
    mvp * (offset + f32x3{0, extent.y(), extent.z()}).append(1)};

  u32 left   = 0;
  u32 right  = 0;
  u32 top    = 0;
  u32 bottom = 0;
  u32 back   = 0;

  for (u32 i = 0; i < NUM_CORNERS; i++)
  {
    f32x4 const & corner = corners[i];

    if (corner.x() < -corner.w())
    {
      left++;
    }

    if (corner.x() > corner.w())
    {
      right++;
    }

    if (corner.y() < -corner.w())
    {
      bottom++;
    }

    if (corner.y() > corner.w())
    {
      top++;
    }

    if (corner.z() < 0)
    {
      back++;
    }
  }

  return left == NUM_CORNERS | right == NUM_CORNERS | top == NUM_CORNERS |
         bottom == NUM_CORNERS | back == NUM_CORNERS;
}

/// @brief Calculate the inverse of a 2d scale and translation-only transformation matrix
constexpr affinef32x3 translate_scale_inv2d(affinef32x3 const & t)
{
  auto sx = 1 / t[0].x();
  auto sy = 1 / t[1].y();
  auto tx = t[0].z();
  auto ty = t[1].z();
  return affinef32x3{
    {{sx, 0, -sx * tx}, {0, sy, -sy * ty}}
  };
}

// [ ] implement for shader?
inline f32x3 refract(f32x3 v, f32x3 n, f32 r)
{
  f32 dot = v.dot(n);
  f32 d   = 1.0F - r * r * (1.0F - dot * dot);

  if (d >= 0.0F)
  {
    d = ash::sqrt(d);
    return r * v - (r * dot + d) * n;
  }

  return f32x3::splat(0);
}

}    // namespace ash
