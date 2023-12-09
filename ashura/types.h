#pragma once
#include <cfloat>
#include <cinttypes>
#include <cstddef>
#include <cstdint>

namespace ash
{

using u8    = uint8_t;
using u16   = uint16_t;
using u32   = uint32_t;
using u64   = uint64_t;
using i8    = int8_t;
using i16   = int16_t;
using i32   = int32_t;
using i64   = int64_t;
using f32   = float;
using f64   = double;
using usize = size_t;
using isize = ptrdiff_t;

constexpr u8    U8_MIN           = 0;
constexpr u16   U16_MIN          = 0;
constexpr u32   U32_MIN          = 0;
constexpr u64   u64_MIN          = 0;
constexpr u8    U8_MAX           = 0xFFU;
constexpr u16   U16_MAX          = 0xFFFFU;
constexpr u32   U32_MAX          = 0xFFFFFFFFU;
constexpr u64   U64_MAX          = 0xFFFFFFFFFFFFFFFFULL;
constexpr i8    I8_MIN           = -0x7F - 1;
constexpr i16   I16_MIN          = -0x7FFF - 1;
constexpr i32   I32_MIN          = -0x7FFFFFFF - 1;
constexpr i64   I64_MIN          = -0x7FFFFFFFFFFFFFFFLL - 1;
constexpr i8    I8_MAX           = 0x7F;
constexpr i16   I16_MAX          = 0x7FFF;
constexpr i32   I32_MAX          = 0x7FFFFFFF;
constexpr i64   I64_MAX          = 0x7FFFFFFFFFFFFFFFLL;
constexpr f32   F32_MIN          = -FLT_MAX;
constexpr f64   F64_MIN          = -DBL_MAX;
constexpr f32   F32_MIN_POSITIVE = FLT_MIN;
constexpr f64   F64_MIN_POSITIVE = DBL_MIN;
constexpr f32   F32_MAX          = FLT_MAX;
constexpr f64   F64_MAX          = DBL_MAX;
constexpr usize USIZE_MIN        = 0;
constexpr usize USIZE_MAX        = (usize) -1;
constexpr isize ISIZE_MIN        = PTRDIFF_MIN;
constexpr isize ISIZE_MAX        = PTRDIFF_MAX;

}        // namespace ash
