/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief manually generated from engine/shaders/*,

// we should have a script to automate this
namespace shader
{

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
  Edge    = 4,
  All     = 7
};

enum class TriangleShadeRate : u32
{
  Instance  = 0,
  Primitive = 1,
  Vertex    = 2
};

namespace sdf
{

enum class ShapeType : u32
{
  RRect    = 0,
  Squircle = 1,
  SDFMap   = 2
};

enum class ShadeType : u32
{
  Flood     = 0,
  Softened  = 1,
  Feathered = 2,
  Stroked   = 3
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

enum class MixOp : u32
{
  None           = 0,
  QuadSmoothMin  = 1,
  CubicSmoothMin = 2
};

}    // namespace sdf

namespace pbr
{

/// @see https://github.com/KhronosGroup/glTF/tree/acfcbe65e40c53d6d3aa55a7299982bf2c01c75d/extensions/2.0/Khronos
/// @see
/// https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/63b7c128266cfd86bbd3f25caf8b3db3fe854015/source/Renderer/shaders/textures.glsl#L1
struct CoreMaterial
{
  alignas(16) f32x4 albedo           = {1, 1, 1, 1};
  alignas(16) f32x4 emission         = {0, 0, 0, 0};
  alignas(4) f32 metallic            = 0;
  alignas(4) f32 roughness           = 0;
  alignas(4) f32 normal              = 0;
  alignas(4) f32 occlusion           = 0;
  alignas(4) f32 ior                 = 1.5F;
  alignas(4) f32 clearcoat           = 0;
  alignas(4) f32 clearcoat_roughness = 0;
  alignas(4) f32 clearcoat_normal    = 0;
  alignas(4) SamplerIndex sampler    = SamplerIndex::LinearEdgeClampBlackFloat;
  alignas(4) TextureIndex albedo_map = TextureIndex::White;
  alignas(4) TextureIndex metallic_map            = TextureIndex::White;
  alignas(4) TextureIndex roughness_map           = TextureIndex::White;
  alignas(4) TextureIndex normal_map              = TextureIndex::White;
  alignas(4) TextureIndex occlusion_map           = TextureIndex::White;
  alignas(4) TextureIndex emission_map            = TextureIndex::White;
  alignas(4) TextureIndex clearcoat_map           = TextureIndex::White;
  alignas(4) TextureIndex clearcoat_roughness_map = TextureIndex::White;
  alignas(4) TextureIndex clearcoat_normal_map    = TextureIndex::White;
};

}    // namespace pbr

namespace quad
{

struct FlatMaterial
{
  alignas(16) f32x4 top;
  alignas(16) f32x4 bottom;
  alignas(8) f32x2 gradient_rotor;
  alignas(4) f32 gradient_center;
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex texture;
};

struct NoiseMaterial
{
  alignas(16) f32x4 intensity;
};

}    // namespace quad

namespace sdf
{

struct FlatMaterial
{
  alignas(16) f32x4 top;
  alignas(16) f32x4 bottom;
  alignas(8) f32x2 gradient_rotor;    // (cos(theta), sin(theta))
  alignas(4) f32 gradient_center;
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex texture;
  alignas(4) SamplerIndex sdf_sampler;
  alignas(4) TextureIndex sdf_map;
};

struct NoiseMaterial
{
  alignas(16) f32x4 intensity;
  alignas(4) SamplerIndex sdf_sampler;
  alignas(4) TextureIndex sdf_map;
};

struct MeshGradientMaterial
{
  alignas(16) f32x4 colors[4];
  alignas(8) f32x2 min;
  alignas(8) f32x2 max;
  alignas(4) f32 aspect_ratio;    // x/y
  alignas(4) f32 frequency;       // = 5.
  alignas(4) f32 amplitude;       // = 30.
  alignas(4) f32 time;
  alignas(4) SamplerIndex sdf_sampler;
  alignas(4) TextureIndex sdf_map;
};

}    // namespace sdf

namespace triangle_fill
{

struct TextureMaterial
{
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex texture;
};

}    // namespace triangle_fill

struct BlurItem
{
  alignas(8) f32x2 uv0;
  alignas(8) f32x2 uv1;
  alignas(8) f32x2 radius;
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex tex;
};

template <typename MaterialType>
struct SdfCompoundItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 uv_transform;
  alignas(8) f32x2 half_bbox_extent;
  alignas(4) sdf::ShadeType shade_type;
  alignas(4) f32 feather;
  alignas(4) u32 first;
  alignas(4) u32 count;
  MaterialType material;
};

typedef SdfCompoundItem<sdf::FlatMaterial> SdfCompoundFlatItem;

struct SdfSubItem
{
  alignas(16) f32x4 radii;
  alignas(8) f32x2 half_extent;
  alignas(8) f32x2 bbox_center;
  alignas(4) sdf::ShapeType shape_type;
  alignas(4) f32 sdf_blend_factor;
  alignas(4) sdf::BlendOp sdf_blend_op;
};

struct PbrVertex
{
  alignas(4) f32 x;
  alignas(4) f32 y;
  alignas(4) f32 z;
  alignas(4) f32 u;
  alignas(4) f32 v;
};

template <typename MaterialType>
struct QuadItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 uv_transform;
  alignas(16) f32x4x4 corners;
  MaterialType material;
};

typedef QuadItem<quad::FlatMaterial> FlatQuadItem;

template <typename MaterialType>
struct SdfItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 uv_transform;
  alignas(16) f32x4 radii;
  alignas(8) f32x2 half_bbox_extent;
  alignas(8) f32x2 half_extent;
  alignas(4) f32 feather;
  alignas(4) sdf::ShadeType shade_type;
  alignas(4) sdf::ShapeType type;
  MaterialType material;
};

typedef SdfItem<sdf::FlatMaterial>         FlatSdfItem;
typedef SdfItem<sdf::NoiseMaterial>        NoiseSdfItem;
typedef SdfItem<sdf::MeshGradientMaterial> MeshGradientSdfItem;

template <typename MaterialType>
struct TriangleSetItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 uv_transform;
  alignas(4) TriangleShadeRate rate;
  MaterialType material;
};

typedef TriangleSetItem<triangle_fill::TextureMaterial> FlatTriangleSetItem;

template <typename MaterialType>
struct PbrItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 world_to_ndc;
  alignas(16) f32x4x4 uv_transform;
  alignas(16) f32x4 eye_position;
  alignas(4) u32 first_light;
  alignas(4) u32 num_lights;
  MaterialType material;
};

typedef PbrItem<pbr::CoreMaterial> CorePbrItem;

}    // namespace shader
}    // namespace ash
