/// SPDX-License-Identifier: MIT

#pragma once
#include "modules/core.slang"
#include "modules/sdf.slang"

struct VertexOutput
{
  [[vk::location(0)]] f32x4 pos : SV_Position;
};

[[vk::binding(0, 0)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 1)]]
StructuredBuffer<f32x4x4> world_transforms;

[[vk::binding(0, 2)]]
StructuredBuffer<f32x2> vertices;

[[vk::binding(0, 3)]]
StructuredBuffer<u32> indices;

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 vertex : SV_VertexID)
{
  var idx             = indices[vertex];
  var rel_pos         = vertices[idx];
  var world_transform = world_transforms[instance];
  var world_pos       = mul(world_transform, f32x4(rel_pos, 0.0, 1.0));
  var screen_pos      = mul(world_to_ndc, world_pos);
  return VertexOutput(screen_pos);
}
