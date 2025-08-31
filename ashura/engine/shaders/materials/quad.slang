/// SPDX-License-Identifier: MIT

#pragma once

#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

namespace quad
{

struct FragmentInfo
{
  f32x4 world_pos;
  f32x4 screen_pos;
  f32x2 rel_pos;
  f32x2 uv;
  u32   instance;
};

interface Material
{
  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[]);
}

struct FlatMaterial : Material
{
  f32x4        top;
  f32x4        bottom;
  f32x2        gradient_rotor;    // (cos(theta), sin(theta))
  f32          gradient_center;
  SamplerIndex sampler;
  TextureIndex texture;

  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 norm_uv   = info.uv + 0.5;
    var   smp       = samplers[NonUniformResourceIndex(this.sampler)];
    var   tex       = textures[NonUniformResourceIndex(texture)];
    f32x4 tex_color = tex.Sample(smp, norm_uv);
    f32   t         = dot(gradient_rotor, info.uv) + gradient_center + 0.5;
    f32x4 color     = lerp(top, bottom, clamp(t, 0.0, 1.0));
    return color * tex_color;
  }
};

struct NoiseMaterial : Material
{
  f32x4 intensity;

  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x4 n = noise2D(info.screen_pos.xy) * this.intensity;
    return n;
  }
};

}    // namespace quad
