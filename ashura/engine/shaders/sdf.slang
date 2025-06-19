/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/custom.inl.slang"
#include "materials/sdf.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

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
  f32x4     radii;
  f32x2     half_bbox_extent;
  f32x2     half_extent;
  f32       feather;
  ShadeType shade_type;
  ShapeType type;
};

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos : SV_Position;
  [[vk::location(1)]] f32x2               rel_pos : REL_POS;
  [[vk::location(2)]] nointerpolation u32 instance : INSTANCE;
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 3)]]
StructuredBuffer<Shape> shapes;

[[vk::binding(0, 4)]]
StructuredBuffer<f32x4x4> transforms;

#ifndef MATERIAL_TYPE

typedef sdf::FlatMaterial MaterialType;

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

// [ ] border effects
[[shader("fragment")]] f32x4 frag(VertexOutput in) : COLOR
{
  var   shape    = shapes[in.instance];
  f32x2 bbox_pos = in.rel_pos * 2 * shape.half_bbox_extent;
  var   material = materials[in.instance];
  var   frag =
    sdf::FragmentInfo(in.screen_pos, in.screen_pos, f32x4(in.rel_pos, 0, 1),
                      shape.half_bbox_extent, shape.half_extent, bbox_pos);

  f32 edge_signed_dist = 0;

  switch (shape.type)
  {
    case ShapeType::RRect:
    {
      edge_signed_dist = sdf::rrect(bbox_pos, shape.half_extent, shape.radii);
    }
    break;
    case ShapeType::Squircle:
    {
      edge_signed_dist = sdf::squircle(bbox_pos, shape.half_extent,
                                       shape.radii.x, shape.radii.y);
    }
    break;
    case ShapeType::SDFMap:
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

  switch (shape.shade_type)
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
        edge_mask * (1 - smoothstep(0, shape.feather, edge_signed_dist));
      f32 soften_alpha = soften_signed_dist;
      return material.soften(frag, edge_signed_dist, edge_mask,
                             soften_signed_dist, soften_alpha, samplers,
                             textures);
    }
    break;
    case ShadeType::Feathered:
    {
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      f32 feather_signed_dist =
        1 - smoothstep(0, shape.feather, edge_signed_dist);
      f32 feather_alpha = edge_mask * feather_signed_dist;
      return material.feather(frag, edge_signed_dist, edge_mask,
                              feather_signed_dist, feather_alpha, samplers,
                              textures);
    }
    break;
    case ShadeType::Stroked:
    {
      // bounding box must be at least (size + feather)
      f32 edge_mask = sdf::antialiased_mask(edge_signed_dist);
      f32 ring_signed_dist =
        sdf::ring_signed_distance(edge_signed_dist, shape.feather);
      f32 ring_mask = edge_mask * sdf::antialiased_mask(ring_signed_dist);
      return material.stroke(frag, edge_signed_dist, edge_mask,
                             ring_signed_dist, ring_mask, samplers, textures);
    }
    break;
    default:
      return f32x4(0);
  }
}
