/// SPDX-License-Identifier: MIT

#include "modules/core.slang"
#include "modules/types.slang"

/// @param radius relative to texture dimensions
struct Blur
{
  f32x2     uv0;
  f32x2     uv1;
  f32x2     radius;
  SamplerId sampler;
  TextureId tex;
};

struct VertexOutput
{
  f32x4                     position : SV_Position;
  [[vk::location(0)]] f32x2 uv : UV_COORD;
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
ConstantBuffer<Blur> blur;

static constexpr f32x2 VERTEX_BUFFER[] = {f32x2(-0.5, -0.5), f32x2(0.5, -0.5),
                                          f32x2(0.5, 0.5), f32x2(-0.5, 0.5)};

[[shader("vertex")]]
VertexOutput vert(u32 vertex_idx : SV_VertexID)
{
  f32x2 rel_pos   = VERTEX_BUFFER[vertex_idx];
  f32x2 local_uv  = rel_pos + 0.5;
  f32x2 region_uv = lerp(blur.uv0, blur.uv1, local_uv);
  return VertexOutput(f32x4(rel_pos * 2, 0, 1), region_uv);
}

[[shader("fragment")]]
f32x4 frag_upsample(VertexOutput in) :
  COLOR
{
  return f32x4(upsample(samplers[NonUniformResourceIndex(blur.sampler)],
                        textures[NonUniformResourceIndex(blur.tex)], in.uv,
                        blur.radius)
                 .xyz,
               1.0);
}

[[shader("fragment")]]
f32x4 frag_downsample(VertexOutput in) :
  COLOR
{
  return f32x4(downsample(samplers[NonUniformResourceIndex(blur.sampler)],
                          textures[NonUniformResourceIndex(blur.tex)], in.uv,
                          blur.radius)
                 .xyz,
               1.0);
}
