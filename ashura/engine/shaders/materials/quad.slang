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
  u32   instance;
};

interface Material
{
  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[]);
}

struct FlatMaterial : Material
{
  f32x4     colors[2];
  f32x2     gradient_rotor;    // (cos(theta), sin(theta))
  f32x2     uv0;
  f32x2     uv1;
  f32       gradient_center;
  SamplerId sampler;
  TextureId texture;

  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 uv        = info.rel_pos.xy + 0.5;
    f32x2 tex_uv    = lerp(this.uv0, this.uv1, uv);
    var   smp       = samplers[NonUniformResourceIndex(this.sampler)];
    var   tex       = textures[NonUniformResourceIndex(texture)];
    f32x4 tex_color = tex.Sample(smp, tex_uv);
    f32   t     = dot(gradient_rotor, info.rel_pos.xy) + gradient_center + 0.5;
    f32x4 color = lerp(colors[0], colors[1], clamp(t, 0.0, 1.0));
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

struct MeshGradientMaterial : Material
{
  f32x4 colors[4];

  f32x4 shade(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    // [ ] finish
    f32x2 t          = f32x2(0);
    f32x2 smooth_min = f32x2(-0.25);
    f32x2 smooth_max = f32x2(0.5);
    t.x              = smoothstep(smooth_min.x, smooth_max.x, t.x);
    t.x              = smoothstep(smooth_min.y, smooth_max.y, t.y);
    f32x4 y0_layer   = lerp(colors[0], colors[1], t.x);
    f32x4 y1_layer   = lerp(colors[2], colors[3], t.x);
    f32x4 layer      = lerp(y0_layer, y1_layer, t.y);
  }
};

}    // namespace quad
