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
  u32               sampler;
  u32               albedo;
  u32               first_index;
  u32               first_vertex;
};

[[vk::push_constant]]
ConstantBuffer<f32x4x4> world_to_view;

[[vk::binding(0, 0)]]
StructuredBuffer<f32x2> vtx_buffer;

[[vk::binding(0, 1)]]
StructuredBuffer<u32> idx_buffer;

[[vk::binding(0, 2)]]
StructuredBuffer<NgonParam> params;

[[vk::binding(0, 3)]]
SamplerState samplers[];

[[vk::binding(0, 4)]]
Texture2D textures[];

struct VertexOutput
{
  f32x4                                   position : SV_Position;
  [[vk::location(0)]] nointerpolation u32 instance : INSTANCE;
  [[vk::location(1)]] f32x2               uv : UV_COORD;
};

[[shader("vertex")]]
VertexOutput vs_main(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  var p   = params[instance];
  var idx = idx_buffer[p.first_index + vertex];
  var pos = vtx_buffer[p.first_vertex + idx];
  return VertexOutput(
    mul(world_to_view, mul(p.transform, f32x4(pos, 0.0, 1.0))), instance,
    pos + 0.5);
}

[[shader("fragment")]]
f32x4 fs_main([[vk::location(0)]] u32   instance : INSTANCE,
              [[vk::location(1)]] f32x2 uv : UV_COORD) :
  COLOR
{
  var p      = params[instance];
  var tex_uv = lerp(p.uv.xy, p.uv.zw, uv);
  return bilerp(p.tint, uv) *
         textures[NonUniformResourceIndex(p.albedo)].Sample(
           samplers[NonUniformResourceIndex(p.sampler)], tex_uv * p.tiling);
}
