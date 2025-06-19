/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/custom.inl.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"

struct VertexOutput
{
  [[vk::location(0)]] f32x4               pos : SV_Position;
  [[vk::location(1)]] f32x2               uv : UV_COORD;
  [[vk::location(2)]] nointerpolation u32 instance : INSTANCE;
};

[[vk::binding(0, 0)]]
StructuredBuffer<f32x2> vertices;

[[vk::binding(0, 1)]]
ConstantBuffer<f32x4x4> transform;

[[vk::binding(0, 2)]]
StructuredBuffer<BezierRegions> fills;

static const u32 NUM_BEZIER_VERTICES = 3;

static const f32x2 BARYCENTRIC_COORDS[3] = {
  {0,   0},
  {0.5, 0},
  {1,   1}
};

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 vertex : SV_VertexID)
{
  var idx     = instance * NUM_BEZIER_VERTICES + vertex;
  var vtx_pos = vertices[idx];
  var uv      = BARYCENTRIC_COORDS[vertex];
  var pos     = mul(transform, f32x4(vtx_pos, 0, 1));
  return VertexOutput(pos, uv, instance);
}

[[shader("fragment")]] void frag(VertexOutput in)
{
  BezierRegions fill_type = fills[in.instance];
  BezierRegions regions   = bary::bezier_region(in.uv);
  var           fill      = regions & fill_type;

  if (fill != 0)
  {
    discard;
  }
}
