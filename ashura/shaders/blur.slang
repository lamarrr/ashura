/// SPDX-License-Identifier: MIT

#include "modules/kawase.slang"
#include "modules/types.slang"

/// @param radius relative to texture dimensions
struct BlurParam
{
  f32x2 uv[2];
  f32x2 radius;
  u16   sampler;
  u16   tex;
};

[[vk::push_constant]]
BlurParam p;

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(1, 0)]]
Texture2D textures[];

static constexpr f32x2 VERTEX_BUFFER[] = {f32x2(-0.5, -0.5), f32x2(0.5, -0.5),
                                          f32x2(0.5, 0.5), f32x2(-0.5, 0.5)};

struct VertexOutput
{
  f32x2 uv : UV_COORD;
  f32x4 position : SV_Position;
};

[[shader("vertex")]]
VertexOutput vs_main(uint32_t vertex_idx : SV_VertexID)
{
  f32x2 local_pos = VERTEX_BUFFER[vertex_idx];
  return VertexOutput(local_pos + 0.5, f32x4(local_pos * 2, 0, 1));
}

[[shader("fragment")]]
f32x4 fs_upsample_main(f32x2 uv : UV_COORD) :
  COLOR
{
  var tex_uv = lerp(p.uv[0], p.uv[1], uv);
  return upsample_clamped(samplers[NonUniformResourceIndex((uint) p.sampler)],
                          textures[NonUniformResourceIndex((uint) p.tex)],
                          tex_uv, p.radius, p.uv[0], p.uv[1]);
}

[[shader("fragment")]]
f32x4 fs_downsample_main(f32x2 uv : UV_COORD) :
  COLOR
{
  var tex_uv = lerp(p.uv[0], p.uv[1], uv);
  return downsample_clamped(samplers[NonUniformResourceIndex((uint) p.sampler)],
                            textures[NonUniformResourceIndex((uint) p.tex)],
                            tex_uv, p.radius, p.uv[0], p.uv[1]);
}
