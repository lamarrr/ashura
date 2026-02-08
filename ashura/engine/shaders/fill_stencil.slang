/// SPDX-License-Identifier: MIT

#pragma once
#include "modules/core.slang"
#include "modules/sdf.slang"

struct VertexOutput
{
    [[vk::location(0)]] f32x4 pos : SV_Position;
};

struct FillStencilShaderParams
{
    f32x4x4   world_to_ndc;
    f32x4x4 * world_transforms;
    f32x2 *   vertices;
    u32 *     indices;
};

[[vk::push_constant]]
ConstantBuffer<FillStencilShaderParams> params;

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_VulkanInstanceID,
                                       u32 vertex : SV_VulkanVertexID)
{
    var idx             = params.indices[vertex];
    var rel_pos         = params.vertices[idx];
    var world_transform = params.world_transforms[instance];
    var world_pos       = mul(world_transform, f32x4(rel_pos, 0.0, 1.0));
    var screen_pos      = mul(params.world_to_ndc, world_pos);
    return VertexOutput(screen_pos);
}

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
    return f32x4(0);
}
