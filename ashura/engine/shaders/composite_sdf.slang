/// SPDX-License-Identifier: MIT

#pragma once
#include "custom.inl.slang"
#include "items.slang"
#include "materials/sdf.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

static const f32x2 VERTEX_BUFFER[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

#ifndef ITEM_TYPE
#  define ITEM_TYPE SdfCompoundGradientItem
#endif

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
StructuredBuffer<ITEM_TYPE> composites;

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 vertex : SV_VertexID)
{
  f32x2 bbox_rel_pos = VERTEX_BUFFER[vertex];
  var   composite    = composites[instance];
  f32x4 world_pos =
    mul(composite.world_transform, f32x4(bbox_rel_pos, 0.0, 1.0));
  f32x4 screen_pos = mul(world_to_ndc, world_pos);
  f32x2 uv = mul(composite.uv_transform, f32x4(bbox_rel_pos, 0.0, 1.0)).xy;
  return VertexOutput(screen_pos, world_pos, bbox_rel_pos, uv, instance);
}

[[shader("fragment")]] f32x4 frag(VertexOutput in) : COLOR
{
  var   c        = composites[in.instance];
  f32x2 bbox_pos = in.bbox_rel_pos * 2 * c.half_bbox_extent;
  var   frag =
    SdfFragmentInfo(in.world_pos, in.screen_pos, in.uv, in.bbox_rel_pos);
  f32 edge_signed_dist = 0;
  var material         = c.material;
  f32 tex_sdf          = material.sdf(frag, samplers, textures);

  for (u32 i = 0; i < 4; i++)
  {
    var   subitem   = c.subitems[i];
    f32x2 shape_pos = bbox_pos - subitem.bbox_center;
    f32   curr      = 0;

    switch (subitem.shape_type)
    {
      case SdfShapeType::RRect:
      {
        curr = sdf::rrect(shape_pos, subitem.half_extent, subitem.radii);
      }
      break;
      case SdfShapeType::Squircle:
      {
        curr = sdf::squircle(shape_pos, subitem.half_extent, subitem.radii.x,
                             subitem.radii.y);
      }
      break;
      case SdfShapeType::SDFMap:
      {
        curr = tex_sdf;
      }
      break;
      default:
      {
        curr = 0;
      }
      break;
    }

    switch (subitem.sdf_blend_op)
    {
      case SdfBlendOp::Sub:
      {
        edge_signed_dist = sdf::sub(edge_signed_dist, curr);
      }
      break;
      case SdfBlendOp::Xor:
      {
        edge_signed_dist = sdf::xor_op(edge_signed_dist, curr);
      }
      break;
      case SdfBlendOp::Round:
      {
        edge_signed_dist = sdf::round(edge_signed_dist, curr);
      }
      break;
      case SdfBlendOp::Union:
      {
        edge_signed_dist = sdf::union(edge_signed_dist, curr);
      }
      break;
      case SdfBlendOp::Intersection:
      {
        edge_signed_dist = sdf::intersection(edge_signed_dist, curr);
      }
      break;
      case SdfBlendOp::SmoothUnion:
      {
        edge_signed_dist =
          sdf::smooth_union(edge_signed_dist, curr, subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::SmoothSub:
      {
        edge_signed_dist =
          sdf::smooth_sub(edge_signed_dist, curr, subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::SmoothIntersection:
      {
        edge_signed_dist = sdf::smooth_intersection(edge_signed_dist, curr,
                                                    subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::ExpSmoothUnion:
      {
        edge_signed_dist =
          sdf::exp_smoothmin(edge_signed_dist, curr, subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::RootSmoothUnion:
      {
        edge_signed_dist =
          sdf::root_smoothmin(edge_signed_dist, curr, subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::SigSmoothUnion:
      {
        edge_signed_dist = sdf::sigmoid_smoothmin(edge_signed_dist, curr,
                                                  subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::QuadSmoothUnion:
      {
        edge_signed_dist =
          sdf::quad_smoothmin(edge_signed_dist, curr, subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::CubicSmoothUnion:
      {
        edge_signed_dist = sdf::cubic_smoothmin(edge_signed_dist, curr,
                                                subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::QuartSmoothUnion:
      {
        edge_signed_dist = sdf::quart_smoothmin(edge_signed_dist, curr,
                                                subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::CircSmoothUnion:
      {
        edge_signed_dist =
          sdf::circ_smoothmin(edge_signed_dist, curr, subitem.sdf_blend_factor);
      }
      break;
      case SdfBlendOp::CircGeomSmoothUnion:
      {
        edge_signed_dist = sdf::circ_geom_smoothmin(edge_signed_dist, curr,
                                                    subitem.sdf_blend_factor);
      }
      break;
      default:
      case SdfBlendOp::None:
        break;
    }
  }

  switch (c.shade_type)
  {
    case SdfShadeType::Flood:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      return material.flood(frag, edge_signed_dist, edge_mask, samplers,
                            textures);
    }
    break;
    case SdfShadeType::Softened:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      f32 soften_signed_dist =
        edge_mask * (1 - smoothstep(0, c.feather, edge_signed_dist));
      f32 soften_alpha = soften_signed_dist;
      return material.soften(frag, edge_signed_dist, edge_mask,
                             soften_signed_dist, soften_alpha, samplers,
                             textures);
    }
    break;
    case SdfShadeType::Feathered:
    {
      f32 edge_mask           = sdf::antialiased_mask(edge_signed_dist);
      f32 feather_signed_dist = 1 - smoothstep(0, c.feather, edge_signed_dist);
      f32 feather_alpha       = edge_mask * feather_signed_dist;
      return material.feather(frag, edge_signed_dist, edge_mask,
                              feather_signed_dist, feather_alpha, samplers,
                              textures);
    }
    break;
    case SdfShadeType::Stroked:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      f32 ring_signed_dist =
        sdf::ring_signed_distance(edge_signed_dist, c.feather);
      f32 ring_mask = edge_mask * sdf::antialiased_mask(ring_signed_dist);
      return material.stroke(frag, edge_signed_dist, edge_mask,
                             ring_signed_dist, ring_mask, samplers, textures);
    }
    break;
    default:
      return f32x4(0);
  }
}
