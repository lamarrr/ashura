/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/custom.inl.slang"
#include "materials/quad.slang"
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
StructuredBuffer<f32x4x4> quads;

[[vk::binding(0, 4)]]
StructuredBuffer<f32x4x4> transforms;

#ifndef MATERIAL_TYPE

typedef quad::FlatMaterial MaterialType;

#else

typedef MATERIAL_TYPE MaterialType;

#endif

[[vk::binding(0, 5)]]
StructuredBuffer<MaterialType> materials;

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_InstanceID,
                                       u32 vertex : SV_VertexID)
{
  f32x4   rel_pos         = quads[instance][vertex];
  f32x4x4 world_transform = transforms[instance];
  f32x4   world_pos       = mul(world_transform, rel_pos);
  f32x4   screen_pos      = mul(world_to_ndc, world_pos);
  return VertexOutput(screen_pos, world_pos, rel_pos, instance);
}

[[shader("fragment")]] f32x4 frag(VertexOutput in) : COLOR
{
  var material = materials[in.instance];
  var frag     = quad::FragmentInfo(in.world_pos, in.screen_pos, in.rel_pos);
  return material.shade(frag, samplers, textures);
}
