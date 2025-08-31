/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/quad.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

namespace triangle_fill
{

interface Material
{
  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[]);
}

struct FragmentInfo
{
  f32x4 world_pos;
  f32x4 screen_pos;
  f32x4 color;
  f32x2 uv;
  u32   instance;
};

struct TextureMaterial : Material
{
  SamplerIndex sampler;
  TextureIndex texture;

  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 norm_uv   = info.uv + 0.5F;
    var   smp       = samplers[NonUniformResourceIndex(this.sampler)];
    var   tex       = textures[NonUniformResourceIndex(texture)];
    f32x4 tex_color = tex.Sample(smp, norm_uv);
    return info.color * tex_color;
  }
};

}    // namespace triangle_fill
