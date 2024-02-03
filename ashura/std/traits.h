#pragma once
#include "ashura/std/types.h"

namespace ash
{
template <typename T>
struct RemoveConstVolatile
{
  using Type = T;
};

template <typename T>
struct RemoveConstVolatile<T const> : RemoveConstVolatile<T>
{
};

template <typename T>
struct RemoveConstVolatile<volatile T> : RemoveConstVolatile<T>
{
};

template <typename T>
using remove_const_volatile = RemoveConstVolatile<T>::Type;

template <typename T>
struct IsConstImpl
{
  static constexpr bool value = false;
};

template <typename T>
struct IsConstImpl<T const>
{
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_const = IsConstImpl<T>::value;

template <typename T>
concept Const = is_const<T>;

template <typename T>
struct IntTraitsImpl;

template <>
struct IntTraitsImpl<u8>
{
  static constexpr u8   NUM_BITS      = 8;
  static constexpr u8   LOG2_NUM_BITS = 3;
  static constexpr u8   MIN           = U8_MIN;
  static constexpr u8   MAX           = U8_MAX;
  static constexpr bool SIGNED        = false;
};

template <>
struct IntTraitsImpl<u16>
{
  static constexpr u8   NUM_BITS      = 16;
  static constexpr u8   LOG2_NUM_BITS = 4;
  static constexpr u16  MIN           = U16_MIN;
  static constexpr u16  MAX           = U16_MAX;
  static constexpr bool SIGNED        = false;
};

template <>
struct IntTraitsImpl<u32>
{
  static constexpr u8   NUM_BITS      = 32;
  static constexpr u8   LOG2_NUM_BITS = 5;
  static constexpr u32  MIN           = U32_MIN;
  static constexpr u32  MAX           = U32_MAX;
  static constexpr bool SIGNED        = false;
};

template <>
struct IntTraitsImpl<u64>
{
  static constexpr u8   NUM_BITS      = 64;
  static constexpr u8   LOG2_NUM_BITS = 6;
  static constexpr u64  MIN           = U64_MIN;
  static constexpr u64  MAX           = U64_MAX;
  static constexpr bool SIGNED        = false;
};

template <>
struct IntTraitsImpl<i8>
{
  static constexpr u8   NUM_BITS      = 8;
  static constexpr u8   LOG2_NUM_BITS = 3;
  static constexpr i8   MIN           = I8_MIN;
  static constexpr i8   MAX           = I8_MAX;
  static constexpr bool SIGNED        = true;
};

template <>
struct IntTraitsImpl<i16>
{
  static constexpr u8   NUM_BITS      = 16;
  static constexpr u8   LOG2_NUM_BITS = 4;
  static constexpr i16  MIN           = I16_MIN;
  static constexpr i16  MAX           = I16_MAX;
  static constexpr bool SIGNED        = true;
};

template <>
struct IntTraitsImpl<i32>
{
  static constexpr u8   NUM_BITS      = 32;
  static constexpr u8   LOG2_NUM_BITS = 5;
  static constexpr i32  MIN           = I32_MIN;
  static constexpr i32  MAX           = I32_MAX;
  static constexpr bool SIGNED        = true;
};

template <>
struct IntTraitsImpl<i64>
{
  static constexpr u8   NUM_BITS      = 64;
  static constexpr u8   LOG2_NUM_BITS = 6;
  static constexpr i64  MIN           = I64_MIN;
  static constexpr i64  MAX           = I64_MAX;
  static constexpr bool SIGNED        = true;
};

template <typename T>
struct IntTraits : IntTraitsImpl<remove_const_volatile<T>>
{
};

}        // namespace ash
