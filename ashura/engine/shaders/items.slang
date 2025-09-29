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

struct SdfSubItem
{
  f32x4        radii;
  f32x2        half_extent;
  f32x2        bbox_center;
  SdfShapeType shape_type;
  f32          sdf_blend_factor;
  SdfBlendOp   sdf_blend_op;
};

struct SdfCompoundItem<MaterialType : SdfMaterial>
{
  f32x4x4      world_transform;
  f32x4x4      uv_transform;
  f32x2        half_bbox_extent;
  SdfShadeType shade_type;
  f32          feather;
  u32          first;
  u32          count;
  MaterialType material;
  SdfSubItem   subitems[4];
};

typedef SdfCompoundItem<SdfGradientMaterial> SdfCompoundGradientItem;

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

struct QuadItem<MaterialType : QuadMaterial>
{
  f32x4x4      world_transform;
  f32x4x4      uv_transform;
  f32x4x4      corners;
  MaterialType material;
};

typedef QuadItem<QuadGradientMaterial> QuadGradientItem;

struct SdfItem<MaterialType : SdfMaterial>
{
  f32x4x4      world_transform;
  f32x4x4      uv_transform;
  f32x4        radii;
  f32x2        half_bbox_extent;
  f32x2        half_extent;
  f32          feather;
  SdfShadeType shade_type;
  SdfShapeType type;
  MaterialType material;
};

typedef SdfItem<SdfGradientMaterial>     SdfGradientItem;
typedef SdfItem<SdfNoiseMaterial>        SdfNoiseItem;
typedef SdfItem<SdfMeshGradientMaterial> SdfMeshGradientItem;

struct TriangleSetItem<MaterialType : TriangleSetMaterial>
{
  f32x4x4      world_transform;
  f32x4x4      uv_transform;
  MaterialType material;
};

typedef TriangleSetItem<TriangleSetTextureMaterial> TriangleSetGradientItem;

struct TriangleVertex
{
  f32 x;
  f32 y;
  f32 r;
  f32 g;
  f32 b;
  f32 a;

  f32x2 pos()
  {
    return f32x2(x, y);
  }

  f32x4 color()
  {
    return f32x4(r, g, b, a);
  }
};

struct PbrItem<MaterialType : PbrMaterial>
{
  f32x4x4      world_transform;
  f32x4x4      world_to_ndc;
  f32x4x4      uv_transform;
  f32x4        eye_position;
  u32          first_light;
  u32          num_lights;
  MaterialType material;
};

typedef PbrItem<PbrCoreMaterial> CorePbrItem;

struct PathVertex
{
  f32x2 position;
  f32   alpha_mask;
  u32   paint_id;
};

struct PathCoverageItem
{
  f32x4x4 world_transform;
};

struct PathPaintItem
{
  f32x4x4              world_transform;
  u32                  paint_id;
  f32                  feather_min;
  f32                  feather_max;
  QuadGradientMaterial material;
};

struct BezierStencilItem
{
  f32x4x4 world_transform;
  u32     first_bezier_vertex;
};
