/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/custom.inl.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"

struct VertexOutput
{
  [[vk::location(0)]] f32x4                         pos : SV_Position;
  [[vk::location(1)]] f32x2                         uv : UV_COORD;
  [[vk::location(2)]] nointerpolation BezierRegions region : FILL_REGIONS;
};

[[vk::binding(0, 0)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 1)]]
StructuredBuffer<u32> triangle_offsets;

[[vk::binding(0, 2)]]
StructuredBuffer<f32x4x4> transforms;

[[vk::binding(0, 3)]]
StructuredBuffer<f32x2> vertices;

[[vk::binding(0, 4)]]
StructuredBuffer<BezierRegions> regions;

static const f32x2 BARYCENTRIC_COORDS[3] = {
  {0,   0},
  {0.5, 0},
  {1,   1}
};

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 primitive : SV_PrimitiveID,
                                       u32 vertex : SV_VertexID)
{
  var rel_pos         = vertices[vertex];
  var uv              = BARYCENTRIC_COORDS[vertex % 3];
  var world_transform = transforms[instance];
  var world_pos       = mul(world_transform, f32x4(rel_pos, 0, 1));
  var screen_pos      = mul(world_to_ndc, world_pos);
  var triangle_offset = triangle_offsets[instance];
  var region          = regions[triangle_offset + primitive];
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
