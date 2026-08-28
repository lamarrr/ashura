/// SPDX-License-Identifier: MIT

#include "items.slang"
#include "modules/core.slang"
#include "modules/types.slang"

#ifndef DOWNSAMPLE
#    define DOWNSAMPLE 1
#endif

#ifndef ITEM_TYPE
#    define ITEM_TYPE BlurItem
#endif

struct VertexOutput
{
    f32x4                                   position : SV_Position;
    [[vk::location(0)]] f32x2               uv       : UV_COORD;
    [[vk::location(1)]] nointerpolation u32 instance : INSTANCE;
};

struct BlurShaderParams
{
    ITEM_TYPE * blurs;
};

[[vk::push_constant]]
ConstantBuffer<BlurShaderParams> params;

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

static constexpr f32x2 VERTEX_BUFFER[] = {f32x2(-0.5, -0.5), f32x2(0.5, -0.5),
                                          f32x2(0.5, 0.5), f32x2(-0.5, 0.5)};

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_VulkanInstanceID,
                  u32 vertex_idx : SV_VulkanVertexID)
{
    f32x2 pos        = VERTEX_BUFFER[vertex_idx];
    var   blur       = params.blurs[instance];
    f32x4 world_pos  = mul(blur.world_transform, f32x4(pos, 0.0, 1.0));
    f32x4 screen_pos = mul(blur.world_to_ndc, world_pos);
    f32x2 uv         = mul(blur.uv_transform, f32x4(pos, 0.0, 1.0)).xy;
    return VertexOutput(f32x4(screen_pos.xy, 0, 1), uv, instance);
}

#if DOWNSAMPLE

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
    var blur = params.blurs[in.instance];
    return f32x4(downsample(samplers[NonUniformResourceIndex(blur.sampler)],
                            textures[NonUniformResourceIndex(blur.tex)], in.uv + 0.5,
                            blur.radius)
                   .xyz,
                 1.0);
}

#else

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
    var blur = params.blurs[in.instance];
    return f32x4(upsample(samplers[NonUniformResourceIndex(blur.sampler)],
                          textures[NonUniformResourceIndex(blur.tex)], in.uv + 0.5,
                          blur.radius)
                   .xyz,
                 1.0);
}

#endif
