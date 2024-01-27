#pragma once
#include "ashura/std/types.h"

namespace ash
{

template <typename T>
struct IntTraits;

template <>
struct IntTraits<u8>
{
  static constexpr u8   NUM_BITS = 8;
  static constexpr u8   MIN      = U8_MIN;
  static constexpr u8   MAX      = U8_MAX;
  static constexpr bool SIGNED   = false;
};

template <>
struct IntTraits<u16>
{
  static constexpr u8   NUM_BITS = 16;
  static constexpr u16  MIN      = U16_MIN;
  static constexpr u16  MAX      = U16_MAX;
  static constexpr bool SIGNED   = false;
};

template <>
struct IntTraits<u32>
{
  static constexpr u8   NUM_BITS = 32;
  static constexpr u32  MIN      = U32_MIN;
  static constexpr u32  MAX      = U32_MAX;
  static constexpr bool SIGNED   = false;
};

template <>
struct IntTraits<u64>
{
  static constexpr u8   NUM_BITS = 64;
  static constexpr u64  MIN      = U64_MIN;
  static constexpr u64  MAX      = U64_MAX;
  static constexpr bool SIGNED   = false;
};

template <>
struct IntTraits<i8>
{
  static constexpr u8   NUM_BITS = 8;
  static constexpr i8   MIN      = I8_MIN;
  static constexpr i8   MAX      = I8_MAX;
  static constexpr bool SIGNED   = true;
};

template <>
struct IntTraits<i16>
{
  static constexpr u8   NUM_BITS = 16;
  static constexpr i16  MIN      = I16_MIN;
  static constexpr i16  MAX      = I16_MAX;
  static constexpr bool SIGNED   = true;
};

template <>
struct IntTraits<i32>
{
  static constexpr u8   NUM_BITS = 32;
  static constexpr i32  MIN      = I32_MIN;
  static constexpr i32  MAX      = I32_MAX;
  static constexpr bool SIGNED   = true;
};

template <>
struct IntTraits<i64>
{
  static constexpr u8   NUM_BITS = 64;
  static constexpr i64  MIN      = I64_MIN;
  static constexpr i64  MAX      = I64_MAX;
  static constexpr bool SIGNED   = true;
};

}        // namespace ash
