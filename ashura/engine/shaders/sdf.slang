/// SPDX-License-Identifier: MIT

#pragma once
#include "custom.inl.slang"
#include "items.slang"
#include "materials/sdf.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

#ifndef ITEM_TYPE
#  define ITEM_TYPE SdfGradientItem
#endif

static const f32x2 VERTEX_BUFFER[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos   : SV_Position;
  [[vk::location(1)]] f32x4               world_pos    : WORLD;
  [[vk::location(2)]] f32x2               bbox_rel_pos : REL_POS;
  [[vk::location(3)]] f32x2               uv           : UV;
  [[vk::location(4)]] nointerpolation u32 instance     : INSTANCE;
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 3)]]
StructuredBuffer<ITEM_TYPE> items;

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 vertex : SV_VertexID)
{
  f32x2 bbox_rel_pos = VERTEX_BUFFER[vertex];
  var   s            = items[instance];
  f32x4 world_pos    = mul(s.world_transform, f32x4(bbox_rel_pos, 0.0, 1.0));
  f32x4 screen_pos   = mul(world_to_ndc, world_pos);
  f32x2 uv           = mul(s.uv_transform, f32x4(bbox_rel_pos, 0.0, 1.0)).xy;
  return VertexOutput(screen_pos, world_pos, bbox_rel_pos, uv, instance);
}

[[shader("fragment")]] f32x4 frag(VertexOutput in) : COLOR
{
  var   item     = items[in.instance];
  var   material = item.material;
  f32x2 bbox_pos = in.bbox_rel_pos * 2 * item.half_bbox_extent;
  var   frag =
    SdfFragmentInfo(in.world_pos, in.screen_pos, in.uv, in.bbox_rel_pos);

  f32 edge_signed_dist = 0;

  switch (item.type)
  {
    case SdfShapeType::RRect:
    {
      edge_signed_dist = sdf::rrect(bbox_pos, item.half_extent, item.radii);
    }
    break;
    case SdfShapeType::Squircle:
    {
      edge_signed_dist =
        sdf::squircle(bbox_pos, item.half_extent, item.radii.x, item.radii.y);
    }
    break;
    case SdfShapeType::SDFMap:
    {
      edge_signed_dist = material.sdf(frag, samplers, textures);
    }
    break;
    default:
    {
      edge_signed_dist = 0;
    }
    break;
  }

  switch (item.shade_type)
  {
    case SdfShadeType::Flood:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      return material.flood(frag, edge_signed_dist, edge_mask, samplers,
                            textures);
    }
    case SdfShadeType::Softened:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      f32 soften_signed_dist =
        edge_mask * (1 - smoothstep(0, item.feather, edge_signed_dist));
      f32 soften_alpha = soften_signed_dist;
      return material.soften(frag, edge_signed_dist, edge_mask,
                             soften_signed_dist, soften_alpha, samplers,
                             textures);
    }
    case SdfShadeType::Feathered:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      f32 feather_signed_dist =
        1 - smoothstep(0, item.feather, edge_signed_dist);
      f32 feather_alpha = edge_mask * feather_signed_dist;
      return material.feather(frag, edge_signed_dist, edge_mask,
                              feather_signed_dist, feather_alpha, samplers,
                              textures);
    }
    case SdfShadeType::Stroked:
    {
      // bounding box must be at least (size + feather)
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      f32 ring_signed_dist =
        sdf::ring_signed_distance(edge_signed_dist, item.feather);
      f32 ring_mask = edge_mask * sdf::antialiased_mask(ring_signed_dist);
      return material.stroke(frag, edge_signed_dist, edge_mask,
                             ring_signed_dist, ring_mask, samplers, textures);
    }
    default:
      return f32x4(0);
  }
}
