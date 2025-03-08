/// SPDX-License-Identifier: MIT

import "modules/kawase.slang";

/// @param radius relative to texture dimensions
struct Params
{
  float2   uv[2];
  float2   radius;
  uint32_t sampler;
  uint32_t tex;
};

[[vk::push_constant]]
Params p;

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(1, 0)]]
Texture2D textures[];

static constexpr float2 VERTEX_BUFFER[] = {
  float2(-0.5, -0.5), float2(0.5, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5)};

struct VertexOutput
{
  float4 position : SV_Position;
  float2 uv : UV_COORD;
};

[[shader("vertex")]]
VertexOutput vs_main(uint32_t vertex_idx : SV_VertexID)
{
  float2 i_pos = VERTEX_BUFFER[vertex_idx];
  return VertexOutput(float4(i_pos * 2, 0, 1), i_pos + 0.5);
}

[[shader("fragment")]]
float4 fs_upsample_main(float2 i_uv : UV_COORD) :
  COLOR
{
  float2 src_uv = lerp(p.uv[0], p.uv[1], i_uv);
  return upsample_clamped(samplers[NonUniformResourceIndex(p.sampler)],
                          textures[NonUniformResourceIndex(p.tex)], src_uv,
                          p.radius, p.uv[0], p.uv[1]);
}

[[shader("fragment")]]
float4 fs_downsample_main(float2 i_uv : UV_COORD) :
  COLOR
{
  float2 src_uv = lerp(p.uv[0], p.uv[1], i_uv);
  return downsample_clamped(samplers[NonUniformResourceIndex(p.sampler)],
                            textures[NonUniformResourceIndex(p.tex)], src_uv,
                            p.radius, p.uv[0], p.uv[1]);
}
