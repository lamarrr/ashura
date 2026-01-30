/// SPDX-License-Identifier: MIT

#pragma once
#include "custom.inl.slang"
#include "items.slang"
#include "materials/triangle_fill.slang"

#ifndef ITEM_TYPE
#    define ITEM_TYPE TriangleSetGradientItem
#endif

struct TriangleFillShaderParams
{
    f32x4x4                                       world_to_ndc;
    DescriptorHandle<StructuredBuffer<ITEM_TYPE>> sets;
    DescriptorHandle<StructuredBuffer<Vertex2D>>  vertices;
    DescriptorHandle<StructuredBuffer<u32>>       indices;
};

[[vk::push_constant]]
ConstantBuffer<TriangleFillShaderParams> params;

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

struct VertexOutput
{
    [[vk::location(0)]] f32x4               screen_pos : SV_Position;
    [[vk::location(1)]] f32x4               world_pos  : WORLD_POSITION;
    [[vk::location(3)]] f32x2               uv         : UV;
    [[vk::location(4)]] nointerpolation u32 instance   : INSTANCE;
};

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_VulkanInstanceID, u32 vertex : SV_VulkanVertexID)
{
    u32   idx        = params.indices[vertex];
    var   vtx        = params.vertices[idx];
    var   set        = params.sets[instance];
    f32x4 world_pos  = mul(set.world_transform, f32x4(vtx.pos, 0.0, 1.0));
    f32x4 screen_pos = mul(params.world_to_ndc, world_pos);
    f32x2 uv         = mul(set.uv_transform, f32x4(vtx.pos, 0.0, 1.0)).xy;
    return VertexOutput(screen_pos, world_pos, uv, instance);
}

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
    var material = params.sets[in.instance].material;
    var frag = TriangleSetFragmentInfo(in.world_pos, in.screen_pos, in.uv, in.instance);
    return material.shade(frag, samplers, textures);
}
