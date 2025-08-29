/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/custom.inl.slang"
#include "materials/sdf.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

typealias BlendOp   = sdf::BlendOp;
typealias ShapeType = sdf::ShapeType;
typealias ShadeType = sdf::ShadeType;

static const f32x2 VERTEX_BUFFER[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

struct Shape
{
  f32x4        radii;
  f32x2        half_extent;
  f32x2        bbox_center;
  ShapeType    shape_type;
  f32          sdf_blend_factor;
  sdf::BlendOp sdf_blend_op;
};

struct Composite
{
  f32x2     half_bbox_extent;
  ShadeType shade_type;
  f32       feather;
  Shape     shapes[NUM_COMPOSITE_SDFS];
};

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos : SV_Position;
  [[vk::location(1)]] f32x2               rel_pos    : REL_POS;
  [[vk::location(2)]] nointerpolation u32 instance   : INSTANCE;
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 3)]]
StructuredBuffer<Composite> composites;

[[vk::binding(0, 4)]]
StructuredBuffer<f32x4x4> transforms;

#ifndef MATERIAL_TYPE

typedef sdf::BlendedMaterial<sdf::FlatMaterial> MaterialType;

#else

typedef MATERIAL_TYPE MaterialType;

#endif

[[vk::binding(0, 5)]]
StructuredBuffer<MaterialType> materials;

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 vertex : SV_VertexID)
{
  f32x2   rel_pos   = VERTEX_BUFFER[vertex];
  f32x4x4 transform = transforms[instance];
  f32x4   screen_pos =
    mul(world_to_ndc, mul(transform, f32x4(rel_pos, 0.0, 1.0)));
  return VertexOutput(screen_pos, rel_pos, instance);
}

[[shader("fragment")]] f32x4 frag(VertexOutput in) : COLOR
{
  var   c        = composites[in.instance];
  f32x2 bbox_pos = in.rel_pos * 2 * c.half_bbox_extent;
  var   material = materials[in.instance];
  var   frag =
    sdf::FragmentInfo(in.screen_pos, in.screen_pos, f32x4(in.rel_pos, 0, 1),
                      c.half_bbox_extent, c.half_bbox_extent, bbox_pos);

  f32 edge_signed_dist = 0;

  for (u32 i = 0; i < NUM_COMPOSITE_SDFS; i++)
  {
    Shape shape     = c.shapes[i];
    f32x2 shape_pos = bbox_pos - shape.bbox_center;
    var   frag =
      sdf::FragmentInfo(in.screen_pos, in.screen_pos, f32x4(in.rel_pos, 0, 1),
                        c.half_bbox_extent, shape.half_extent, shape_pos);
    f32 curr = 0;

    switch (shape.shape_type)
    {
      case ShapeType::RRect:
      {
        curr = sdf::rrect(shape_pos, shape.half_extent, shape.radii);
      }
      break;
      case ShapeType::Squircle:
      {
        curr = sdf::squircle(shape_pos, shape.half_extent, shape.radii.x,
                             shape.radii.y);
      }
      break;
      case ShapeType::SDFMap:
      {
        curr = material.sdf(frag, i, samplers, textures);
      }
      break;
      default:
      {
        curr = 0;
      }
      break;
    }

    switch (shape.sdf_blend_op)
    {
      case BlendOp::Sub:
      {
        edge_signed_dist = sdf::sub(edge_signed_dist, curr);
      }
      break;
      case BlendOp::Xor:
      {
        edge_signed_dist = sdf::xor_op(edge_signed_dist, curr);
      }
      break;
      case BlendOp::Round:
      {
        edge_signed_dist = sdf::round(edge_signed_dist, curr);
      }
      break;
      case BlendOp::Union:
      {
        edge_signed_dist = sdf::union(edge_signed_dist, curr);
      }
      break;
      case BlendOp::Intersection:
      {
        edge_signed_dist = sdf::intersection(edge_signed_dist, curr);
      }
      break;
      case BlendOp::SmoothUnion:
      {
        edge_signed_dist =
          sdf::smooth_union(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::SmoothSub:
      {
        edge_signed_dist =
          sdf::smooth_sub(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::SmoothIntersection:
      {
        edge_signed_dist = sdf::smooth_intersection(edge_signed_dist, curr,
                                                    shape.sdf_blend_factor);
      }
      break;
      case BlendOp::ExpSmoothUnion:
      {
        edge_signed_dist =
          sdf::exp_smoothmin(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::RootSmoothUnion:
      {
        edge_signed_dist =
          sdf::root_smoothmin(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::SigSmoothUnion:
      {
        edge_signed_dist = sdf::sigmoid_smoothmin(edge_signed_dist, curr,
                                                  shape.sdf_blend_factor);
      }
      break;
      case BlendOp::QuadSmoothUnion:
      {
        edge_signed_dist =
          sdf::quad_smoothmin(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::CubicSmoothUnion:
      {
        edge_signed_dist =
          sdf::cubic_smoothmin(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::QuartSmoothUnion:
      {
        edge_signed_dist =
          sdf::quart_smoothmin(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::CircSmoothUnion:
      {
        edge_signed_dist =
          sdf::circ_smoothmin(edge_signed_dist, curr, shape.sdf_blend_factor);
      }
      break;
      case BlendOp::CircGeomSmoothUnion:
      {
        edge_signed_dist = sdf::circ_geom_smoothmin(edge_signed_dist, curr,
                                                    shape.sdf_blend_factor);
      }
      break;
      default:
      case BlendOp::None:
        break;
    }
  }

  switch (c.shade_type)
  {
    case ShadeType::Flood:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      return material.flood(frag, edge_signed_dist, edge_mask, samplers,
                            textures);
    }
    break;
    case ShadeType::Softened:
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
    case ShadeType::Feathered:
    {
      f32 edge_mask           = sdf::antialiased_mask(edge_signed_dist);
      f32 feather_signed_dist = 1 - smoothstep(0, c.feather, edge_signed_dist);
      f32 feather_alpha       = edge_mask * feather_signed_dist;
      return material.feather(frag, edge_signed_dist, edge_mask,
                              feather_signed_dist, feather_alpha, samplers,
                              textures);
    }
    break;
    case ShadeType::Stroked:
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
