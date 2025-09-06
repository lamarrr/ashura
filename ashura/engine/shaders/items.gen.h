/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief manually generated from engine/shaders/*,

// do not change the underlying type. It maps directly to the GPU handle
enum class [[nodiscard]] TextureIndex : u32
{
  Base = 0,

  /// @brief Nominal Texture Ids for the GPU system's texture set
  White              = 0,     // [1, 1, 1, 1]
  Transparent        = 1,     // [0, 0, 0, 0]
  RedTransparent     = 2,     // [1, 0, 0, 0]
  GreenTransparent   = 3,     // [0, 1, 0, 0]
  BlueTransparent    = 4,     // [0, 0, 1, 0]
  YellowTransparent  = 5,     // [1, 1, 0, 0]
  MagentaTransparent = 6,     // [1, 0, 1, 0]
  CyanTransparent    = 7,     // [0, 1, 1, 0]
  WhiteTransparent   = 8,     // [1, 1, 1, 0]
  Black              = 9,     // [0, 0, 0, 1]
  Red                = 10,    // [1, 0, 0, 1]
  Green              = 11,    // [0, 1, 0, 1]
  Blue               = 12,    // [0, 0, 1, 1]
  Yellow             = 13,    // [1, 1, 0, 1]
  Magenta            = 14,    // [1, 0, 1, 1]
  Cyan               = 15     // [0, 1, 1, 1]
};

// do not change the underlying type. It maps directly to the GPU handle
enum class [[nodiscard]] SamplerIndex : u32
{
  LinearRepeatTransparentFloat           = 0,
  LinearRepeatTransparentInt             = 1,
  LinearRepeatBlackFloat                 = 2,
  LinearRepeatBlackInt                   = 3,
  LinearRepeatWhiteFloat                 = 4,
  LinearRepeatWhiteInt                   = 5,
  LinearMirroredRepeatTransparentFloat   = 6,
  LinearMirroredRepeatTransparentInt     = 7,
  LinearMirroredRepeatBlackFloat         = 8,
  LinearMirroredRepeatBlackInt           = 9,
  LinearMirroredRepeatWhiteFloat         = 10,
  LinearMirroredRepeatWhiteInt           = 11,
  LinearEdgeClampTransparentFloat        = 12,
  LinearEdgeClampTransparentInt          = 13,
  LinearEdgeClampBlackFloat              = 14,
  LinearEdgeClampBlackInt                = 15,
  LinearEdgeClampWhiteFloat              = 16,
  LinearEdgeClampWhiteInt                = 17,
  LinearBorderClampTransparentFloat      = 18,
  LinearBorderClampTransparentInt        = 19,
  LinearBorderClampBlackFloat            = 20,
  LinearBorderClampBlackInt              = 21,
  LinearBorderClampWhiteFloat            = 22,
  LinearBorderClampWhiteInt              = 23,
  LinearMirrorEdgeClampTransparentFloat  = 24,
  LinearMirrorEdgeClampTransparentInt    = 25,
  LinearMirrorEdgeClampBlackFloat        = 26,
  LinearMirrorEdgeClampBlackInt          = 27,
  LinearMirrorEdgeClampWhiteFloat        = 28,
  LinearMirrorEdgeClampWhiteInt          = 29,
  NearestRepeatTransparentFloat          = 30,
  NearestRepeatTransparentInt            = 31,
  NearestRepeatBlackFloat                = 32,
  NearestRepeatBlackInt                  = 33,
  NearestRepeatWhiteFloat                = 34,
  NearestRepeatWhiteInt                  = 35,
  NearestMirroredRepeatTransparentFloat  = 36,
  NearestMirroredRepeatTransparentInt    = 37,
  NearestMirroredRepeatBlackFloat        = 38,
  NearestMirroredRepeatBlackInt          = 39,
  NearestMirroredRepeatWhiteFloat        = 40,
  NearestMirroredRepeatWhiteInt          = 41,
  NearestEdgeClampTransparentFloat       = 42,
  NearestEdgeClampTransparentInt         = 43,
  NearestEdgeClampBlackFloat             = 44,
  NearestEdgeClampBlackInt               = 45,
  NearestEdgeClampWhiteFloat             = 46,
  NearestEdgeClampWhiteInt               = 47,
  NearestBorderClampTransparentFloat     = 48,
  NearestBorderClampTransparentInt       = 49,
  NearestBorderClampBlackFloat           = 50,
  NearestBorderClampBlackInt             = 51,
  NearestBorderClampWhiteFloat           = 52,
  NearestBorderClampWhiteInt             = 53,
  NearestMirrorEdgeClampTransparentFloat = 54,
  NearestMirrorEdgeClampTransparentInt   = 55,
  NearestMirrorEdgeClampBlackFloat       = 56,
  NearestMirrorEdgeClampBlackInt         = 57,
  NearestMirrorEdgeClampWhiteFloat       = 58,
  NearestMirrorEdgeClampWhiteInt         = 59
};

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

enum class SdfShapeType : u32
{
  RRect    = 0,
  Squircle = 1,
  SDFMap   = 2
};

enum class SdfShadeType : u32
{
  Flood     = 0,
  Softened  = 1,
  Feathered = 2,
  Stroked   = 3
};

enum class SdfBlendOp : u32
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

enum class SdfMixOp : u32
{
  None           = 0,
  QuadSmoothMin  = 1,
  CubicSmoothMin = 2
};

/// @see https://github.com/KhronosGroup/glTF/tree/acfcbe65e40c53d6d3aa55a7299982bf2c01c75d/extensions/2.0/Khronos
/// @see
/// https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/63b7c128266cfd86bbd3f25caf8b3db3fe854015/source/Renderer/shaders/textures.glsl#L1
struct PbrCoreMaterial
{
  alignas(16) f32x4 albedo;              // {1, 1, 1, 1}
  alignas(16) f32x4 emission;            // {0, 0, 0, 0}
  alignas(4) f32 metallic;               // 0
  alignas(4) f32 roughness;              // 0
  alignas(4) f32 normal;                 // 0
  alignas(4) f32 occlusion;              // 0
  alignas(4) f32 ior;                    // 1.5F
  alignas(4) f32 clearcoat;              // 0
  alignas(4) f32 clearcoat_roughness;    // 0
  alignas(4) f32 clearcoat_normal;       // 0
  alignas(4) SamplerIndex sampler;    // SamplerIndex::LinearEdgeClampBlackFloat
  alignas(4) TextureIndex albedo_map;                 // TextureIndex::White
  alignas(4) TextureIndex metallic_map;               // TextureIndex::White
  alignas(4) TextureIndex roughness_map;              // TextureIndex::White
  alignas(4) TextureIndex normal_map;                 // TextureIndex::White
  alignas(4) TextureIndex occlusion_map;              // TextureIndex::White
  alignas(4) TextureIndex emission_map;               // TextureIndex::White
  alignas(4) TextureIndex clearcoat_map;              // TextureIndex::White
  alignas(4) TextureIndex clearcoat_roughness_map;    // TextureIndex::White
  alignas(4) TextureIndex clearcoat_normal_map;       // TextureIndex::White
};

struct QuadGradientMaterial
{
  alignas(16) f32x4 top;
  alignas(16) f32x4 bottom;
  alignas(8) f32x2 gradient_rotor;
  alignas(4) f32 gradient_center;
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex texture;
};

struct SdfGradientMaterial
{
  alignas(16) f32x4 top;
  alignas(16) f32x4 bottom;
  alignas(8) f32x2 gradient_rotor;
  alignas(4) f32 gradient_center;
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex texture;
  alignas(4) SamplerIndex sdf_sampler;
  alignas(4) TextureIndex sdf_map;
};

struct SdfNoiseMaterial
{
  alignas(16) f32x4 intensity;
  alignas(4) SamplerIndex sdf_sampler;
  alignas(4) TextureIndex sdf_map;
};

struct SdfMeshGradientMaterial
{
  alignas(16) f32x4 colors[4];
  alignas(8) f32x2 min;
  alignas(8) f32x2 max;
  alignas(4) f32 aspect_ratio;
  alignas(4) f32 frequency;
  alignas(4) f32 amplitude;
  alignas(4) f32 time;
  alignas(4) SamplerIndex sdf_sampler;
  alignas(4) TextureIndex sdf_map;
};

struct TriangleSetTextureMaterial
{
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex texture;
};

struct BlurItem
{
  alignas(8) f32x2 uv0;
  alignas(8) f32x2 uv1;
  alignas(8) f32x2 radius;
  alignas(4) SamplerIndex sampler;
  alignas(4) TextureIndex tex;
};

struct SdfSubItem
{
  alignas(16) f32x4 radii;
  alignas(8) f32x2 half_extent;
  alignas(8) f32x2 bbox_center;
  alignas(4) SdfShapeType shape_type;
  alignas(4) f32 sdf_blend_factor;
  alignas(4) SdfBlendOp sdf_blend_op;
};

template <typename MaterialType>
struct SdfCompoundItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 uv_transform;
  alignas(8) f32x2 half_bbox_extent;
  alignas(4) SdfShadeType shade_type;
  alignas(4) f32 feather;
  alignas(4) u32 first;
  alignas(4) u32 count;
  MaterialType material;
  SdfSubItem   subitems[4];
};

typedef SdfCompoundItem<SdfGradientMaterial> SdfCompoundGradientItem;

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

typedef QuadItem<QuadGradientMaterial> QuadGradientItem;

template <typename MaterialType>
struct SdfItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 uv_transform;
  alignas(16) f32x4 radii;
  alignas(8) f32x2 half_bbox_extent;
  alignas(8) f32x2 half_extent;
  alignas(4) f32 feather;
  alignas(4) SdfShadeType shade_type;
  alignas(4) SdfShapeType type;
  MaterialType material;
};

typedef SdfItem<SdfGradientMaterial>     SdfGradientItem;
typedef SdfItem<SdfNoiseMaterial>        SdfNoiseItem;
typedef SdfItem<SdfMeshGradientMaterial> SdfMeshGradientItem;

template <typename MaterialType>
struct TriangleSetItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(16) f32x4x4 uv_transform;
  MaterialType material;
};

typedef TriangleSetItem<TriangleSetTextureMaterial> TriangleSetGradientItem;

struct TriangleVertex
{
  alignas(4) f32 x;
  alignas(4) f32 y;
  alignas(4) f32 r;
  alignas(4) f32 g;
  alignas(4) f32 b;
  alignas(4) f32 a;
};

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

typedef PbrItem<PbrCoreMaterial> CorePbrItem;

struct PathVertex
{
  alignas(8) f32x2 position;
  alignas(4) f32 alpha_mask;
  alignas(4) u32 paint_id;
};

struct PathCoverageItem
{
  alignas(16) f32x4x4 world_transform;
};

struct PathPaintItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(4) u32 paint_id;
  alignas(4) f32 feather_min;
  alignas(4) f32 feather_max;
  QuadGradientMaterial material;
};

struct BezierStencilItem
{
  alignas(16) f32x4x4 world_transform;
  alignas(4) u32 first_bezier_vertex;
};

}    // namespace shader
}    // namespace ash
