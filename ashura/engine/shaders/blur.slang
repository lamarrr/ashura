/// SPDX-License-Identifier: MIT

#include "modules/kawase.slang"
#include "modules/types.slang"

/// @param radius relative to texture dimensions
struct BlurShaderParam
{
  f32x4             uv;
  f32x2             radius;
  u32               sampler;
  u32               tex;
  row_major f32x4x4 uv_transform;
};

struct VertexOutput
{
  f32x4                     position : SV_Position;
  [[vk::location(0)]] f32x2 uv : UV_COORD;
};

[[vk::push_constant]]
row_major ConstantBuffer<BlurShaderParam, Std430DataLayout> constants;

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

static constexpr f32x2 VERTEX_BUFFER[] = {f32x2(-0.5, -0.5), f32x2(0.5, -0.5),
                                          f32x2(0.5, 0.5), f32x2(-0.5, 0.5)};

[[shader("vertex")]]
VertexOutput vs_main(u32 vertex_idx : SV_VertexID)
{
  f32x2 local_pos = VERTEX_BUFFER[vertex_idx];
  return VertexOutput(f32x4(local_pos * 2, 0, 1), local_pos + 0.5);
}

[[shader("fragment")]]
f32x4 fs_upsample_main([[vk::location(0)]] f32x2 uv : UV_COORD) :
  COLOR
{
  var tex_uv = lerp(constants.uv.xy, constants.uv.zw, uv);
  return f32x4(
    kawase::upsample(samplers[NonUniformResourceIndex(constants.sampler)],
                     textures[NonUniformResourceIndex(constants.tex)], tex_uv,
                     constants.radius)
      .xyz,
    1.0);
}

[[shader("fragment")]]
f32x4 fs_downsample_main([[vk::location(0)]] f32x2 uv : UV_COORD) :
  COLOR
{
  var tex_uv = lerp(constants.uv.xy, constants.uv.zw, uv);
  return f32x4(
    kawase::downsample(samplers[NonUniformResourceIndex(constants.sampler)],
                       textures[NonUniformResourceIndex(constants.tex)], tex_uv,
                       constants.radius)
      .xyz,
    1.0);
}
