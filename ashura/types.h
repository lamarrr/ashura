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

constexpr u8 U8_MIN = 0;
constexpr u8 U8_MAX = 0xFFU;

constexpr i8 I8_MIN = -0x7F - 1;
constexpr i8 I8_MAX = 0x7F;

constexpr u16 U16_MIN = 0;
constexpr u16 U16_MAX = 0xFFFFU;

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

constexpr usize MAX_STANDARD_ALIGNMENT = alignof(max_align_t);

constexpr f32 PI = 3.14159265358979323846f;

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

typedef struct Vec2       Vec2;
typedef struct Vec2       Complex;
typedef struct Vec3       Vec3;
typedef struct Vec4       Vec4;
typedef struct Vec4       Quaternion;
typedef struct Vec4       Color;
typedef struct Vec4U8     Vec4U8;
typedef struct Vec4U8     ColorU8;
typedef struct Vec2I      Vec2I;
typedef struct Vec3I      Vec3I;
typedef struct Vec2U      Vec2U;
typedef struct Vec3U      Vec3U;
typedef struct Vec4U      Vec4U;
typedef struct Vec4U      ColorU;
typedef struct Mat2       Mat2;
typedef struct Mat3       Mat3;
typedef struct Mat3Affine Mat3Affine;
typedef struct Mat4       Mat4;
typedef struct Mat4Affine Mat4Affine;
typedef struct Slice      Slice;
template <typename T>
struct Span;

struct Vec2
{
  union
  {
    f32 c = 0;
    f32 i;
    f32 width;
  };

  union
  {
    f32 y = 0;
    f32 j;
    f32 height;
  };
};

struct Vec3
{
  union
  {
    f32 x = 0;
    f32 r;
    f32 width;
  };

  union
  {
    f32 y = 0;
    f32 g;
    f32 height;
  };

  union
  {
    f32 z = 0;
    f32 b;
    f32 depth;
  };
};

struct Vec4
{
  union
  {
    f32 x = 0;
    f32 c;
    f32 r;
    f32 width;
  };

  union
  {
    f32 y = 0;
    f32 i;
    f32 g;
    f32 height;
  };

  union
  {
    f32 z = 0;
    f32 j;
    f32 b;
    f32 depth;
  };

  union
  {
    f32 w = 0;
    f32 k;
    f32 a;
    f32 hyper;
  };
};

struct Vec4U8
{
  union
  {
    u8 x = 0;
    u8 r;
    u8 width;
  };

  union
  {
    u8 y = 0;
    u8 g;
    u8 height;
  };

  union
  {
    u8 z = 0;
    u8 b;
    u8 depth;
  };

  union
  {
    u8 w = 0;
    u8 a;
    u8 hyper;
  };
};

struct Vec2I
{
  union
  {
    i32 x = 0;
    i32 width;
  };

  union
  {
    i32 y = 0;
    i32 height;
  };
};

struct Vec3I
{
  union
  {
    i32 x = 0;
    i32 width;
  };

  union
  {
    i32 y = 0;
    i32 height;
  };

  union
  {
    i32 z = 0;
    i32 depth;
  };
};

struct Vec2U
{
  union
  {
    u32 x = 0;
    u32 width;
  };

  union
  {
    u32 y = 0;
    u32 height;
  };
};

struct Vec3U
{
  union
  {
    u32 x = 0;
    u32 r;
    u32 width;
  };

  union
  {
    u32 y = 0;
    u32 g;
    u32 height;
  };

  union
  {
    u32 z = 0;
    u32 b;
    u32 depth;
  };
};

struct Vec4U
{
  union
  {
    u32 x = 0;
    u32 r;
    u32 width;
  };

  union
  {
    u32 y = 0;
    u32 g;
    u32 height;
  };

  union
  {
    u32 z = 0;
    u32 b;
    u32 depth;
  };

  union
  {
    u32 w = 0;
    u32 a;
    u32 hyper;
  };
};

struct Mat2
{
  Vec2 rows[2] = {};
};

struct Mat3
{
  Vec3 rows[3] = {};
};

struct Mat3Affine
{
  Vec3 rows[2] = {};
};

struct Mat4
{
  Vec4 rows[3] = {};
};

struct Mat4Affine
{
  Vec4 rows[3] = {};
};

struct Slice
{
  usize offset = 0;
  usize size   = 0;
};

}        // namespace ash
