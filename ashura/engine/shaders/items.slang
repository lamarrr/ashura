/// SPDX-License-Identifier: MIT

#pragma once

#include "materials/pbr.slang"
#include "materials/quad.slang"
#include "materials/sdf.slang"
#include "materials/triangle_fill.slang"
#include "modules/types.slang"

/// @param radius relative to texture dimensions
struct BlurItem
{
  f32x2        uv0;
  f32x2        uv1;
  f32x2        radius;
  SamplerIndex sampler;
  TextureIndex tex;
};

struct SdfCompoundItem<MaterialType : sdf::Material>
{
  f32x4x4        world_transform;
  f32x4x4        uv_transform;
  f32x2          half_bbox_extent;
  sdf::ShadeType shade_type;
  f32            feather;
  u32            first;
  u32            count;
  MaterialType   material;
};

typedef SdfCompoundItem<sdf::FlatMaterial> SdfCompoundFlatItem;

struct SdfSubItem
{
  f32x4          radii;
  f32x2          half_extent;
  f32x2          bbox_center;
  sdf::ShapeType shape_type;
  f32            sdf_blend_factor;
  sdf::BlendOp   sdf_blend_op;
};

struct PbrVertex
{
  f32 x;
  f32 y;
  f32 z;
  f32 u;
  f32 v;

  f32x4 pos()
  {
    return f32x4(x, y, z, 1);
  }

  f32x2 uv()
  {
    return f32x2(u, v);
  }
};

struct QuadItem<MaterialType : quad::Material>
{
  f32x4x4      world_transform;
  f32x4x4      uv_transform;
  f32x4x4      corners;
  MaterialType material;
};

typedef QuadItem<quad::FlatMaterial> FlatQuadItem;

struct SdfItem<MaterialType : sdf::Material>
{
  f32x4x4        world_transform;
  f32x4x4        uv_transform;
  f32x4          radii;
  f32x2          half_bbox_extent;
  f32x2          half_extent;
  f32            feather;
  sdf::ShadeType shade_type;
  sdf::ShapeType type;
  MaterialType   material;
};

typedef SdfItem<sdf::FlatMaterial>         FlatSdfItem;
typedef SdfItem<sdf::NoiseMaterial>        NoiseSdfItem;
typedef SdfItem<sdf::MeshGradientMaterial> MeshGradientSdfItem;

struct TriangleSetItem<MaterialType : triangle_fill::Material>
{
  f32x4x4           world_transform;
  f32x4x4           uv_transform;
  TriangleShadeRate rate;
  MaterialType      material;
};

typedef TriangleSetItem<triangle_fill::TextureMaterial> FlatTriangleSetItem;

struct PbrItem<MaterialType : pbr::Material>
{
  f32x4x4      world_transform;
  f32x4x4      world_to_ndc;
  f32x4x4      uv_transform;
  f32x4        eye_position;
  u32          first_light;
  u32          num_lights;
  MaterialType material;
};

typedef PbrItem<pbr::CoreMaterial> CorePbrItem;
