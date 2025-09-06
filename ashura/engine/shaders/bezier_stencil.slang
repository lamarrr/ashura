/// SPDX-License-Identifier: MIT

#pragma once
#include "items.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"

struct VertexOutput
{
  f32x4                                             pos    : SV_Position;
  [[vk::location(0)]] f32x2                         uv     : UV_COORD;
  [[vk::location(1)]] nointerpolation BezierRegions region : FILL_REGIONS;
};

[[vk::binding(0, 0)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 1)]]
StructuredBuffer<BezierStencilItem> items;

[[vk::binding(0, 2)]]
StructuredBuffer<f32x2> vertices;

[[vk::binding(0, 3)]]
StructuredBuffer<u32> indices;

static const f32x2 BARYCENTRIC_COORDS[3] = {
  {0.0, 0.0},
  {0.5, 0.0},
  {1.0, 1.0}
};

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 vertex : SV_VertexID)
{
  u32   idx        = indices[vertex];
  f32x2 rel_pos    = vertices[idx];
  f32x2 uv         = BARYCENTRIC_COORDS[vertex % 3];
  var   item       = items[instance];
  f32x4 world_pos  = mul(item.world_transform, f32x4(rel_pos, 0.0, 1.0));
  f32x4 screen_pos = mul(world_to_ndc, world_pos);
  var   region = item.first_bezier_vertex >= vertex ? BezierRegions::Inside: BezierRegions::All  
                                                          ;
  return VertexOutput(screen_pos, uv, region);
}

[[shader("fragment")]] void frag(VertexOutput in)
{
  BezierRegions fill_type = in.region;
  BezierRegions regions   = bary::bezier_region(in.uv);
  var           fill      = regions & fill_type;

  if (fill == BezierRegions::None)
  {
    discard;
  }
}
