/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/custom.inl.slang"
#include "materials/ngon.slang"
#include "modules/core.slang"
#include "modules/types.slang"

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos : SV_Position;
  [[vk::location(1)]] f32x4               world_pos : WORLD_POSITION;
  [[vk::location(2)]] f32x4               rel_pos : REL_POS;
  [[vk::location(3)]] nointerpolation u32 instance : INSTANCE;
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 3)]]
StructuredBuffer<f32x4x4> transforms;

[[vk::binding(0, 4)]]
StructuredBuffer<f32x2> vertices;

[[vk::binding(0, 5)]]
StructuredBuffer<u32> indices;

#ifndef MATERIAL_TYPE

typedef quad::FlatMaterial MaterialType;

#else

typedef MATERIAL_TYPE MaterialType;

#endif

// [ ] line shader; material with tiling? for dashed lines

[[vk::binding(0, 6)]]
StructuredBuffer<MaterialType> materials;

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  u32     idx             = indices[vertex];
  f32x2   rel_pos         = vertices[idx];
  f32x4x4 world_transform = transforms[instance];
  f32x4   world_pos       = mul(world_transform, f32x4(rel_pos, 0.0, 1.0));
  f32x4   screen_pos      = mul(world_to_ndc, world_pos);
  return VertexOutput(screen_pos, world_pos, f32x4(rel_pos, 0, 1), instance);
}

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
  var material = materials[in.instance];
  var frag     = ngon::FragmentInfo(in.rel_pos, in.world_pos, in.screen_pos);
  return material.shade(frag, samplers, textures);
}
