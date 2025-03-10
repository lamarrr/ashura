/// SPDX-License-Identifier: MIT

#pragma once
#include "modules/core.slang"
#include "modules/types.slang"

struct NgonParam
{
  row_major f32x4x4 transform;
  f32x4             tint[4];
  f32x4             uv;
  f32               tiling;
  u16               sampler;
  u16               albedo;
  u32               first_index;
  u32               first_vertex;
};

[[vk::push_constant]]
f32x4x4 world_to_view;

[[vk::binding(0, 0)]]
StructuredBuffer<f32x2> vtx_buffer;

[[vk::binding(1, 0)]]
StructuredBuffer<u32> idx_buffer;

[[vk::binding(2, 0)]]
StructuredBuffer<NgonParam> params;

[[vk::binding(3, 0)]]
SamplerState samplers[];

[[vk::binding(4, 0)]]
Texture2D textures[];

struct VertexOutput
{
  f32x2 uv : UV_COORD;
  f32x4 position : SV_Position;
};

[[shader("vertex")]]
VertexOutput vs_main(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  var p   = params[instance];
  var idx = idx_buffer[p.first_index + vertex];
  var pos = vtx_buffer[p.first_vertex + idx];
  return VertexOutput(
    pos + 0.5, mul(world_to_view, mul(p.transform, f32x4(pos, 0.0, 1.0))));
}

[[shader("fragment")]]
f32x4 fs_main(u32 instance : SV_InstanceID, f32x2 uv : UV_COORD) :
  COLOR
{
  var p      = params[instance];
  var tex_uv = lerp(p.uv.xy, p.uv.zw, uv);
  return bilerp(p.tint, uv) *
         textures[NonUniformResourceIndex((uint) p.albedo)].Sample(
           samplers[NonUniformResourceIndex((uint) p.sampler)],
           tex_uv * p.tiling);
}
