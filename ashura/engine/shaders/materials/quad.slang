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
  f32x4 rel_pos;
};

interface Material
{
  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[]);
}

struct FlatMaterial : Material
{
  f32x4     colors[2];
  f32x2     color_rotor;    // (cos(theta), sin(theta))
  f32x2     uv0;
  f32x2     uv1;
  SamplerId sampler;
  TextureId texture;

  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 uv     = info.rel_pos.xy + 0.5;
    f32x2 tex_uv = lerp(this.uv0, this.uv1, uv);
    f32x2 color_rel_pos =
      mul(rotor_matrix(color_rotor), f32x3(info.rel_pos.xy, 1)).xy;
    f32x2        color_uv  = color_rel_pos + 0.5;
    SamplerState smp       = samplers[NonUniformResourceIndex(this.sampler)];
    Texture2D    tex       = textures[NonUniformResourceIndex(texture)];
    f32x4        tex_color = tex.Sample(smp, tex_uv);
    f32x4        color     = lerp(colors[0], colors[1], uv.y);
    return color * tex_color;
  }
};

struct NoiseMaterial : Material
{
  f32x4 intensity;
  f32x2 offset;

  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x4 n = noise2D(info.screen_pos.xy + offset) * this.intensity;
    return n;
  }
};

}    // namespace quad
