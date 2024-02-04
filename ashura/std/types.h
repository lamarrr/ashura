#pragma once
#include <cfloat>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

namespace ash
{
typedef uint8_t           u8;
typedef uint16_t          u16;
typedef uint32_t          u32;
typedef uint64_t          u64;
typedef int8_t            i8;
typedef int16_t           i16;
typedef int32_t           i32;
typedef int64_t           i64;
typedef float             f32;
typedef double            f64;
typedef size_t            usize;
typedef ptrdiff_t         isize;
typedef u8                uid8;
typedef u16               uid16;
typedef u32               uid32;
typedef u64               uid64;
typedef struct Void       Void;
typedef struct Vec2       Vec2;
typedef struct Vec3       Vec3;
typedef struct Vec4       Vec4;
typedef struct Vec4U8     Vec4U8;
typedef struct Vec2I      Vec2I;
typedef struct Vec3I      Vec3I;
typedef struct Vec2U      Vec2U;
typedef struct Vec3U      Vec3U;
typedef struct Vec4U      Vec4U;
typedef struct Mat2       Mat2;
typedef struct Mat3       Mat3;
typedef struct Mat3Affine Mat3Affine;
typedef struct Mat4       Mat4;
typedef struct Mat4Affine Mat4Affine;
typedef struct Slice      Slice;
template <typename T>
struct Span;
template <typename RepT>
struct BitSpan;

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

constexpr uid8  INVALID_UID8  = U8_MAX;
constexpr uid16 INVALID_UID16 = U16_MAX;
constexpr uid32 INVALID_UID32 = U32_MAX;
constexpr uid64 INVALID_UID64 = U64_MAX;

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

struct alignas(8) Vec2
{
  f32 x = 0;
  f32 y = 0;

  static constexpr Vec2 uniform(f32 value)
  {
    return Vec2{value, value};
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

  static constexpr Vec3 uniform(f32 value)
  {
    return Vec3{value, value, value};
  }
};

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

  static constexpr Vec4 uniform(f32 value)
  {
    return Vec4{value, value, value, value};
  }
};

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

constexpr Vec2I operator-(Vec2I a, Vec2I b)
{
  return Vec2I{a.x - b.x, a.y - b.y};
}

constexpr Vec2I operator*(Vec2I a, Vec2I b)
{
  return Vec2I{a.x * b.x, a.y * b.y};
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
};

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

constexpr Vec2U operator-(Vec2U a, Vec2U b)
{
  return Vec2U{a.x - b.x, a.y - b.y};
}

constexpr Vec2U operator*(Vec2U a, Vec2U b)
{
  return Vec2U{a.x * b.x, a.y * b.y};
}

constexpr Vec2U operator/(Vec2U a, Vec2U b)
{
  return Vec2U{a.x / b.x, a.y / b.y};
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

struct Mat2
{
  Vec2 rows[2] = {};

  static constexpr Mat2 uniform(f32 value)
  {
    return Mat2{.rows = {{value, value}, {value, value}}};
  }

  static constexpr Mat2 diagonal(f32 value)
  {
    return Mat2{.rows = {{value, 0}, {0, value}}};
  }

  static constexpr Mat2 identity_mat2()
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

  static constexpr Mat3 uniform(f32 value)
  {
    return Mat3{.rows = {{value, value, value},
                         {value, value, value},
                         {value, value, value}}};
  }

  static constexpr Mat3 diagonal(f32 value)
  {
    return Mat3{.rows = {{value, 0, 0}, {0, value, 0}, {0, 0, value}}};
  }

  static constexpr Mat3 identity_mat3()
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

  explicit constexpr operator Mat3() const
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

  static constexpr Mat4 uniform(f32 value)
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

  explicit constexpr operator Mat4() const
  {
    return Mat4{.rows = {rows[0], rows[1], rows[2], {0, 0, 0, 1}}};
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
constexpr usize size(T (&a)[N])
{
  return N;
}

template <typename T>
constexpr auto size(T &&a) -> decltype(a.size())
{
  return a.size();
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
  {
    begin(r)
  } -> InputIterator;
  {
    end(r)
  } -> InputIterator;
};

template <typename R>
concept OutputRange = requires(R r) {
  {
    begin(r)
  } -> OutputIterator;
  {
    end(r)
  } -> OutputIterator;
};

template <typename T>
struct Span
{
  T    *m_data = nullptr;
  usize m_size = 0;

  constexpr bool is_empty() const
  {
    return m_size == 0;
  }

  constexpr T *data() const
  {
    return m_data;
  }

  constexpr usize size() const
  {
    return m_size;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * m_size;
  }

  constexpr T *begin() const
  {
    return m_data;
  }

  constexpr T *end() const
  {
    return m_data + m_size;
  }

  constexpr T &operator[](usize index) const
  {
    return m_data[index];
  }

  constexpr Span<T> operator[](Slice slice) const
  {
    // written such that overflow will not occur even if both offset and size
    // are set to USIZE_MAX
    slice.offset = slice.offset > m_size ? m_size : slice.offset;
    slice.span   = (m_size - slice.offset) > slice.span ? slice.span :
                                                          (m_size - slice.offset);
    return Span<T>{m_data + slice.offset, slice.span};
  }

  constexpr operator Span<T const>() const
  {
    return Span<T const>{m_data, m_size};
  }

  constexpr Span<T const> as_const() const
  {
    return Span<T const>{m_data, m_size};
  }

  constexpr Span<u8 const> as_u8() const
  {
    return Span<u8 const>{reinterpret_cast<u8 const *>(m_data), size_bytes()};
  }

  constexpr Span<char const> as_char() const
  {
    return Span<char const>{reinterpret_cast<char const *>(m_data),
                            size_bytes()};
  }

  constexpr Span slice(usize slice_offset, usize slice_span) const
  {
    return (*this)[Slice{slice_offset, slice_span}];
  }

  constexpr Span slice(usize slice_offset) const
  {
    return slice(slice_offset, USIZE_MAX);
  }
};

constexpr Span<char const> operator""_span(char const *lit, usize len)
{
  return Span<char const>{lit, len};
}

template <typename T>
constexpr Span<T const> to_span(std::initializer_list<T> init)
{
  return Span<T const>{init.begin(), init.size()};
}

template <typename T, usize N>
constexpr Span<T> to_span(T (&array)[N])
{
  return Span<T>{array, N};
}

template <typename Container>
constexpr auto to_span(Container &container)
    -> decltype(Span{data(container), size(container)})
{
  return Span{data(container), size(container)};
}

/// @index: max of Rep::NUM_BITS - 1
template <typename RepT>
struct BitRef
{
  using Rep = RepT;

  Rep *pack      = nullptr;
  u16  bit_index = 0;

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

/// @data: is never changed
template <typename RepT>
struct BitIterator
{
  using Rep = RepT;

  Rep  *data  = nullptr;
  usize index = 0;

  constexpr BitIterator operator+(usize advance) const
  {
    return BitIterator{data, index + advance};
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

  constexpr operator BitIterator<Rep const>() const
  {
    return BitIterator{data, index};
  }
};

template <typename Rep>
constexpr bool operator==(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.data == b.data && a.index == b.index;
}

template <typename Rep>
constexpr bool operator!=(BitIterator<Rep> a, BitIterator<Rep> b)
{
  return a.data != b.data || a.index != b.index;
}

template <typename Rep>
constexpr BitRef<Rep> operator*(BitIterator<Rep> it)
{
  constexpr u16 INDEX_SHIFT = NumTraits<Rep>::LOG2_NUM_BITS;
  constexpr u16 INDEX_MASK  = NumTraits<Rep>::NUM_BITS - 1;
  return BitRef{it.data + (it.index >> INDEX_SHIFT), (it.index & INDEX_MASK)};
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
  using Rep = RepT;

  Rep  *m_data     = nullptr;
  usize m_num_bits = 0;

  constexpr BitRef<Rep> operator[](usize index) const
  {
    constexpr u16 INDEX_SHIFT = NumTraits<Rep>::LOG2_NUM_BITS;
    constexpr u16 INDEX_MASK  = NumTraits<Rep>::NUM_BITS - 1;
    return BitRef{m_data + (index >> INDEX_SHIFT), (index & INDEX_MASK)};
  }

  constexpr operator BitSpan<Rep const>() const
  {
    return BitSpan<Rep const>{m_data, m_num_bits};
  }

  constexpr BitIterator<Rep> begin() const
  {
    return BitIterator<Rep>{m_data, 0};
  }

  constexpr BitIterator<Rep> end() const
  {
    return BitIterator<Rep>{m_data, m_num_bits};
  }

  constexpr bool is_empty() const
  {
    return m_num_bits == 0;
  }
};

}        // namespace ash
