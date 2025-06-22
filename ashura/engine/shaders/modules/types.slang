/// SPDX-License-Identifier: MIT

#pragma once

typedef uint8_t u8;
typedef int8_t  i8;

typedef uint16_t u16;
typedef int16_t  i16;

typedef uint32_t u32;
typedef int32_t  i32;

typedef float16_t    f16;
typedef float16_t4   f16x4;
typedef float16_t3   f16x3;
typedef float16_t2   f16x2;
typedef float16_t4x4 f16x4x4;
typedef float16_t3x3 f16x3x3;
typedef float16_t2x2 f16x2x2;

typedef float32_t    f32;
typedef float32_t4   f32x4;
typedef float32_t3   f32x3;
typedef float32_t2   f32x2;
typedef float32_t4x4 f32x4x4;
typedef float32_t3x3 f32x3x3;
typedef float32_t2x2 f32x2x2;

typedef u32 SamplerId;
typedef u32 TextureId;

static const f32 PI = 3.1415926535897932384626433832795;

static const f32 GAMMA = 2.2;

static const f32 EPSILON = 0.00000011920929;

enum class BlendMode : u32
{
  Clear      = 0,
  Src        = 1,
  Dst        = 2,
  SrcOver    = 3,
  DstOver    = 4,
  SrcIn      = 5,
  DstIn      = 6,
  SrcOut     = 7,
  DstOut     = 8,
  SrcAtop    = 9,
  DstAtop    = 10,
  Xor        = 11,
  Plus       = 12,
  Modulate   = 13,
  Screen     = 14,
  Overlay    = 15,
  Darken     = 16,
  Lighten    = 17,
  ColorDodge = 18,
  ColorBurn  = 19,
  HardLight  = 20,
  SoftLight  = 21,
  Difference = 22,
  Exclusion  = 23,
  Multiply   = 24,
  Hue        = 25,
  Saturation = 26,
  Color      = 27,
  Luminosity = 28
};

enum class BezierRegions : u32
{
  None    = 0,
  Inside  = 1,
  Outside = 2,
  All     = 3
};
