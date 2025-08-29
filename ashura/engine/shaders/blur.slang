/// SPDX-License-Identifier: MIT

#include "modules/core.slang"
#include "modules/types.slang"

#ifndef DOWNSAMPLE
#  define DOWNSAMPLE 1
#endif

/// @param radius relative to texture dimensions
struct Blur
{
  f32x2        uv0;
  f32x2        uv1;
  f32x2        radius;
  SamplerIndex sampler;
  TextureIndex tex;
};

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
StructuredBuffer<Blur> blurs;

static constexpr f32x2 VERTEX_BUFFER[] = {f32x2(-0.5, -0.5), f32x2(0.5, -0.5),
                                          f32x2(0.5, 0.5), f32x2(-0.5, 0.5)};

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_InstanceID, u32 vertex_idx : SV_VertexID)
{
  f32x2 rel_pos   = VERTEX_BUFFER[vertex_idx];
  f32x2 local_uv  = rel_pos + 0.5;
  var   blur      = blurs[instance];
  f32x2 region_uv = lerp(blur.uv0, blur.uv1, local_uv);
  return VertexOutput(f32x4(rel_pos * 2, 0, 1), region_uv, instance);
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
