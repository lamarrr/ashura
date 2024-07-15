/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/traits.h"
#include <bit>
#include <cfloat>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <new>
#include <type_traits>

namespace ash
{
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef float     f32;
typedef double    f64;
typedef size_t    usize;
typedef ptrdiff_t isize;
typedef u64       uid;
typedef u64       Hash;

constexpr u8 U8_MIN = 0;
constexpr u8 U8_MAX = 0xFF;

constexpr i8 I8_MIN = -0x7F - 1;
constexpr i8 I8_MAX = 0x7F;

constexpr u16 U16_MIN = 0;
constexpr u16 U16_MAX = 0xFFFF;

constexpr i16 I16_MIN = -0x7FFF - 1;
constexpr i16 I16_MAX = 0x7FFF;

constexpr u32 U32_MIN = 0;
constexpr u32 U32_MAX = 0xFFFFFFFFU;

constexpr i32 I32_MIN = -0x7FFFFFFF - 1;
constexpr i32 I32_MAX = 0x7FFFFFFF;

constexpr u64 U64_MIN = 0;
constexpr u64 U64_MAX = 0xFFFFFFFFFFFFFFFFULL;

constexpr i64 I64_MIN = -0x7FFFFFFFFFFFFFFFLL - 1;
constexpr i64 I64_MAX = 0x7FFFFFFFFFFFFFFFLL;

constexpr usize USIZE_MIN = 0;
constexpr usize USIZE_MAX = SIZE_MAX;

constexpr isize ISIZE_MIN = PTRDIFF_MIN;
constexpr isize ISIZE_MAX = PTRDIFF_MAX;

constexpr f32 F32_MIN          = -FLT_MAX;
constexpr f32 F32_MIN_POSITIVE = FLT_MIN;
constexpr f32 F32_MAX          = FLT_MAX;
constexpr f32 F32_EPSILON      = FLT_EPSILON;

constexpr f64 F64_MIN          = -DBL_MAX;
constexpr f64 F64_MIN_POSITIVE = DBL_MIN;
constexpr f64 F64_MAX          = DBL_MAX;
constexpr f32 F64_EPSILON      = DBL_EPSILON;

constexpr uid UID_MAX = U64_MAX;

constexpr f32 PI = 3.14159265358979323846F;

template <typename E>
using enum_ut = std::underlying_type_t<E>;

template <typename E>
[[nodiscard]] constexpr enum_ut<E> enum_uv(E a)
{
  return static_cast<enum_ut<E>>(a);
}

template <typename E>
[[nodiscard]] constexpr enum_ut<E> enum_uv_or(E a, E b)
{
  return static_cast<enum_ut<E>>(enum_uv(a) | enum_uv(b));
}

template <typename E>
[[nodiscard]] constexpr enum_ut<E> enum_uv_and(E a, E b)
{
  return static_cast<enum_ut<E>>(enum_uv(a) & enum_uv(b));
}

template <typename E>
[[nodiscard]] constexpr enum_ut<E> enum_uv_not(E a)
{
  return static_cast<enum_ut<E>>(~enum_uv(a));
}

template <typename E>
[[nodiscard]] constexpr enum_ut<E> enum_uv_xor(E a, E b)
{
  return static_cast<enum_ut<E>>(enum_uv(a) ^ enum_uv(b));
}

template <typename E>
[[nodiscard]] constexpr E enum_or(E a, E b)
{
  return static_cast<E>(enum_uv_or(a, b));
}

template <typename E>
[[nodiscard]] constexpr E enum_and(E a, E b)
{
  return static_cast<E>(enum_uv_and(a, b));
}

template <typename E>
[[nodiscard]] constexpr E enum_xor(E a, E b)
{
  return static_cast<E>(enum_uv_xor(a, b));
}

template <typename E>
[[nodiscard]] constexpr E enum_not(E a)
{
  return static_cast<E>(enum_uv_not(a));
}

#define ASH_DEFINE_ENUM_BIT_OPS(E)                 \
  [[nodiscard]] constexpr E operator|(E a, E b)    \
  {                                                \
    return ::ash::enum_or(a, b);                   \
  }                                                \
                                                   \
  [[nodiscard]] constexpr E operator&(E a, E b)    \
  {                                                \
    return ::ash::enum_and(a, b);                  \
  }                                                \
                                                   \
  [[nodiscard]] constexpr E operator^(E a, E b)    \
  {                                                \
    return ::ash::enum_xor(a, b);                  \
  }                                                \
                                                   \
  [[nodiscard]] constexpr E operator~(E a)         \
  {                                                \
    return ::ash::enum_not(a);                     \
  }                                                \
                                                   \
  [[nodiscard]] constexpr E &operator|=(E &a, E b) \
  {                                                \
    a = a | b;                                     \
    return a;                                      \
  }                                                \
                                                   \
  [[nodiscard]] constexpr E &operator&=(E &a, E b) \
  {                                                \
    a = a & b;                                     \
    return a;                                      \
  }                                                \
                                                   \
  [[nodiscard]] constexpr E &operator^=(E &a, E b) \
  {                                                \
    a = a ^ b;                                     \
    return a;                                      \
  }

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

/// regular void
struct Void
{
};

template <typename... T>
struct Tuple
{
  static constexpr u8 NUM_ELEMENTS = 0;
};

Tuple() -> Tuple<>;

template <typename T0>
struct Tuple<T0>
{
  typedef T0 Type0;

  static constexpr u8 NUM_ELEMENTS = 1;

  T0 v0{};
};

template <typename T0>
Tuple(T0) -> Tuple<T0>;

template <typename T0, typename T1>
struct Tuple<T0, T1>
{
  typedef T0 Type0;
  typedef T1 Type1;

  static constexpr u8 NUM_ELEMENTS = 2;

  T0 v0{};
  T1 v1{};
};

template <typename T0, typename T1>
Tuple(T0, T1) -> Tuple<T0, T1>;

template <typename T0, typename T1, typename T2>
struct Tuple<T0, T1, T2>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;

  static constexpr u8 NUM_ELEMENTS = 3;

  T0 v0{};
  T1 v1{};
  T2 v2{};
};

template <typename T0, typename T1, typename T2>
Tuple(T0, T1, T2) -> Tuple<T0, T1, T2>;

template <typename T0, typename T1, typename T2, typename T3>
struct Tuple<T0, T1, T2, T3>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;
  typedef T3 Type3;

  static constexpr u8 NUM_ELEMENTS = 4;

  T0 v0{};
  T1 v1{};
  T2 v2{};
  T3 v3{};
};

template <typename T0, typename T1, typename T2, typename T3>
Tuple(T0, T1, T2, T3) -> Tuple<T0, T1, T2, T3>;

template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct Tuple<T0, T1, T2, T3, T4>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;
  typedef T3 Type3;
  typedef T4 Type4;

  static constexpr u8 NUM_ELEMENTS = 5;

  T0 v0{};
  T1 v1{};
  T2 v2{};
  T3 v3{};
  T4 v4{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4>
Tuple(T0, T1, T2, T3, T4) -> Tuple<T0, T1, T2, T3, T4>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5>
struct Tuple<T0, T1, T2, T3, T4, T5>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;
  typedef T3 Type3;
  typedef T4 Type4;
  typedef T5 Type5;

  static constexpr u8 NUM_ELEMENTS = 6;

  T0 v0{};
  T1 v1{};
  T2 v2{};
  T3 v3{};
  T4 v4{};
  T5 v5{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5>
Tuple(T0, T1, T2, T3, T4, T5) -> Tuple<T0, T1, T2, T3, T4, T5>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6>
struct Tuple<T0, T1, T2, T3, T4, T5, T6>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;
  typedef T3 Type3;
  typedef T4 Type4;
  typedef T5 Type5;
  typedef T6 Type6;

  static constexpr u8 NUM_ELEMENTS = 7;

  T0 v0{};
  T1 v1{};
  T2 v2{};
  T3 v3{};
  T4 v4{};
  T5 v5{};
  T6 v6{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6>
Tuple(T0, T1, T2, T3, T4, T5, T6) -> Tuple<T0, T1, T2, T3, T4, T5, T6>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;
  typedef T3 Type3;
  typedef T4 Type4;
  typedef T5 Type5;
  typedef T6 Type6;
  typedef T7 Type7;

  static constexpr u8 NUM_ELEMENTS = 8;

  T0 v0{};
  T1 v1{};
  T2 v2{};
  T3 v3{};
  T4 v4{};
  T5 v5{};
  T6 v6{};
  T7 v7{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;
  typedef T3 Type3;
  typedef T4 Type4;
  typedef T5 Type5;
  typedef T6 Type6;
  typedef T7 Type7;
  typedef T8 Type8;

  static constexpr u8 NUM_ELEMENTS = 9;

  T0 v0{};
  T1 v1{};
  T2 v2{};
  T3 v3{};
  T4 v4{};
  T5 v5{};
  T6 v6{};
  T7 v7{};
  T8 v8{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7,
      T8) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>
{
  typedef T0 Type0;
  typedef T1 Type1;
  typedef T2 Type2;
  typedef T3 Type3;
  typedef T4 Type4;
  typedef T5 Type5;
  typedef T6 Type6;
  typedef T7 Type7;
  typedef T8 Type8;
  typedef T9 Type9;

  static constexpr u8 NUM_ELEMENTS = 10;

  T0 v0{};
  T1 v1{};
  T2 v2{};
  T3 v3{};
  T4 v4{};
  T5 v5{};
  T6 v6{};
  T7 v7{};
  T8 v8{};
  T9 v9{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8,
      T9) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>
{
  typedef T0  Type0;
  typedef T1  Type1;
  typedef T2  Type2;
  typedef T3  Type3;
  typedef T4  Type4;
  typedef T5  Type5;
  typedef T6  Type6;
  typedef T7  Type7;
  typedef T8  Type8;
  typedef T9  Type9;
  typedef T10 Type10;

  static constexpr u8 NUM_ELEMENTS = 11;

  T0  v0{};
  T1  v1{};
  T2  v2{};
  T3  v3{};
  T4  v4{};
  T5  v5{};
  T6  v6{};
  T7  v7{};
  T8  v8{};
  T9  v9{};
  T10 v10{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9,
      T10) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>
{
  typedef T0  Type0;
  typedef T1  Type1;
  typedef T2  Type2;
  typedef T3  Type3;
  typedef T4  Type4;
  typedef T5  Type5;
  typedef T6  Type6;
  typedef T7  Type7;
  typedef T8  Type8;
  typedef T9  Type9;
  typedef T10 Type10;
  typedef T11 Type11;

  static constexpr u8 NUM_ELEMENTS = 12;

  T0  v0{};
  T1  v1{};
  T2  v2{};
  T3  v3{};
  T4  v4{};
  T5  v5{};
  T6  v6{};
  T7  v7{};
  T8  v8{};
  T9  v9{};
  T10 v10{};
  T11 v11{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10,
      T11) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>
{
  typedef T0  Type0;
  typedef T1  Type1;
  typedef T2  Type2;
  typedef T3  Type3;
  typedef T4  Type4;
  typedef T5  Type5;
  typedef T6  Type6;
  typedef T7  Type7;
  typedef T8  Type8;
  typedef T9  Type9;
  typedef T10 Type10;
  typedef T11 Type11;
  typedef T12 Type12;

  static constexpr u8 NUM_ELEMENTS = 13;

  T0  v0{};
  T1  v1{};
  T2  v2{};
  T3  v3{};
  T4  v4{};
  T5  v5{};
  T6  v6{};
  T7  v7{};
  T8  v8{};
  T9  v9{};
  T10 v10{};
  T11 v11{};
  T12 v12{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11,
      T12) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12, typename T13>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>
{
  typedef T0  Type0;
  typedef T1  Type1;
  typedef T2  Type2;
  typedef T3  Type3;
  typedef T4  Type4;
  typedef T5  Type5;
  typedef T6  Type6;
  typedef T7  Type7;
  typedef T8  Type8;
  typedef T9  Type9;
  typedef T10 Type10;
  typedef T11 Type11;
  typedef T12 Type12;
  typedef T13 Type13;

  static constexpr u8 NUM_ELEMENTS = 14;

  T0  v0{};
  T1  v1{};
  T2  v2{};
  T3  v3{};
  T4  v4{};
  T5  v5{};
  T6  v6{};
  T7  v7{};
  T8  v8{};
  T9  v9{};
  T10 v10{};
  T11 v11{};
  T12 v12{};
  T13 v13{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12, typename T13>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12,
      T13) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12, typename T13, typename T14>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>
{
  typedef T0  Type0;
  typedef T1  Type1;
  typedef T2  Type2;
  typedef T3  Type3;
  typedef T4  Type4;
  typedef T5  Type5;
  typedef T6  Type6;
  typedef T7  Type7;
  typedef T8  Type8;
  typedef T9  Type9;
  typedef T10 Type10;
  typedef T11 Type11;
  typedef T12 Type12;
  typedef T13 Type13;
  typedef T14 Type14;

  static constexpr u8 NUM_ELEMENTS = 15;

  T0  v0{};
  T1  v1{};
  T2  v2{};
  T3  v3{};
  T4  v4{};
  T5  v5{};
  T6  v6{};
  T7  v7{};
  T8  v8{};
  T9  v9{};
  T10 v10{};
  T11 v11{};
  T12 v12{};
  T13 v13{};
  T14 v14{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12, typename T13, typename T14>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14)
    -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>;

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12, typename T13, typename T14,
          typename T15>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14,
             T15>
{
  typedef T0  Type0;
  typedef T1  Type1;
  typedef T2  Type2;
  typedef T3  Type3;
  typedef T4  Type4;
  typedef T5  Type5;
  typedef T6  Type6;
  typedef T7  Type7;
  typedef T8  Type8;
  typedef T9  Type9;
  typedef T10 Type10;
  typedef T11 Type11;
  typedef T12 Type12;
  typedef T13 Type13;
  typedef T14 Type14;
  typedef T15 Type15;

  static constexpr u8 NUM_ELEMENTS = 16;

  T0  v0{};
  T1  v1{};
  T2  v2{};
  T3  v3{};
  T4  v4{};
  T5  v5{};
  T6  v6{};
  T7  v7{};
  T8  v8{};
  T9  v9{};
  T10 v10{};
  T11 v11{};
  T12 v12{};
  T13 v13{};
  T14 v14{};
  T15 v15{};
};

template <typename T0, typename T1, typename T2, typename T3, typename T4,
          typename T5, typename T6, typename T7, typename T8, typename T9,
          typename T10, typename T11, typename T12, typename T13, typename T14,
          typename T15>
Tuple(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14,
      T15) -> Tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,
                    T14, T15>;

template <unsigned int Index, typename Tuple>
constexpr auto &&impl_get(Tuple &&tuple)
{
  static_assert(Index < remove_ref<Tuple>::NUM_ELEMENTS,
                "Index of Tuple Elements out of bounds");
  if constexpr (Index == 0)
  {
    return tuple.v0;
  }
  if constexpr (Index == 1)
  {
    return tuple.v1;
  }
  if constexpr (Index == 2)
  {
    return tuple.v2;
  }
  if constexpr (Index == 3)
  {
    return tuple.v3;
  }
  if constexpr (Index == 4)
  {
    return tuple.v4;
  }
  if constexpr (Index == 5)
  {
    return tuple.v5;
  }
  if constexpr (Index == 6)
  {
    return tuple.v6;
  }
  if constexpr (Index == 7)
  {
    return tuple.v7;
  }
  if constexpr (Index == 8)
  {
    return tuple.v8;
  }
  if constexpr (Index == 9)
  {
    return tuple.v9;
  }
  if constexpr (Index == 10)
  {
    return tuple.v10;
  }
  if constexpr (Index == 11)
  {
    return tuple.v11;
  }
  if constexpr (Index == 12)
  {
    return tuple.v12;
  }
  if constexpr (Index == 13)
  {
    return tuple.v13;
  }
  if constexpr (Index == 14)
  {
    return tuple.v14;
  }
  if constexpr (Index == 15)
  {
    return tuple.v15;
  }
}

template <u8 Index, typename... T>
constexpr auto const &get(Tuple<T...> const &tuple)
{
  return impl_get<Index>(tuple);
}

template <u8 Index, typename... T>
constexpr auto &get(Tuple<T...> &tuple)
{
  return impl_get<Index>(tuple);
}

template <u8 Index, typename... T>
constexpr auto const &&get(Tuple<T...> const &&tuple)
{
  return impl_get<Index>(tuple);
}

template <u8 Index, typename... T>
constexpr auto &&get(Tuple<T...> &&tuple)
{
  return impl_get<Index>(tuple);
}

template <typename Fn, typename Tuple>
constexpr void impl_apply(Fn &&op, Tuple &&tuple)
{
  constexpr u8 NUM_ELEMENTS = remove_ref<Tuple>::NUM_ELEMENTS;
  if constexpr (NUM_ELEMENTS > 0)
  {
    op(tuple.v0);
  }
  if constexpr (NUM_ELEMENTS > 1)
  {
    op(tuple.v1);
  }
  if constexpr (NUM_ELEMENTS > 2)
  {
    op(tuple.v2);
  }
  if constexpr (NUM_ELEMENTS > 3)
  {
    op(tuple.v3);
  }
  if constexpr (NUM_ELEMENTS > 4)
  {
    op(tuple.v4);
  }
  if constexpr (NUM_ELEMENTS > 5)
  {
    op(tuple.v5);
  }
  if constexpr (NUM_ELEMENTS > 6)
  {
    op(tuple.v6);
  }
  if constexpr (NUM_ELEMENTS > 7)
  {
    op(tuple.v7);
  }
  if constexpr (NUM_ELEMENTS > 8)
  {
    op(tuple.v8);
  }
  if constexpr (NUM_ELEMENTS > 9)
  {
    op(tuple.v9);
  }
  if constexpr (NUM_ELEMENTS > 10)
  {
    op(tuple.v10);
  }
  if constexpr (NUM_ELEMENTS > 11)
  {
    op(tuple.v11);
  }
  if constexpr (NUM_ELEMENTS > 12)
  {
    op(tuple.v12);
  }
  if constexpr (NUM_ELEMENTS > 13)
  {
    op(tuple.v13);
  }
  if constexpr (NUM_ELEMENTS > 14)
  {
    op(tuple.v14);
  }
  if constexpr (NUM_ELEMENTS > 15)
  {
    op(tuple.v15);
  }
}

template <typename Fn, typename... T>
constexpr void apply(Fn &&op, Tuple<T...> const &tuple)
{
  impl_apply((Fn &&) op, tuple);
}

template <typename Fn, typename... T>
constexpr void apply(Fn &&op, Tuple<T...> &tuple)
{
  impl_apply((Fn &&) op, tuple);
}

template <typename Fn, typename... T>
constexpr void apply(Fn &&op, Tuple<T...> const &&tuple)
{
  impl_apply((Fn &&) op, tuple);
}

template <typename Fn, typename... T>
constexpr void apply(Fn &&op, Tuple<T...> &&tuple)
{
  impl_apply((Fn &&) op, tuple);
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

ASH_DEFINE_ENUM_BIT_OPS(Axes)

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

constexpr Vec3 to_vec3(Vec2 xy, f32 z)
{
  return Vec3{xy.x, xy.y, z};
}

constexpr Vec3 to_vec3(f32 x, Vec2 yz)
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

constexpr Vec4 to_vec4(Vec3 xyz, f32 w)
{
  return Vec4{xyz.x, xyz.y, xyz.z, w};
}

constexpr Vec4 to_vec4(f32 x, Vec3 yzw)
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
        .x = x / 255.0f, .y = y / 255.0f, .z = z / 255.0f, .w = w / 255.0f};
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

constexpr Vec2U operator-(Vec2U a)
{
  return Vec2U{-a.x, -a.y};
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
  f32 const y = std::bit_cast<f32>(0x5f3759df - (std::bit_cast<u32>(num) >> 1));
  return y * (1.5f - (num * 0.5f * y * y));
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

struct Slice
{
  usize offset = 0;
  usize span   = 0;

  constexpr usize end() const
  {
    return offset + span;
  }

  constexpr Slice resolve(usize size) const
  {
    // written such that overflow will not occur even if both offset and span
    // are set to USIZE_MAX
    usize o = offset > size ? size : offset;
    usize s = ((size - o) > span) ? span : size - o;
    return Slice{o, s};
  }

  constexpr bool is_empty() const
  {
    return span == 0;
  }
};

struct Slice32
{
  u32 offset = 0;
  u32 span   = 0;

  constexpr usize end() const
  {
    return offset + span;
  }

  constexpr Slice32 resolve(u32 size) const
  {
    // written such that overflow will not occur even if both offset and span
    // are set to U32_MAX
    u32 o = offset > size ? size : offset;
    u32 s = ((size - o) > span) ? span : size - o;
    return Slice32{o, s};
  }

  constexpr bool is_empty() const
  {
    return span == 0;
  }

  constexpr operator Slice() const
  {
    return Slice{offset, span};
  }
};

struct Slice64
{
  u64 offset = 0;
  u64 span   = 0;

  constexpr u64 end() const
  {
    return offset + span;
  }

  constexpr Slice64 resolve(u64 size) const
  {
    // written such that overflow will not occur even if both offset and span
    // are set to U64_MAX
    u64 o = offset > size ? size : offset;
    u64 s = ((size - o) > span) ? span : size - o;
    return Slice64{o, s};
  }

  constexpr bool is_empty() const
  {
    return span == 0;
  }

  constexpr operator Slice() const
  {
    return Slice{offset, span};
  }
};

template <typename T, usize N>
constexpr T *begin(T (&a)[N])
{
  return a;
}

template <typename T>
constexpr auto begin(T &&a) -> decltype(a.begin())
{
  return a.begin();
}

template <typename T, usize N>
constexpr T *end(T (&a)[N])
{
  return a + N;
}

template <typename T>
constexpr auto end(T &&a) -> decltype(a.end())
{
  return a.end();
}

template <typename T, usize N>
constexpr T *data(T (&a)[N])
{
  return a;
}

template <typename T>
constexpr auto data(T &&a) -> decltype(a.data())
{
  return a.data();
}

template <typename T, usize N>
constexpr usize size(T (&)[N])
{
  return N;
}

template <typename T>
constexpr auto size(T &&a) -> decltype(a.size())
{
  return a.size();
}

template <typename T>
constexpr auto is_empty(T &&a)
{
  return size(a) == 0;
}

template <typename It>
concept InputIterator = requires(It it) {
  {
    // deref
    *it
  };
  {
    // advance
    it = it++
  };
  {
    // advance
    it = ++it
  };
  {
    // distance
    it = it + (it - it)
  };
  {
    // equal
    (it == it) && true
  };
  {
    // not equal
    (it != it) && true
  };
};

template <typename It>
concept OutputIterator = InputIterator<It> && requires(It it) {
  {
    // assign
    *it = *it
  };
};

template <typename R>
concept InputRange = requires(R r) {
  { begin(r) } -> InputIterator;
  { end(r) } -> InputIterator;
};

template <typename R>
concept OutputRange = requires(R r) {
  { begin(r) } -> OutputIterator;
  { end(r) } -> OutputIterator;
};

template <typename T, usize N>
struct Array
{
  using Type                  = T;
  static constexpr usize SIZE = N;

  T data_[SIZE]{};

  constexpr bool is_empty() const
  {
    return false;
  }

  constexpr T *data()
  {
    return data_;
  }

  constexpr T const *data() const
  {
    return data_;
  }

  constexpr usize size() const
  {
    return SIZE;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * SIZE;
  }

  constexpr T const *begin() const
  {
    return data_;
  }

  constexpr T *begin()
  {
    return data_;
  }

  constexpr T const *end() const
  {
    return data_ + SIZE;
  }

  constexpr T *end()
  {
    return data_ + SIZE;
  }

  constexpr T &operator[](usize index)
  {
    return data_[index];
  }

  constexpr T const &operator[](usize index) const
  {
    return data_[index];
  }

  constexpr operator T const *() const
  {
    return data_;
  }

  constexpr operator T *()
  {
    return data_;
  }
};

template <typename T>
struct Span
{
  using Type = T;

  T    *data_ = nullptr;
  usize size_ = 0;

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T *data() const
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

  constexpr T *begin() const
  {
    return data_;
  }

  constexpr T *end() const
  {
    return data_ + size_;
  }

  constexpr T &operator[](usize index) const
  {
    return data_[index];
  }

  constexpr operator Span<T const>() const
  {
    return Span<T const>{data_, size_};
  }

  constexpr Span<T const> as_const() const
  {
    return Span<T const>{data_, size_};
  }

  constexpr Span<u8 const> as_u8() const
  {
    return Span<u8 const>{reinterpret_cast<u8 const *>(data_), size_bytes()};
  }

  constexpr Span<char const> as_char() const
  {
    return Span<char const>{reinterpret_cast<char const *>(data_),
                            size_bytes()};
  }

  constexpr Span slice(usize offset, usize span) const
  {
    return slice(Slice{offset, span});
  }

  constexpr Span slice(Slice s) const
  {
    s = s.resolve(size_);
    return Span{data_ + s.offset, s.span};
  }

  constexpr Span slice(usize offset) const
  {
    return slice(offset, USIZE_MAX);
  }
};

constexpr Span<char const> operator""_span(char const *lit, usize n)
{
  return Span<char const>{lit, n};
}

constexpr Span<char8_t const> operator""_span(char8_t const *lit, usize n)
{
  return Span<char8_t const>{lit, n};
}

constexpr Span<char16_t const> operator""_span(char16_t const *lit, usize n)
{
  return Span<char16_t const>{lit, n};
}

constexpr Span<char32_t const> operator""_span(char32_t const *lit, usize n)
{
  return Span<char32_t const>{lit, n};
}

constexpr Span<u8 const> utf(Span<char8_t const> s)
{
  return Span<u8 const>{(u8 const *) s.data_, s.size_};
}

constexpr Span<u32 const> utf(Span<char32_t const> s)
{
  return Span<u32 const>{(u32 const *) s.data_, s.size_};
}

/// @param index max of Rep::NUM_BITS - 1
template <typename RepT>
struct BitRef
{
  using Rep = RepT;

  Rep *pack      = nullptr;
  u16  bit_index = 0;

  template <typename U>
  constexpr BitRef const &operator=(BitRef<U> bit) const
    requires(OutputIterator<Rep *>)
  {
    return (*this = static_cast<bool>(bit));
  }

  constexpr BitRef const &operator=(BitRef const &bit) const
    requires(OutputIterator<Rep *>)
  {
    return (*this = static_cast<bool>(bit));
  }

  constexpr BitRef const &operator=(bool bit) const
    requires(OutputIterator<Rep *>)
  {
    *pack = (*pack & ~(((Rep) 1) << bit_index)) | (((Rep) (bit)) << bit_index);
    return *this;
  }

  constexpr operator bool() const
  {
    return (*pack >> bit_index) & 1;
  }

  constexpr operator BitRef<Rep const>() const
  {
    return BitRef<Rep const>{pack, bit_index};
  }

  bool operator|(bool other) const
  {
    return this->operator bool() || other;
  }

  bool operator&(bool other) const
  {
    return this->operator bool() && other;
  }

  bool operator~() const
  {
    return !(this->operator bool());
  }
};

/// @param data is never changed
template <typename RepT>
struct BitIterator
{
  using Rep  = RepT;
  using Type = bool;

  Rep  *data  = nullptr;
  usize index = 0;

  constexpr BitIterator operator+(usize advance) const
  {
    return BitIterator{data, index + advance};
  }

  constexpr BitIterator operator-(usize advance) const
  {
    return BitIterator{data, index - advance};
  }

  constexpr BitIterator &operator++()
  {
    index++;
    return *this;
  }

  constexpr BitIterator operator++(int)
  {
    BitIterator out{data, index};
    index++;
    return out;
  }

  constexpr BitIterator &operator--()
  {
    index--;
    return *this;
  }

  constexpr BitIterator operator--(int)
  {
    BitIterator out{data, index};
    index--;
    return out;
  }

  constexpr operator BitIterator<Rep const>() const
  {
    return BitIterator{data, index};
  }
};

template <typename Rep>
constexpr bool operator==(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.index == b.index;
}

template <typename Rep>
constexpr bool operator!=(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.index != b.index;
}

template <typename Rep>
constexpr bool operator<(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.index < b.index;
}

template <typename Rep>
constexpr bool operator>(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.index > b.index;
}

template <typename Rep>
constexpr BitRef<Rep> operator*(BitIterator<Rep> it)
{
  constexpr u16 INDEX_SHIFT = NumTraits<Rep>::LOG2_NUM_BITS;
  constexpr u16 INDEX_MASK  = NumTraits<Rep>::NUM_BITS - 1;
  return BitRef{it.data + (it.index >> INDEX_SHIFT),
                static_cast<u16>(it.index & INDEX_MASK)};
}

/// UB if not pointing to the same data
template <typename Rep>
constexpr usize operator-(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.index - b.index;
}

/// no slice support
template <typename RepT>
struct BitSpan
{
  using Rep  = RepT;
  using Type = bool;

  Rep  *data_     = nullptr;
  usize num_bits_ = 0;

  constexpr BitRef<Rep> operator[](usize index) const
  {
    constexpr u16 INDEX_SHIFT = NumTraits<Rep>::LOG2_NUM_BITS;
    constexpr u16 INDEX_MASK  = NumTraits<Rep>::NUM_BITS - 1;
    return BitRef{data_ + (index >> INDEX_SHIFT),
                  static_cast<u16>(index & INDEX_MASK)};
  }

  constexpr operator BitSpan<Rep const>() const
  {
    return BitSpan<Rep const>{data_, num_bits_};
  }

  constexpr BitIterator<Rep> begin() const
  {
    return BitIterator<Rep>{data_, 0};
  }

  constexpr BitIterator<Rep> end() const
  {
    return BitIterator<Rep>{data_, num_bits_};
  }

  constexpr bool is_empty() const
  {
    return num_bits_ == 0;
  }
};

template <typename T>
constexpr Span<T const> to_span(std::initializer_list<T> list)
{
  return Span<T const>{list.begin(), list.size()};
}

template <typename T, usize N>
constexpr Span<T> to_span(T (&array)[N])
{
  return Span<T>{array, N};
}

template <typename Container>
constexpr auto to_span(Container &c) -> decltype(Span{data(c), size(c)})
{
  return Span{data(c), size(c)};
}

template <typename Lambda>
struct defer
{
  Lambda lambda;
  constexpr defer(defer &&)                 = delete;
  constexpr defer(defer const &)            = delete;
  constexpr defer &operator=(defer &&)      = delete;
  constexpr defer &operator=(defer const &) = delete;
  constexpr defer(Lambda &&l) : lambda{(Lambda &&) l}
  {
  }
  constexpr ~defer()
  {
    lambda();
  }
};

template <typename Lambda>
defer(Lambda &&) -> defer<Lambda>;

struct Noop
{
  constexpr void operator()(auto &&...) const
  {
  }
};

struct Add
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a + b;
  }
};

struct Sub
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a - b;
  }
};

struct Mul
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a * b;
  }
};

struct Div
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a / b;
  }
};

struct Equal
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a == b;
  }
};

struct NotEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a != b;
  }
};

struct Lesser
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a < b;
  }
};

struct LesserOrEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a <= b;
  }
};

struct Greater
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a > b;
  }
};

struct GreaterOrEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a >= b;
  }
};

struct Compare
{
  constexpr int operator()(auto const &a, auto const &b) const
  {
    if (a == b)
    {
      return 0;
    }
    if (a > b)
    {
      return -1;
    }
    return 1;
  }
};

struct Min
{
  template <typename T>
  constexpr T const &operator()(T const &a, T const &b) const
  {
    return a < b ? a : b;
  }
};

struct Max
{
  template <typename T>
  constexpr auto const &operator()(T const &a, T const &b) const
  {
    return a > b ? a : b;
  }
};

struct Swap
{
  template <typename T>
  constexpr void operator()(T &a, T &b) const
  {
    T a_tmp{(T &&) a};
    a = (T &&) b;
    b = (T &&) a_tmp;
  }
};

struct Clamp
{
  template <typename T>
  constexpr T const &operator()(T const &value, T const &min,
                                T const &max) const
  {
    return value < min ? min : (value > max ? max : value);
  }
};

constexpr Noop           noop;
constexpr Add            add;
constexpr Sub            sub;
constexpr Mul            mul;
constexpr Div            div;
constexpr Equal          equal;
constexpr NotEqual       not_equal;
constexpr Lesser         lesser;
constexpr LesserOrEqual  lesser_or_equal;
constexpr Greater        greater;
constexpr GreaterOrEqual greater_or_equal;
constexpr Compare        compare;
constexpr Min            min;
constexpr Max            max;
constexpr Swap           swap;
constexpr Clamp          clamp;

/// Fn is a function handle and doesn't manage any lifetime.
///
template <typename Sig>
struct Fn;

template <typename R, typename... Args>
struct Fn<R(Args...)>
{
  using Dispatcher = R (*)(void *, Args...);

  constexpr R operator()(Args... args) const
  {
    return dispatcher(data, static_cast<Args &&>(args)...);
  }

  Dispatcher dispatcher = nullptr;
  void      *data       = nullptr;
};

template <typename R, typename... Args>
struct PFnDispatcher
{
  static constexpr R dispatch(void *data, Args... args)
  {
    using PFn = R (*)(Args...);

    PFn pfn = reinterpret_cast<PFn>(data);

    return pfn(static_cast<Args &&>(args)...);
  }
};

template <typename Sig>
struct PFnTraits;

template <typename R, typename... Args>
struct PFnTraits<R(Args...)>
{
  using Ptr        = R (*)(Args...);
  using Signature  = R(Args...);
  using Fn         = Fn<Signature>;
  using ReturnType = R;
  using Dispatcher = PFnDispatcher<R, Args...>;
};

template <typename R, typename... Args>
struct PFnTraits<R (*)(Args...)> : public PFnTraits<R(Args...)>
{
};

template <typename T, typename R, typename... Args>
struct FunctorDispatcher
{
  static constexpr R dispatch(void *data, Args... args)
  {
    return (*(reinterpret_cast<T *>(data)))(static_cast<Args &&>(args)...);
  }
};

template <class Sig>
struct MemberFnTraits
{
};

// non-const member functions
template <class T, typename R, typename... Args>
struct MemberFnTraits<R (T::*)(Args...)>
{
  using Ptr        = R (*)(Args...);
  using Signature  = R(Args...);
  using Fn         = Fn<Signature>;
  using Type       = T;
  using ReturnType = R;
  using Dispatcher = FunctorDispatcher<T, R, Args...>;
};

// const member functions
template <class T, typename R, typename... Args>
struct MemberFnTraits<R (T::*)(Args...) const>
{
  using Ptr        = R (*)(Args...);
  using Signature  = R(Args...);
  using Fn         = Fn<Signature>;
  using Type       = T const;
  using ReturnType = R;
  using Dispatcher = FunctorDispatcher<T const, R, Args...>;
};

template <class T>
struct FunctorTraits : public MemberFnTraits<decltype(&T::operator())>
{
};

// make a function view from a raw function pointer.
template <typename R, typename... Args>
auto to_fn(R (*pfn)(Args...))
{
  using Traits     = PFnTraits<R(Args...)>;
  using Fn         = typename Traits::Fn;
  using Dispatcher = typename Traits::Dispatcher;

  return Fn{&Dispatcher::dispatch, reinterpret_cast<void *>(pfn)};
}

/// make a function view from a non-capturing functor (i.e. lambda's without
/// data)
template <typename StaticFunctor>
auto to_fn(StaticFunctor functor)
{
  using Traits = FunctorTraits<StaticFunctor>;
  using PFn    = typename Traits::Ptr;

  PFn pfn = static_cast<PFn>(functor);

  return to_fn(pfn);
}

/// make a function view from a functor reference. Functor should outlive the Fn
template <typename Functor>
auto fn(Functor *functor)
{
  using Traits     = FunctorTraits<Functor>;
  using Fn         = typename Traits::Fn;
  using Dispatcher = typename Traits::Dispatcher;

  return Fn{&Dispatcher::dispatch,
            const_cast<void *>(reinterpret_cast<void const *>(functor))};
}

template <typename T, typename R, typename... Args>
auto fn(T *t, R (*fn)(T *, Args...))
{
  return Fn<R(Args...)>{reinterpret_cast<R (*)(void *, Args...)>(fn),
                        const_cast<void *>(reinterpret_cast<void const *>(t))};
}

template <typename T, typename StaticFunctor>
auto fn(T *t, StaticFunctor functor)
{
  using Traits = FunctorTraits<StaticFunctor>;
  using PFn    = typename Traits::Ptr;

  PFn pfn = static_cast<PFn>(functor);

  return fn(t, pfn);
}

///
/// The `SourceLocation`  class represents certain information about the source
/// code, such as file names, line numbers, and function names. Previously,
/// functions that desire to obtain this information about the call site (for
/// logging, testing, or debugging purposes) must use macros so that predefined
/// macros like `__LINE__` and `__FILE__` are expanded in the context of the
/// caller. The `SourceLocation` class provides a better alternative.
///
///
/// based on: https://en.cppreference.com/w/cpp/utility/source_location
///
struct [[nodiscard]] SourceLocation
{
  static constexpr SourceLocation current(
#if ASH_HAS_BUILTIN(FILE) || (defined(__cpp_lib_source_location) && \
                              __cpp_lib_source_location >= 201907L)
      char const *file = __builtin_FILE(),
#elif defined(__FILE__)
      char const *file = __FILE__,
#else
      char const *file = "unknown",
#endif

#if ASH_HAS_BUILTIN(FUNCTION) || (defined(__cpp_lib_source_location) && \
                                  __cpp_lib_source_location >= 201907L)
      char const *function = __builtin_FUNCTION(),
#else
      char const *function = "unknown",
#endif

#if ASH_HAS_BUILTIN(LINE) || (defined(__cpp_lib_source_location) && \
                              __cpp_lib_source_location >= 201907L)
      u32 line = __builtin_LINE(),
#elif defined(__LINE__)
      u32 line = __LINE__,
#else
      u32 line = 0,
#endif

#if ASH_HAS_BUILTIN(COLUMN) || (defined(__cpp_lib_source_location) && \
                                __cpp_lib_source_location >= 201907L)
      u32 column = __builtin_COLUMN()
#else
      u32 column = 0
#endif
  )
  {
    return SourceLocation{file, function, line, column};
  }

  char const *file     = "";
  char const *function = "";
  u32         line     = 0;
  u32         column   = 0;
};

}        // namespace ash
