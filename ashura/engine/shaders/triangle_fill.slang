/// SPDX-License-Identifier: MIT

#pragma once
#include "custom.inl.slang"
#include "items.slang"
#include "materials/triangle_fill.slang"

#ifndef ITEM_TYPE
#  define ITEM_TYPE TriangleSetGradientItem
#endif

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 3)]]
StructuredBuffer<ITEM_TYPE> sets;

[[vk::binding(0, 4)]]
StructuredBuffer<TriangleVertex> vertices;

[[vk::binding(0, 5)]]
StructuredBuffer<u32> indices;

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos : SV_Position;
  [[vk::location(1)]] f32x4               world_pos  : WORLD_POSITION;
  [[vk::location(2)]] f32x4               color      : VTX_COLOR;
  [[vk::location(3)]] f32x2               uv         : UV;
  [[vk::location(4)]] nointerpolation u32 instance   : INSTANCE;
};

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  u32   idx        = indices[vertex];
  var   vtx        = vertices[idx];
  var   set        = sets[instance];
  f32x4 world_pos  = mul(set.world_transform, f32x4(vtx.pos(), 0.0, 1.0));
  f32x4 screen_pos = mul(world_to_ndc, world_pos);
  f32x2 uv         = mul(set.uv_transform, f32x4(vtx.pos(), 0.0, 1.0)).xy;
  return VertexOutput(screen_pos, world_pos, vtx.color(), uv, instance);
}

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
  var material = sets[in.instance].material;
  var frag     = TriangleSetFragmentInfo(in.world_pos, in.screen_pos, in.color,
                                         in.uv, in.instance);
  return material.shade(frag, samplers, textures);
}
