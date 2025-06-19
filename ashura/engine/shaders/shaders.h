/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

namespace shader
{

namespace blur
{

struct Blur
{
  alignas(8) f32x2 uv0;
  alignas(8) f32x2 uv1;
  alignas(8) f32x2 radius;
  SamplerId sampler;
  TextureId tex;
};

}    // namespace blur

enum class FillRule : u32
{
  EvenOdd = 0,
  NonZero = 1
};

enum class BezierRegions : u32
{
  None    = 0,
  Inside  = 1,
  Outside = 2,
  All     = 3
};

namespace quad
{

struct FlatMaterial
{
  alignas(16) f32x4 colors[2];
  alignas(8) f32x2 color_rotor;
  alignas(8) f32x2 uv0;
  alignas(8) f32x2 uv1;
  SamplerId sampler;
  TextureId texture;
};

struct NoiseMaterial
{
  alignas(16) f32x4 intensity;
  alignas(8) f32x2 offset;
};

}    // namespace quad

namespace ngon
{
using FlatMaterial = quad::FlatMaterial;
}

namespace pbr
{

/// @see https://github.com/KhronosGroup/glTF/tree/acfcbe65e40c53d6d3aa55a7299982bf2c01c75d/extensions/2.0/Khronos
/// @see
/// https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/63b7c128266cfd86bbd3f25caf8b3db3fe854015/source/Renderer/shaders/textures.glsl#L1
struct BaseMaterial
{
  alignas(16) f32x4 albedo          = {1, 1, 1, 1};
  alignas(16) f32x4 emission        = {0, 0, 0, 0};
  f32       metallic                = 0;
  f32       roughness               = 0;
  f32       normal                  = 0;
  f32       occlusion               = 0;
  f32       ior                     = 1.5F;
  f32       clearcoat               = 0;
  f32       clearcoat_roughness     = 0;
  f32       clearcoat_normal        = 0;
  SamplerId sampler                 = SamplerId::LinearBlack;
  TextureId albedo_map              = TextureId::White;
  TextureId metallic_map            = TextureId::White;
  TextureId roughness_map           = TextureId::White;
  TextureId normal_map              = TextureId::White;
  TextureId occlusion_map           = TextureId::White;
  TextureId emission_map            = TextureId::White;
  TextureId clearcoat_map           = TextureId::White;
  TextureId clearcoat_roughness_map = TextureId::White;
  TextureId clearcoat_normal_map    = TextureId::White;
};

struct Vertex
{
  f32 x;
  f32 y;
  f32 z;
  f32 u;
  f32 v;
};

typedef u32 Index;

}    // namespace pbr

namespace sdf
{

struct FlatMaterial
{
  quad::FlatMaterial tint;
  SamplerId          sampler_id;
  TextureId          map_id;
};

struct NoiseMaterial
{
  quad::NoiseMaterial noise;
  u32                 sampler_id;
  u32                 map_id;
};

enum class ShadeType : u32
{
  Flood     = 0,
  Softened  = 1,
  Feathered = 2,
  Stroked   = 3
};

enum class ShapeType : u32
{
  RRect    = 0,
  Squircle = 1,
  SDFMap   = 2
};

enum class BlendOp : u32
{
  None                = 0,
  Sub                 = 1,
  Xor                 = 2,
  Round               = 3,
  Onion               = 4,
  Union               = 5,
  Intersection        = 6,
  SmoothUnion         = 7,
  SmoothSub           = 8,
  SmoothIntersection  = 9,
  ExpSmoothUnion      = 10,
  RootSmoothUnion     = 11,
  SigSmoothUnion      = 12,
  QuadSmoothUnion     = 13,
  CubicSmoothUnion    = 14,
  QuartSmoothUnion    = 15,
  CircSmoothUnion     = 16,
  CircGeomSmoothUnion = 17
};

struct Shape
{
  alignas(16) f32x4 radii;
  alignas(8) f32x2 half_bbox_extent;
  alignas(8) f32x2 half_extent;
  f32       feather;
  ShadeType shade_type;
  ShapeType type;
};

}    // namespace sdf

namespace composite_sdf
{

#define NUM_COMPOSITE_SDFS 4

struct Shape
{
  alignas(16) f32x4 radii;
  alignas(8) f32x2 half_extent;
  alignas(8) f32x2 bbox_center;
  sdf::ShapeType shape_type;
  f32            sdf_blend_factor;
  sdf::BlendOp   sdf_blend_op;
};

struct Composite
{
  alignas(8) f32x2 half_bbox_extent;
  sdf::ShadeType shade_type;
  f32            feather;
  Shape          shapes[NUM_COMPOSITE_SDFS];
};

template <typename M>
struct BlendedMaterial
{
  M materials[NUM_COMPOSITE_SDFS];
};

using FlatMaterial = BlendedMaterial<sdf::FlatMaterial>;

}    // namespace composite_sdf
}    // namespace shader
}    // namespace ash
