/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/traits.h"
#include <bit>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <new>
#include <type_traits>
#include <utility>

namespace ash
{

typedef char8_t   c8;
typedef char16_t  c16;
typedef char32_t  c32;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef size_t    usize;
typedef ptrdiff_t isize;
typedef uintptr_t uptr;
typedef intptr_t  iptr;
typedef u8        bool8;
typedef u16       bool16;
typedef u32       bool32;
typedef u64       bool64;
typedef usize     sbool;
typedef float     f32;
typedef double    f64;
typedef u16       hash16;
typedef u32       hash32;
typedef u64       hash64;

inline constexpr u8 U8_MIN = 0;
inline constexpr u8 U8_MAX = 0xFF;

inline constexpr i8 I8_MIN = -0x7F - 1;
inline constexpr i8 I8_MAX = 0x7F;

inline constexpr u16 U16_MIN = 0;
inline constexpr u16 U16_MAX = 0xFFFF;

inline constexpr i16 I16_MIN = -0x7FFF - 1;
inline constexpr i16 I16_MAX = 0x7FFF;

inline constexpr u32 U32_MIN = 0;
inline constexpr u32 U32_MAX = 0xFFFF'FFFFU;

inline constexpr i32 I32_MIN = -0x7FFF'FFFF - 1;
inline constexpr i32 I32_MAX = 0x7FFF'FFFF;

inline constexpr u64 U64_MIN = 0;
inline constexpr u64 U64_MAX = 0xFFFF'FFFF'FFFF'FFFFULL;

inline constexpr i64 I64_MIN = -0x7FFF'FFFF'FFFF'FFFFLL - 1;
inline constexpr i64 I64_MAX = 0x7FFF'FFFF'FFFF'FFFFLL;

inline constexpr usize USIZE_MIN = 0;
inline constexpr usize USIZE_MAX = SIZE_MAX;

inline constexpr isize ISIZE_MIN = PTRDIFF_MIN;
inline constexpr isize ISIZE_MAX = PTRDIFF_MAX;

inline constexpr f32 F32_MIN          = -FLT_MAX;
inline constexpr f32 F32_MIN_POSITIVE = FLT_MIN;
inline constexpr f32 F32_MAX          = FLT_MAX;
inline constexpr f32 F32_EPS          = FLT_EPSILON;
inline constexpr f32 F32_INF          = INFINITY;

inline constexpr f64 F64_MIN          = -DBL_MAX;
inline constexpr f64 F64_MIN_POSITIVE = DBL_MIN;
inline constexpr f64 F64_MAX          = DBL_MAX;
inline constexpr f64 F64_EPS          = DBL_EPSILON;
inline constexpr f64 F64_INF          = INFINITY;

inline constexpr f32 PI = 3.14159265358979323846F;

enum class Ordering : i32
{
  Less    = -1,
  Equal   = 0,
  Greater = 1,
};

constexpr Ordering reverse_ordering(Ordering ordering)
{
  return Ordering{-static_cast<i32>(ordering)};
}

struct Add
{
  constexpr auto operator()(auto const & a, auto const & b) const
  {
    return a + b;
  }
};

struct Sub
{
  constexpr auto operator()(auto const & a, auto const & b) const
  {
    return a - b;
  }
};

struct Mul
{
  constexpr auto operator()(auto const & a, auto const & b) const
  {
    return a * b;
  }
};

struct Div
{
  constexpr auto operator()(auto const & a, auto const & b) const
  {
    return a / b;
  }
};

struct Eq
{
  constexpr bool operator()(auto const & a, auto const & b) const
  {
    return a == b;
  }
};

struct NEq
{
  constexpr bool operator()(auto const & a, auto const & b) const
  {
    return a != b;
  }
};

struct Less
{
  constexpr bool operator()(auto const & a, auto const & b) const
  {
    return a < b;
  }
};

struct LEq
{
  constexpr bool operator()(auto const & a, auto const & b) const
  {
    return a <= b;
  }
};

struct Gt
{
  constexpr bool operator()(auto const & a, auto const & b) const
  {
    return a > b;
  }
};

struct GEq
{
  constexpr bool operator()(auto const & a, auto const & b) const
  {
    return a >= b;
  }
};

struct Cmp
{
  constexpr Ordering operator()(auto const & a, auto const & b) const
  {
    if (a == b)
    {
      return Ordering::Equal;
    }
    if (a > b)
    {
      return Ordering::Greater;
    }
    return Ordering::Less;
  }
};

struct Min
{
  template <typename T>
  constexpr T operator()(T const & a, T const & b) const
  {
    return a < b ? a : b;
  }

  template <typename T, typename... T1>
  constexpr T operator()(T const & a, T const & b, T1 const &... cs) const
  {
    T m = this->template operator()<T>(a, b);
    ((m = this->template operator()<T>(static_cast<T &&>(m), cs)), ...);
    return m;
  }
};

struct Max
{
  template <typename T>
  constexpr T operator()(T const & a, T const & b) const
  {
    return a > b ? a : b;
  }

  template <typename T, typename... T1>
  constexpr T operator()(T const & a, T const & b, T1 const &... cs) const
  {
    T m = this->template operator()<T>(a, b);
    ((m = this->template operator()<T>(static_cast<T &&>(m), cs)), ...);
    return m;
  }
};

struct Swap
{
  template <typename T>
  constexpr void operator()(T & a, T & b) const
  {
    T s{static_cast<T &&>(a)};
    a = static_cast<T &&>(b);
    b = static_cast<T &&>(s);
  }
};

struct Clamp
{
  template <typename T>
  constexpr T operator()(T const & value, T const & min, T const & max) const
  {
    return value < min ? min : (value > max ? max : value);
  }
};

constexpr Add   add;
constexpr Sub   sub;
constexpr Mul   mul;
constexpr Div   div;
constexpr Eq    eq;
constexpr NEq   neq;
constexpr Less  lt;
constexpr LEq   leq;
constexpr Gt    gt;
constexpr GEq   geq;
constexpr Cmp   cmp;
constexpr Min   min;
constexpr Max   max;
constexpr Swap  swap;
constexpr Clamp clamp;

constexpr u8 sat_add(u8 a, u8 b)
{
  return ((a + b) < a) ? U8_MAX : (a + b);
}

constexpr u16 sat_add(u16 a, u16 b)
{
  return ((a + b) < a) ? U16_MAX : (a + b);
}

constexpr u32 sat_add(u32 a, u32 b)
{
  return ((a + b) < a) ? U32_MAX : (a + b);
}

constexpr u64 sat_add(u64 a, u64 b)
{
  return ((a + b) < a) ? U64_MAX : (a + b);
}

constexpr i8 sat_add(i8 a, u8 b)
{
  return (i8) clamp((i16) ((i16) a + (i16) b), (i16) I8_MIN, (i16) I8_MAX);
}

constexpr i16 sat_add(i16 a, i16 b)
{
  return (i16) clamp((i32) a + (i32) b, (i32) I16_MIN, (i32) I16_MAX);
}

constexpr i32 sat_add(i32 a, i32 b)
{
  return (i32) clamp((i64) a + (i64) b, (i64) I32_MIN, (i64) I32_MAX);
}

constexpr i64 sat_add(i64 a, i64 b)
{
  if (a > 0)
  {
    b = I64_MAX - a;
  }
  else if (b < (I64_MIN - a))
  {
    b = I64_MIN - a;
  }

  return a + b;
}

constexpr u8 sat_sub(u8 a, u8 b)
{
  return (u8) clamp((i16) ((i16) a - (i16) b), (i16) U8_MIN, (i16) U8_MAX);
}

constexpr u16 sat_sub(u16 a, u16 b)
{
  return (u16) clamp((i32) ((i32) a - (i32) b), (i32) U16_MIN, (i32) U16_MAX);
}

constexpr u32 sat_sub(u32 a, u32 b)
{
  return (u32) clamp((i64) ((i64) a - (i64) b), (i64) U32_MIN, (i64) U32_MAX);
}

constexpr i8 sat_sub(i8 a, i8 b)
{
  return (i8) clamp((i16) ((i16) a - (i16) b), (i16) I8_MIN, (i16) I8_MAX);
}

constexpr i16 sat_sub(i16 a, i16 b)
{
  return (i16) clamp((i32) ((i32) a - (i32) b), (i32) I16_MIN, (i32) I16_MAX);
}

constexpr i32 sat_sub(i32 a, i32 b)
{
  return (i32) clamp((i64) ((i64) a - (i64) b), (i64) I32_MIN, (i64) I32_MAX);
}

constexpr u8 sat_mul(u8 a, u8 b)
{
  return (u8) min((u16) ((u16) a * (u16) b), (u16) U8_MAX);
}

constexpr u16 sat_mul(u16 a, u16 b)
{
  return (u16) min((u32) a * (u32) b, (u32) U16_MAX);
}

constexpr u32 sat_mul(u32 a, u32 b)
{
  return (u32) min((u64) a * (u64) b, (u64) U32_MAX);
}

constexpr i8 sat_mul(i8 a, i8 b)
{
  return (i8) clamp((i16) ((i16) a * (i16) b), (i16) I8_MIN, (i16) I8_MAX);
}

constexpr i16 sat_mul(i16 a, i16 b)
{
  return (i16) clamp((i32) a * (i32) b, (i32) I16_MIN, (i32) I16_MAX);
}

constexpr i32 sat_mul(i32 a, i32 b)
{
  return (i32) clamp((i64) a * (i64) b, (i64) I32_MIN, (i64) I32_MAX);
}

// [ ] sat_cast

using std::bit_cast;

template <typename T>
constexpr bool has_bits(T src, T cmp)
{
  return (src & cmp) == cmp;
}

template <typename T>
constexpr bool has_any_bit(T src, T cmp)
{
  return (src & cmp) != (T) 0;
}

constexpr bool is_pow2(u8 x)
{
  return (x & (x - 1)) == 0U;
}

constexpr bool is_pow2(u16 x)
{
  return (x & (x - 1)) == 0U;
}

constexpr bool is_pow2(u32 x)
{
  return (x & (x - 1)) == 0U;
}

constexpr bool is_pow2(u64 x)
{
  return (x & (x - 1)) == 0ULL;
}

constexpr bool get_bit(u8 s, usize i)
{
  return (s >> i) & 1;
}

constexpr bool get_bit(u16 s, usize i)
{
  return (s >> i) & 1;
}

constexpr bool get_bit(u32 s, usize i)
{
  return (s >> i) & 1;
}

constexpr bool get_bit(u64 s, usize i)
{
  return (s >> i) & 1;
}

constexpr void clear_bit(u8 & s, usize i)
{
  s &= ~(((u8) 1) << i);
}

constexpr void clear_bit(u16 & s, usize i)
{
  s &= ~(((u16) 1) << i);
}

constexpr void clear_bit(u32 & s, usize i)
{
  s &= ~(((u32) 1) << i);
}

constexpr void clear_bit(u64 & s, usize i)
{
  s &= ~(((u64) 1) << i);
}

constexpr void set_bit(u8 & s, usize i)
{
  s |= (((u8) 1) << i);
}

constexpr void set_bit(u16 & s, usize i)
{
  s |= (((u16) 1) << i);
}

constexpr void set_bit(u32 & s, usize i)
{
  s |= (((u32) 1) << i);
}

constexpr void set_bit(u64 & s, usize i)
{
  s |= (((u64) 1) << i);
}

constexpr void assign_bit(u8 & s, usize i, bool b)
{
  s &= ~(((u8) 1) << i);
  s |= (((u8) b) << i);
}

constexpr void assign_bit(u16 & s, usize i, bool b)
{
  s &= ~(((u16) 1) << i);
  s |= (((u16) b) << i);
}

constexpr void assign_bit(u32 & s, usize i, bool b)
{
  s &= ~(((u32) 1) << i);
  s |= (((u32) b) << i);
}

constexpr void assign_bit(u64 & s, usize i, bool b)
{
  s &= ~(((u64) 1) << i);
  s |= (((u64) b) << i);
}

constexpr void flip_bit(u8 & s, usize i)
{
  s = s ^ (((usize) 1) << i);
}

constexpr void flip_bit(u16 & s, usize i)
{
  s = s ^ (((usize) 1) << i);
}

constexpr void flip_bit(u32 & s, usize i)
{
  s = s ^ (((usize) 1) << i);
}

constexpr void flip_bit(u64 & s, usize i)
{
  s = s ^ (((usize) 1) << i);
}

constexpr unsigned long long operator""_KB(unsigned long long x)
{
  return x << 10;
}

constexpr unsigned long long operator""_MB(unsigned long long x)
{
  return x << 20;
}

constexpr unsigned long long operator""_GB(unsigned long long x)
{
  return x << 30;
}

constexpr unsigned long long operator""_TB(unsigned long long x)
{
  return x << 40;
}

template <typename T>
struct NumTraits;

template <>
struct NumTraits<u8>
{
  static constexpr u8   NUM_BITS       = 8;
  static constexpr u8   LOG2_NUM_BITS  = 3;
  static constexpr u8   MIN            = U8_MIN;
  static constexpr u8   MAX            = U8_MAX;
  static constexpr bool SIGNED         = false;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<u16>
{
  static constexpr u8   NUM_BITS       = 16;
  static constexpr u8   LOG2_NUM_BITS  = 4;
  static constexpr u16  MIN            = U16_MIN;
  static constexpr u16  MAX            = U16_MAX;
  static constexpr bool SIGNED         = false;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<u32>
{
  static constexpr u8   NUM_BITS       = 32;
  static constexpr u8   LOG2_NUM_BITS  = 5;
  static constexpr u32  MIN            = U32_MIN;
  static constexpr u32  MAX            = U32_MAX;
  static constexpr bool SIGNED         = false;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<u64>
{
  static constexpr u8   NUM_BITS       = 64;
  static constexpr u8   LOG2_NUM_BITS  = 6;
  static constexpr u64  MIN            = U64_MIN;
  static constexpr u64  MAX            = U64_MAX;
  static constexpr bool SIGNED         = false;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i8>
{
  static constexpr u8   NUM_BITS       = 8;
  static constexpr u8   LOG2_NUM_BITS  = 3;
  static constexpr i8   MIN            = I8_MIN;
  static constexpr i8   MAX            = I8_MAX;
  static constexpr bool SIGNED         = true;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i16>
{
  static constexpr u8   NUM_BITS       = 16;
  static constexpr u8   LOG2_NUM_BITS  = 4;
  static constexpr i16  MIN            = I16_MIN;
  static constexpr i16  MAX            = I16_MAX;
  static constexpr bool SIGNED         = true;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i32>
{
  static constexpr u8   NUM_BITS       = 32;
  static constexpr u8   LOG2_NUM_BITS  = 5;
  static constexpr i32  MIN            = I32_MIN;
  static constexpr i32  MAX            = I32_MAX;
  static constexpr bool SIGNED         = true;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<i64>
{
  static constexpr u8   NUM_BITS       = 64;
  static constexpr u8   LOG2_NUM_BITS  = 6;
  static constexpr i64  MIN            = I64_MIN;
  static constexpr i64  MAX            = I64_MAX;
  static constexpr bool SIGNED         = true;
  static constexpr bool FLOATING_POINT = false;
};

template <>
struct NumTraits<f32>
{
  static constexpr u8   NUM_BITS       = 32;
  static constexpr u8   LOG2_NUM_BITS  = 5;
  static constexpr f32  MIN            = F32_MIN;
  static constexpr f32  MAX            = F32_MAX;
  static constexpr bool SIGNED         = true;
  static constexpr bool FLOATING_POINT = true;
};

template <>
struct NumTraits<f64>
{
  static constexpr u8   NUM_BITS       = 64;
  static constexpr u8   LOG2_NUM_BITS  = 6;
  static constexpr f64  MIN            = F64_MIN;
  static constexpr f64  MAX            = F64_MAX;
  static constexpr bool SIGNED         = true;
  static constexpr bool FLOATING_POINT = true;
};

template <typename T>
struct NumTraits<T const> : NumTraits<T>
{
};

template <typename T>
struct NumTraits<T volatile> : NumTraits<T>
{
};

template <typename T>
struct NumTraits<T const volatile> : NumTraits<T>
{
};

template <typename Repr, usize NumBits>
inline constexpr usize BIT_PACKS =
  (NumBits + (NumTraits<Repr>::NUM_BITS - 1)) >> NumTraits<Repr>::LOG2_NUM_BITS;

template <typename Repr>
constexpr usize bit_packs(usize num_bits)
{
  return (num_bits + (NumTraits<Repr>::NUM_BITS - 1)) >>
         NumTraits<Repr>::LOG2_NUM_BITS;
}

/// regular void
struct Void
{
};

template <typename E>
using enum_ut = std::underlying_type_t<E>;

template <typename E>
constexpr enum_ut<E> enum_uv(E a)
{
  return static_cast<enum_ut<E>>(a);
}

template <typename E>
constexpr enum_ut<E> enum_uv_or(E a, E b)
{
  return static_cast<enum_ut<E>>(enum_uv(a) | enum_uv(b));
}

template <typename E>
constexpr enum_ut<E> enum_uv_and(E a, E b)
{
  return static_cast<enum_ut<E>>(enum_uv(a) & enum_uv(b));
}

template <typename E>
constexpr enum_ut<E> enum_uv_not(E a)
{
  return static_cast<enum_ut<E>>(~enum_uv(a));
}

template <typename E>
constexpr enum_ut<E> enum_uv_xor(E a, E b)
{
  return static_cast<enum_ut<E>>(enum_uv(a) ^ enum_uv(b));
}

template <typename E>
constexpr E enum_or(E a, E b)
{
  return static_cast<E>(enum_uv_or(a, b));
}

template <typename E>
constexpr E enum_and(E a, E b)
{
  return static_cast<E>(enum_uv_and(a, b));
}

template <typename E>
constexpr E enum_xor(E a, E b)
{
  return static_cast<E>(enum_uv_xor(a, b));
}

template <typename E>
constexpr E enum_not(E a)
{
  return static_cast<E>(enum_uv_not(a));
}

#define ASH_BIT_ENUM_OPS(E)            \
  constexpr E operator|(E a, E b)      \
  {                                    \
    return ::ash::enum_or(a, b);       \
  }                                    \
                                       \
  constexpr E operator&(E a, E b)      \
  {                                    \
    return ::ash::enum_and(a, b);      \
  }                                    \
                                       \
  constexpr E operator^(E a, E b)      \
  {                                    \
    return ::ash::enum_xor(a, b);      \
  }                                    \
                                       \
  constexpr E operator~(E a)           \
  {                                    \
    return ::ash::enum_not(a);         \
  }                                    \
                                       \
  constexpr E & operator|=(E & a, E b) \
  {                                    \
    a = a | b;                         \
    return a;                          \
  }                                    \
                                       \
  constexpr E & operator&=(E & a, E b) \
  {                                    \
    a = a & b;                         \
    return a;                          \
  }                                    \
                                       \
  constexpr E & operator^=(E & a, E b) \
  {                                    \
    a = a ^ b;                         \
    return a;                          \
  }

template <typename T>
struct ref
{
  using Type = T;

  T * repr_;

  constexpr ref(T & v) : repr_{&v}
  {
  }

  constexpr ref(ref const &)             = default;
  constexpr ref(ref &&)                  = default;
  constexpr ref & operator=(ref const &) = default;
  constexpr ref & operator=(ref &&)      = default;
  constexpr ~ref()                       = default;

  constexpr T & unref() const
  {
    return *repr_;
  }

  constexpr T * operator->() const
  {
    return repr_;
  }

  constexpr operator T &() const
  {
    return *repr_;
  }

  constexpr T & operator*() const
  {
    return *repr_;
  }

  constexpr T * ptr() const
  {
    return repr_;
  }
};

template <typename T>
ref(T &) -> ref<T>;

template <typename S = usize>
struct SliceT
{
  S offset = 0;
  S span   = 0;

  static constexpr SliceT all()
  {
    return SliceT{.offset = 0, .span = NumTraits<S>::MAX};
  }

  constexpr S begin() const
  {
    return offset;
  }

  constexpr S end() const
  {
    return offset + span;
  }

  constexpr SliceT operator()(S size) const
  {
    // written such that overflow will not occur even if both offset and span
    // are set to USIZE_MAX
    S out_offset = offset > size ? size : offset;
    S out_span   = ((size - out_offset) > span) ? span : size - out_offset;
    return SliceT{out_offset, out_span};
  }

  constexpr bool is_empty() const
  {
    return span == 0;
  }

  template <typename T>
  explicit constexpr operator SliceT<T>() const
  {
    return SliceT<T>{.offset = (T) offset, .span = (T) span};
  }
};

using Slice   = SliceT<usize>;
using Slice32 = SliceT<u32>;
using Slice64 = SliceT<u64>;

struct IterEnd
{
};

template <typename T>
struct SpanIter
{
  T * iter_ = nullptr;
  T * end_  = nullptr;

  constexpr SpanIter & operator++()
  {
    ++iter_;
    return *this;
  }

  constexpr T & operator*() const
  {
    return *iter_;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return iter_ != end_;
  }
};

template <typename T, usize N>
constexpr SpanIter<T> begin(T (&a)[N])
{
  return SpanIter<T>{.iter_ = a, .end_ = a + N};
}

template <typename T>
constexpr auto begin(T && a) -> decltype(a.begin())
{
  return a.begin();
}

template <typename T, usize N>
constexpr auto end(T (&)[N])
{
  return IterEnd{};
}

template <typename T>
constexpr auto end(T && a) -> decltype(a.end())
{
  return a.end();
}

template <typename T, usize N>
constexpr T * data(T (&a)[N])
{
  return a;
}

template <typename T>
constexpr auto data(T && a) -> decltype(a.data())
{
  return a.data();
}

template <typename T, usize N>
constexpr usize size(T (&)[N])
{
  return N;
}

template <typename T>
constexpr auto size(T && a) -> decltype(a.size())
{
  return a.size();
}

template <typename T, u32 N>
constexpr u32 size32(T (&)[N])
{
  return N;
}

template <typename T>
constexpr auto size32(T && a) -> decltype(a.size32())
{
  return a.size32();
}

template <typename T, u64 N>
constexpr u64 size64(T (&)[N])
{
  return N;
}

template <typename T>
constexpr auto size64(T && a) -> decltype(a.size64())
{
  return a.size64();
}

template <typename T, usize N>
constexpr usize size_bytes(T (&)[N])
{
  return sizeof(T) * N;
}

template <typename T>
constexpr auto size_bytes(T && a) -> decltype(a.size())
{
  return sizeof(T) * a.size();
}

template <typename T, usize N>
constexpr usize size_bits(T (&)[N])
{
  return sizeof(T) * 8 * N;
}

template <typename T>
constexpr auto size_bits(T && a) -> decltype(a.size())
{
  return sizeof(T) * a.size() * 8;
}

template <typename T>
constexpr auto is_empty(T && a)
{
  return size(a) == 0;
}

/// @brief Iterator Model. Iterators are only required to
/// produce values, they are not required to provide
/// references to the values
template <typename T>
concept Iter = requires (T it) {
  {
    // value producer
    *it
  };
  {
    // in-place (pre-fix) advancement
    ++it
  };
};

/// @brief Range Model. Ranges are read-only by default.
template <typename R>
concept Range = requires (R r) {
  {
    // can get an iterator to its beginning element
    begin(r)
  } -> Iter;
  {
    // returns boolean when asked if it has ended
    !(begin(r) != end(r))
  };
};

template <typename T>
concept OutIter = Iter<T>;

template <typename T>
concept OutRange = Range<T>;

template <typename U, typename T>
concept SpanCompatible = Convertible<U (*)[], T (*)[]>;

template <typename Container>
using ContainerDataType =
  std::remove_pointer_t<decltype(data(std::declval<Container &>()))>;

template <typename Container>
concept SpanContainer = requires (Container cont) {
  { data(cont) };
  { size(cont) };
};

template <typename Container, typename T>
concept SpanCompatibleContainer =
  SpanContainer<Container> && SpanCompatible<ContainerDataType<Container>, T>;

template <typename T>
struct Span
{
  using Type = T;
  using Repr = T;
  using Iter = SpanIter<T>;

  T *   data_ = nullptr;
  usize size_ = 0;

  constexpr Span() = default;

  constexpr Span(T * data, usize size) : data_{data}, size_{size}
  {
  }

  constexpr Span(T * begin, T const * end) :
    data_{begin},
    size_{static_cast<usize>(end - begin)}
  {
  }

  constexpr Span(Iter iter, IterEnd = {}) : Span{iter.iter_, iter.end_}
  {
  }

  template <usize N>
  constexpr Span(T (&data)[N]) : data_{data}, size_{N}
  {
  }

  template <SpanCompatibleContainer<T> C>
  constexpr Span(C & cont) : data_{ash::data(cont)}, size_{ash::size(cont)}
  {
  }

  template <SpanCompatible<T> U>
  constexpr Span(Span<U> span) : data_{span.data_}, size_{span.size_}
  {
  }

  constexpr Span(Span const &) = default;

  constexpr Span(Span &&) = default;

  constexpr Span & operator=(Span const &) = default;

  constexpr Span & operator=(Span &&) = default;

  constexpr ~Span() = default;

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T * data() const
  {
    return data_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr u32 size32() const
  {
    return (u32) size_;
  }

  constexpr u64 size64() const
  {
    return (u64) size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  constexpr Iter begin() const
  {
    return Iter{.iter_ = data_, .end_ = data_ + size_};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr T * pbegin() const
  {
    return data_;
  }

  constexpr T * pend() const
  {
    return data_ + size_;
  }

  constexpr T & first() const
  {
    return get(0);
  }

  constexpr T & last() const
  {
    return get(size_ - 1);
  }

  constexpr T & operator[](usize index) const
  {
    return data_[index];
  }

  constexpr T & get(usize index) const
  {
    return data_[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args) const requires (NonConst<T>)
  {
    data_[index] = T{static_cast<Args &&>(args)...};
  }

  constexpr Span<T const> as_const() const
  {
    return Span<T const>{data_, size_};
  }

  constexpr Span<u8> as_u8() const requires (NonConst<T>)
  {
    return Span<u8>{reinterpret_cast<u8 *>(data_), size_bytes()};
  }

  constexpr Span<u8 const> as_u8() const requires (Const<T>)
  {
    return Span<u8 const>{reinterpret_cast<u8 const *>(data_), size_bytes()};
  }

  constexpr Span<char> as_char() const requires (NonConst<T>)
  {
    return Span<char>{reinterpret_cast<char *>(data_), size_bytes()};
  }

  constexpr Span<char const> as_char() const requires (Const<T>)
  {
    return Span<char const>{reinterpret_cast<char const *>(data_),
                            size_bytes()};
  }

  constexpr Span<c8> as_c8() const requires (NonConst<T>)
  {
    return Span<c8>{reinterpret_cast<c8 *>(data_), size_bytes()};
  }

  constexpr Span<c8 const> as_c8() const requires (Const<T>)
  {
    return Span<c8 const>{reinterpret_cast<c8 const *>(data_), size_bytes()};
  }

  constexpr Span slice(usize offset, usize span) const
  {
    return slice(Slice{offset, span});
  }

  constexpr Span slice(Slice s) const
  {
    s = s(size_);
    return Span{data_ + s.offset, s.span};
  }

  constexpr Span slice(usize offset) const
  {
    return slice(offset, USIZE_MAX);
  }

  template <typename U>
  Span<U> reinterpret() const
  {
    return Span<U>{reinterpret_cast<U *>(data_),
                   (sizeof(T) * size_) / sizeof(U)};
  }
};

template <typename T, usize N>
Span(T (&)[N]) -> Span<T>;

template <SpanContainer C>
Span(C & container) -> Span<std::remove_pointer_t<decltype(data(container))>>;

template <typename T>
constexpr Span<T const> span(std::initializer_list<T> list)
{
  return Span<T const>{list.begin(), list.size()};
}

template <typename T, usize N>
constexpr Span<T> span(T (&array)[N])
{
  return Span<T>{array, N};
}

template <SpanContainer C>
constexpr auto span(C & c)
{
  return Span{data(c), size(c)};
}

template <typename T, usize N>
constexpr Span<T> view(T (&array)[N])
{
  return span(array);
}

template <typename R>
constexpr auto view(R & range) -> decltype(range.view())
{
  return range.view();
}

constexpr Span<char const> operator""_str(char const * lit, usize n)
{
  return Span<char const>{lit, n};
}

constexpr Span<c8 const> operator""_str(c8 const * lit, usize n)
{
  return Span<c8 const>{lit, n};
}

constexpr Span<c16 const> operator""_str(c16 const * lit, usize n)
{
  return Span<c16 const>{lit, n};
}

constexpr Span<c32 const> operator""_str(c32 const * lit, usize n)
{
  return Span<c32 const>{lit, n};
}

constexpr bool get_bit(Span<u8 const> s, usize i)
{
  return get_bit(s[i >> 3], i & 7);
}

constexpr bool get_bit(Span<u16 const> s, usize i)
{
  return get_bit(s[i >> 4], i & 15);
}

constexpr bool get_bit(Span<u32 const> s, usize i)
{
  return get_bit(s[i >> 5], i & 31);
}

constexpr bool get_bit(Span<u64 const> s, usize i)
{
  return get_bit(s[i >> 6], i & 63);
}

constexpr void set_bit(Span<u8> s, usize i)
{
  set_bit(s[i >> 3], i & 7);
}

constexpr void set_bit(Span<u16> s, usize i)
{
  set_bit(s[i >> 4], i & 15);
}

constexpr void set_bit(Span<u32> s, usize i)
{
  set_bit(s[i >> 5], i & 31);
}

constexpr void set_bit(Span<u64> s, usize i)
{
  set_bit(s[i >> 6], i & 63);
}

constexpr void assign_bit(Span<u8> s, usize i, bool b)
{
  assign_bit(s[i >> 3], i & 7, b);
}

constexpr void assign_bit(Span<u16> s, usize i, bool b)
{
  assign_bit(s[i >> 4], i & 15, b);
}

constexpr void assign_bit(Span<u32> s, usize i, bool b)
{
  assign_bit(s[i >> 5], i & 31, b);
}

constexpr void assign_bit(Span<u64> s, usize i, bool b)
{
  assign_bit(s[i >> 6], i & 63, b);
}

constexpr void clear_bit(Span<u8> s, usize i)
{
  clear_bit(s[i >> 3], i & 7);
}

constexpr void clear_bit(Span<u16> s, usize i)
{
  clear_bit(s[i >> 4], i & 15);
}

constexpr void clear_bit(Span<u32> s, usize i)
{
  clear_bit(s[i >> 5], i & 31);
}

constexpr void clear_bit(Span<u64> s, usize i)
{
  clear_bit(s[i >> 6], i & 63);
}

constexpr void flip_bit(Span<u8> s, usize i)
{
  flip_bit(s[i >> 3], i & 7);
}

constexpr void flip_bit(Span<u16> s, usize i)
{
  flip_bit(s[i >> 4], i & 15);
}

constexpr void flip_bit(Span<u32> s, usize i)
{
  flip_bit(s[i >> 5], i & 31);
}

constexpr void flip_bit(Span<u64> s, usize i)
{
  flip_bit(s[i >> 6], i & 63);
}

template <typename T>
constexpr usize impl_find_set_bit(Span<T const> s)
{
  T const * const begin = s.pbegin();
  T const *       iter  = s.pbegin();
  T const * const end   = s.pend();

  while (iter != end && *iter == 0)
  {
    iter++;
  }

  usize const idx = static_cast<usize>(iter - begin)
                    << NumTraits<T>::LOG2_NUM_BITS;

  if (iter == end)
  {
    return idx;
  }

  return idx | std::countr_zero(*iter);
}

template <typename T>
constexpr usize impl_find_clear_bit(Span<T const> s)
{
  T const * const begin = s.pbegin();
  T const *       iter  = s.pbegin();
  T const * const end   = s.pend();

  while (iter != end && *iter == NumTraits<T>::MAX)
  {
    iter++;
  }

  usize const idx = static_cast<usize>(iter - begin)
                    << NumTraits<T>::LOG2_NUM_BITS;

  if (iter == end)
  {
    return idx;
  }

  return idx | std::countr_one(*iter);
}

constexpr usize find_set_bit(Span<u8 const> s)
{
  return impl_find_set_bit(s);
}

constexpr usize find_set_bit(Span<u16 const> s)
{
  return impl_find_set_bit(s);
}

constexpr usize find_set_bit(Span<u32 const> s)
{
  return impl_find_set_bit(s);
}

constexpr usize find_set_bit(Span<u64 const> s)
{
  return impl_find_set_bit(s);
}

constexpr usize find_clear_bit(Span<u8 const> s)
{
  return impl_find_clear_bit(s);
}

constexpr usize find_clear_bit(Span<u16 const> s)
{
  return impl_find_clear_bit(s);
}

constexpr usize find_clear_bit(Span<u32 const> s)
{
  return impl_find_clear_bit(s);
}

constexpr usize find_clear_bit(Span<u64 const> s)
{
  return impl_find_clear_bit(s);
}

template <typename R>
struct BitSpanIter
{
  Span<R> repr_{};
  usize   pos_  = 0;
  usize   size_ = 0;

  constexpr bool operator*() const
  {
    return get_bit(repr_, pos_);
  }

  constexpr BitSpanIter & operator++()
  {
    ++pos_;
    return *this;
  }

  constexpr bool operator!=(IterEnd) const
  {
    return pos_ != size_;
  }
};

template <typename R>
struct BitSpan
{
  using Type = bool;
  using Repr = R;
  using Iter = BitSpanIter<R>;

  Span<R> repr_ = {};

  constexpr BitSpan() = default;

  constexpr BitSpan(Span<R> repr) : repr_{repr}
  {
  }

  template <usize N>
  constexpr BitSpan(R (&data)[N]) : repr_{data}
  {
  }

  template <SpanCompatibleContainer<R> C>
  constexpr BitSpan(C & cont) : repr_{cont}
  {
  }

  constexpr BitSpan(BitSpan const &) = default;

  constexpr BitSpan(BitSpan &&) = default;

  constexpr BitSpan & operator=(BitSpan const &) = default;

  constexpr BitSpan & operator=(BitSpan &&) = default;

  constexpr ~BitSpan() = default;

  constexpr Span<R> repr() const
  {
    return repr_;
  }

  constexpr usize size() const
  {
    return repr_.size_bytes() * 8;
  }

  constexpr bool is_empty() const
  {
    return repr_.is_empty();
  }

  constexpr auto begin() const
  {
    return Iter{.repr_ = repr_, .bit_pos_ = 0, .bit_size_ = size()};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr bool operator[](usize index) const
  {
    return ash::get_bit(repr_, index);
  }

  constexpr bool get(usize index) const
  {
    return ash::get_bit(repr_, index);
  }

  constexpr void set(usize index, bool value) const requires (NonConst<R>)
  {
    ash::assign_bit(repr_, index, value);
  }

  constexpr bool get_bit(usize index) const
  {
    return ash::get_bit(repr_, index);
  }

  constexpr void set_bit(usize index) const requires (NonConst<R>)
  {
    ash::set_bit(repr_, index);
  }

  constexpr void clear_bit(usize index) const requires (NonConst<R>)
  {
    ash::clear_bit(repr_, index);
  }

  constexpr void flip_bit(usize index) const requires (NonConst<R>)
  {
    ash::flip_bit(repr_, index);
  }

  constexpr usize find_set_bit()
  {
    return ash::find_set_bit(repr_);
  }

  constexpr usize find_clear_bit()
  {
    return ash::find_clear_bit(repr_);
  }

  constexpr operator BitSpan<R const>() const
  {
    return BitSpan<R const>{repr_};
  }

  constexpr BitSpan<R const> as_const() const
  {
    return BitSpan<R const>{repr_};
  }
};

template <typename T, usize N>
struct Array
{
  using Type      = T;
  using View      = Span<T>;
  using ConstView = Span<T const>;
  using Iter      = SpanIter<T>;
  using ConstIter = SpanIter<T const>;

  static constexpr usize SIZE = N;

  T data_[SIZE]{};

  static constexpr bool is_empty()
  {
    return false;
  }

  constexpr T * data()
  {
    return data_;
  }

  constexpr T const * data() const
  {
    return data_;
  }

  static constexpr usize size()
  {
    return SIZE;
  }

  static constexpr u32 size32()
  {
    return (u32) SIZE;
  }

  static constexpr u64 size64()
  {
    return (u64) SIZE;
  }

  static constexpr usize capacity()
  {
    return SIZE;
  }

  static constexpr usize size_bytes()
  {
    return sizeof(T) * SIZE;
  }

  constexpr T * begin()
  {
    return data_;
  }

  constexpr T const * begin() const
  {
    return data_;
  }

  constexpr T * end()
  {
    return data_ + SIZE;
  }

  constexpr T const * end() const
  {
    return data_ + SIZE;
  }

  constexpr T & first()
  {
    return get(0);
  }

  constexpr T const & first() const
  {
    return get(0);
  }

  constexpr T & last()
  {
    return get(SIZE - 1);
  }

  constexpr T const & last() const
  {
    return get(SIZE - 1);
  }

  constexpr T & get(usize index)
  {
    return data_[index];
  }

  constexpr T const & get(usize index) const
  {
    return data_[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args)
  {
    data_[index] = T{static_cast<Args &&>(args)...};
  }

  constexpr T & operator[](usize index)
  {
    return data_[index];
  }

  constexpr T const & operator[](usize index) const
  {
    return data_[index];
  }

  constexpr operator T *()
  {
    return data_;
  }

  constexpr operator T const *() const
  {
    return data_;
  }

  constexpr ConstView view() const
  {
    return ConstView{data(), size()};
  }

  constexpr View view()
  {
    return View{data(), size()};
  }
};

template <typename T>
struct Array<T, 0>
{
  using Type      = T;
  using View      = Span<T>;
  using ConstView = Span<T const>;
  using Iter      = SpanIter<T>;
  using ConstIter = SpanIter<T const>;

  static constexpr usize SIZE = 0;

  static constexpr bool is_empty()
  {
    return true;
  }

  constexpr T * data()
  {
    return nullptr;
  }

  constexpr T const * data() const
  {
    return nullptr;
  }

  static constexpr usize size()
  {
    return SIZE;
  }

  static constexpr u32 size32()
  {
    return (u32) SIZE;
  }

  static constexpr u64 size64()
  {
    return (u64) SIZE;
  }

  static constexpr usize capacity()
  {
    return SIZE;
  }

  static constexpr usize size_bytes()
  {
    return sizeof(T) * SIZE;
  }

  constexpr T * begin()
  {
    return nullptr;
  }

  constexpr T const * begin() const
  {
    return nullptr;
  }

  constexpr T * end()
  {
    return nullptr;
  }

  constexpr T const * end() const
  {
    return nullptr;
  }

  constexpr T & first() requires (SIZE > 1)
  {
    return get(0);
  }

  constexpr T const & first() const requires (SIZE > 1)
  {
    return get(0);
  }

  constexpr T & last() requires (SIZE > 1)
  {
    return get(SIZE - 1);
  }

  constexpr T const & last() const requires (SIZE > 1)
  {
    return get(SIZE - 1);
  }

  constexpr T & get(usize index) requires (SIZE > 1)
  {
    return data()[index];
  }

  constexpr T const & get(usize index) const requires (SIZE > 1)
  {
    return data()[index];
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args) requires (SIZE > 1)
  {
    data()[index] = T{static_cast<Args &&>(args)...};
  }

  constexpr T & operator[](usize index) requires (SIZE > 1)
  {
    return data()[index];
  }

  constexpr T const & operator[](usize index) const
  {
    return data()[index];
  }

  constexpr operator T *()
  {
    return nullptr;
  }

  constexpr operator T const *() const
  {
    return nullptr;
  }

  constexpr ConstView view() const
  {
    return ConstView{data(), size()};
  }

  constexpr View view()
  {
    return View{data(), size()};
  }
};

template <typename T, typename... U>
Array(T, U...) -> Array<T, 1 + sizeof...(U)>;

template <typename T, usize N>
struct IsTriviallyRelocatable<Array<T, N>>
{
  static constexpr bool value = TriviallyRelocatable<T>;
};

template <typename Repr, usize N>
using Bits = Repr[BIT_PACKS<Repr, N>];

template <typename Repr, usize N>
using BitArray = Array<Repr, BIT_PACKS<Repr, N>>;

template <typename Lambda>
struct defer
{
  Lambda lambda;
  constexpr defer(defer &&)                  = delete;
  constexpr defer(defer const &)             = delete;
  constexpr defer & operator=(defer &&)      = delete;
  constexpr defer & operator=(defer const &) = delete;

  constexpr defer(Lambda && l) : lambda{static_cast<Lambda &&>(l)}
  {
  }

  constexpr ~defer()
  {
    lambda();
  }
};

template <typename Lambda>
defer(Lambda &&) -> defer<Lambda>;

template <typename Sig>
struct PFnThunk;

template <typename R, typename... Args>
struct PFnThunk<R(Args...)>
{
  static constexpr R thunk(void * data, Args... args)
  {
    using PFn = R (*)(Args...);

    PFn pfn = reinterpret_cast<PFn>(data);

    return pfn(static_cast<Args &&>(args)...);
  }
};

template <typename T, typename Sig>
struct FunctorThunk;

template <typename T, typename R, typename... Args>
struct FunctorThunk<T, R(Args...)>
{
  static constexpr R thunk(void * data, Args... args)
  {
    return (*(reinterpret_cast<T *>(data)))(static_cast<Args &&>(args)...);
  }
};

template <typename Sig>
struct Fn;

/// @brief Fn is a type-erased function containing a callback and a pointer. Fn
/// is a reference to both the function to be called and its associated data, it
/// doesn't manage any lifetime.
/// @param thunk function/callback to be invoked. typically a
/// dispatcher/thunk.
/// @param data associated data/context for the thunk to operate on.
template <typename R, typename... Args>
struct Fn<R(Args...)>
{
  using Thunk = R (*)(void *, Args...);

  void * data = nullptr;

  Thunk thunk = nullptr;

  explicit constexpr Fn() = default;

  constexpr Fn(Fn const &)             = default;
  constexpr Fn(Fn &&)                  = default;
  constexpr Fn & operator=(Fn const &) = default;
  constexpr Fn & operator=(Fn &&)      = default;
  constexpr ~Fn()                      = default;

  constexpr Fn(void * data, Thunk thunk) : data{data}, thunk{thunk}
  {
  }

  Fn(R (*pfn)(Args...)) :
    data{reinterpret_cast<void *>(pfn)},
    thunk{&PFnThunk<R(Args...)>::thunk}
  {
  }

  template <typename PFn>
  requires (Convertible<PFn, R (*)(Args...)>)
  Fn(PFn pfn) : Fn{static_cast<R (*)(Args...)>(pfn)}
  {
  }

  template <typename T>
  Fn(T * data, R (*thunk)(T *, Args...)) :
    data{const_cast<void *>(reinterpret_cast<void const *>(data))},
    thunk{reinterpret_cast<Thunk>(thunk)}
  {
  }

  template <typename T, typename PFn>
  requires (Convertible<PFn, R (*)(T *, Args...)>)
  Fn(T * data, PFn thunk) : Fn{data, static_cast<R (*)(T *, Args...)>(thunk)}
  {
  }

  constexpr R operator()(Args... args) const
  {
    return thunk(data, static_cast<Args &&>(args)...);
  }
};

template <typename R, typename... Args>
Fn(R (*)(Args...)) -> Fn<R(Args...)>;

template <typename T, typename R, typename... Args>
Fn(T *, R (*)(T *, Args...)) -> Fn<R(Args...)>;

template <typename Sig>
struct PFnTraits;

template <typename R, typename... Args>
struct PFnTraits<R(Args...)>
{
  using Ptr    = R (*)(Args...);
  using Fn     = ash::Fn<R(Args...)>;
  using Return = R;
  using Thunk  = PFnThunk<R(Args...)>;
};

template <typename R, typename... Args>
struct PFnTraits<R (*)(Args...)> : PFnTraits<R(Args...)>
{
};

template <typename Sig>
struct MethodTraits;

/// @brief non-const method traits
template <typename T, typename R, typename... Args>
struct MethodTraits<R (T::*)(Args...)>
{
  using Ptr    = R (*)(Args...);
  using Fn     = ash::Fn<R(Args...)>;
  using Type   = T;
  using Return = R;
  using Thunk  = FunctorThunk<T, R(Args...)>;
};

/// @brief const method traits
template <typename T, typename R, typename... Args>
struct MethodTraits<R (T::*)(Args...) const>
{
  using Ptr    = R (*)(Args...);
  using Fn     = ash::Fn<R(Args...)>;
  using Type   = T const;
  using Return = R;
  using Thunk  = FunctorThunk<T const, R(Args...)>;
};

template <typename F>
struct FunctorTraits : MethodTraits<decltype(&F::operator())>
{
};

/// @brief make a function view from a raw function pointer.
template <typename R, typename... Args>
auto fn(R (*pfn)(Args...))
{
  return Fn<R(Args...)>{pfn};
}

/// @brief make a function view from a functor reference. Functor should outlive
/// the Fn
template <typename F>
auto fn(F & functor)
{
  using Traits = FunctorTraits<F>;
  using Fn     = typename Traits::Fn;
  using Thunk  = typename Traits::Thunk;

  return Fn{const_cast<void *>(reinterpret_cast<void const *>(&functor)),
            &Thunk::thunk};
}

/// @brief create a function view from an object reference and a function
/// thunk to execute using the object reference as its first argument.
template <typename T, typename R, typename... Args>
auto fn(T * data, R (*thunk)(T *, Args...))
{
  return Fn<R(Args...)>{data, thunk};
}

struct Noop
{
  template <typename... Args>
  using Sig = void(Args...);

  template <typename... Args>
  constexpr operator Sig<Args...> *() const
  {
    return [](Args...) {};
  }

  template <typename... Args>
  constexpr void operator()(Args &&...) const
  {
  }
};

constexpr Noop noop;

/// @brief The `SourceLocation`  class represents certain information about the
/// source code, such as file names, line numbers, and function names.
/// Previously, functions that desire to obtain this information about the call
/// site (for logging, testing, or debugging purposes) must use macros so that
/// predefined macros like `__LINE__` and `__FILE__` are expanded in the context
/// of the caller. The `SourceLocation` class provides a better alternative.
///
/// based on: https://en.cppreference.com/w/cpp/utility/source_location
///
struct SourceLocation
{
  static constexpr SourceLocation current(
#if ASH_HAS_BUILTIN(FILE) || (defined(__cpp_lib_source_location) && \
                              __cpp_lib_source_location >= 201'907L)
    char const * file = __builtin_FILE(),
#elif defined(__FILE__)
    char const * file = __FILE__,
#else
    char const * file = "unknown",
#endif

#if ASH_HAS_BUILTIN(FUNCTION) || (defined(__cpp_lib_source_location) && \
                                  __cpp_lib_source_location >= 201'907L)
    char const * function = __builtin_FUNCTION(),
#else
    char const * function = "unknown",
#endif

#if ASH_HAS_BUILTIN(LINE) || (defined(__cpp_lib_source_location) && \
                              __cpp_lib_source_location >= 201'907L)
    u32 line = __builtin_LINE(),
#elif defined(__LINE__)
    u32 line = __LINE__,
#else
    u32 line = 0,
#endif

#if ASH_HAS_BUILTIN(COLUMN) || (defined(__cpp_lib_source_location) && \
                                __cpp_lib_source_location >= 201'907L)
    u32 column = __builtin_COLUMN()
#else
    u32 column = 0
#endif
  )
  {
    return SourceLocation{file, function, line, column};
  }

  char const * file     = "";
  char const * function = "";
  u32          line     = 0;
  u32          column   = 0;
};

template <typename T = void>
struct Pin
{
  typedef T Type;

  T v;

  template <typename... Args>
  constexpr Pin(Args &&... args) : v{static_cast<Args &&>(args)...}
  {
  }

  constexpr Pin(Pin const &)             = delete;
  constexpr Pin(Pin &&)                  = delete;
  constexpr Pin & operator=(Pin const &) = delete;
  constexpr Pin & operator=(Pin &&)      = delete;
  constexpr ~Pin()                       = default;
};

template <>
struct Pin<void>
{
  typedef void Type;

  constexpr Pin()                        = default;
  constexpr Pin(Pin const &)             = delete;
  constexpr Pin(Pin &&)                  = delete;
  constexpr Pin & operator=(Pin const &) = delete;
  constexpr Pin & operator=(Pin &&)      = delete;
  constexpr ~Pin()                       = default;
};

/// @brief In-place type constructor flag. Intended for functions that take
/// generic types and want to overload with a second type that constructs the type using the
/// provided arguments.
struct Inplace
{
};

inline constexpr Inplace inplace{};

/// @brief uninitialized storage
template <usize Alignment, usize Capacity>
struct InplaceStorage
{
  alignas(Alignment) mutable u8 storage_[Capacity];
};

template <usize Alignment>
struct InplaceStorage<Alignment, 0>
{
  static constexpr u8 * storage_ = nullptr;
};

template <typename T>
using Storage = InplaceStorage<alignof(T), sizeof(T)>;

}    // namespace ash
