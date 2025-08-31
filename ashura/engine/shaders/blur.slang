/// SPDX-License-Identifier: MIT

#include "items.slang"
#include "modules/core.slang"
#include "modules/types.slang"

#ifndef DOWNSAMPLE
#  define DOWNSAMPLE 1
#endif

#ifndef ITEM_TYPE
#  define ITEM_TYPE BlurItem
#endif

struct VertexOutput
{
  f32x4                                   position : SV_Position;
  [[vk::location(0)]] f32x2               uv       : UV_COORD;
  [[vk::location(1)]] nointerpolation u32 instance : INSTANCE;
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
StructuredBuffer<ITEM_TYPE> blurs;

static constexpr f32x2 VERTEX_BUFFER[] = {f32x2(-0.5, -0.5), f32x2(0.5, -0.5),
                                          f32x2(0.5, 0.5), f32x2(-0.5, 0.5)};

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_InstanceID, u32 vertex_idx : SV_VertexID)
{
  f32x2 pos        = VERTEX_BUFFER[vertex_idx];
  f32x2 screen_pos = pos * 2;
  f32x2 norm_uv    = pos + 0.5;
  var   blur       = blurs[instance];
  f32x2 region_uv  = lerp(blur.uv0, blur.uv1, norm_uv);
  return VertexOutput(f32x4(screen_pos, 0, 1), region_uv, instance);
}

#if DOWNSAMPLE

// [ ] config cmake

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
  var blur = blurs[in.instance];
  return f32x4(downsample(samplers[NonUniformResourceIndex(blur.sampler)],
                          textures[NonUniformResourceIndex(blur.tex)], in.uv,
                          blur.radius)
                 .xyz,
               1.0);
}

#else

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
  var blur = blurs[in.instance];
  return f32x4(upsample(samplers[NonUniformResourceIndex(blur.sampler)],
                        textures[NonUniformResourceIndex(blur.tex)], in.uv,
                        blur.radius)
                 .xyz,
               1.0);
}

#endif
