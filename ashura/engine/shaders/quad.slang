/// SPDX-License-Identifier: MIT

#pragma once
#include "custom.inl.slang"
#include "items.slang"
#include "materials/quad.slang"
#include "modules/types.slang"

static const f32x2 REL_POS[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

#ifndef ITEM_TYPE
#    define ITEM_TYPE QuadGradientItem
#endif

struct VertexOutput
{
    [[vk::location(0)]] f32x4               screen_pos : SV_Position;
    [[vk::location(1)]] f32x4               world_pos  : WORLD_POSITION;
    [[vk::location(2)]] f32x2               rel_pos    : REL_POSITION;
    [[vk::location(3)]] f32x2               uv         : UV;
    [[vk::location(4)]] nointerpolation u32 instance   : INSTANCE;
};

struct QuadShaderParams
{
    f32x4x4                                       world_to_ndc;
    DescriptorHandle<StructuredBuffer<ITEM_TYPE>> quads;
};

[[vk::push_constant]]
ConstantBuffer<QuadShaderParams> params;

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_VulkanInstanceID,
                                       u32 vertex : SV_VulkanVertexID)
{
    var   quad       = params.quads[instance];
    var   pos        = quad.corners[vertex];
    f32x4 world_pos  = mul(quad.world_transform, pos);
    f32x4 screen_pos = mul(params.world_to_ndc, world_pos);
    f32x2 uv         = mul(quad.uv_transform, pos).xy;
    f32x2 rel_pos    = REL_POS[vertex];
    return VertexOutput(screen_pos, world_pos, rel_pos, uv, instance);
}

[[shader("fragment")]] f32x4 frag(VertexOutput in) : COLOR
{
    var material = params.quads[in.instance].material;
    var frag =
      QuadFragmentInfo(in.world_pos, in.screen_pos, in.rel_pos, in.uv, in.instance);
    return material.shade(frag, samplers, textures);
}
