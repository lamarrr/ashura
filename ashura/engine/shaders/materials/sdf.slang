/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/quad.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

struct SdfFragmentInfo
{
  f32x4 world_pos;
  f32x4 screen_pos;
  f32x2 uv;
  f32x2 sdf_uv;
  f32x2 bbox_rel_pos;
};

interface SdfMaterial
{
  f32x4 flood(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[]);

  f32x4 soften(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[]);

  f32x4 feather(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[]);

  f32x4 stroke(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[]);

  f32 sdf(SdfFragmentInfo info, SamplerState samplers[], Texture2D textures[]);
}

struct SdfGradientMaterial : SdfMaterial
{
  f32x4        top;
  f32x4        bottom;
  f32x2        gradient_rotor;    // (cos(theta), sin(theta))
  f32          gradient_center;
  SamplerIndex sampler;
  TextureIndex texture;
  SamplerIndex sdf_sampler;
  TextureIndex sdf_map;

  f32x4 shade(SdfFragmentInfo info, SamplerState samplers[],
              Texture2D textures[])
  {
    f32x2 norm_uv   = info.uv + 0.5;
    var   smp       = samplers[NonUniformResourceIndex(this.sampler)];
    var   tex       = textures[NonUniformResourceIndex(texture)];
    f32x4 tex_color = tex.Sample(smp, norm_uv);
    f32   t         = dot(gradient_rotor, info.uv) + gradient_center + 0.5;
    f32x4 color     = lerp(top, bottom, clamp(t, 0.0, 1.0));
    return color * tex_color;
  }

  f32x4 flood(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color.w *= edge_mask;
    return color;
  }

  f32x4 soften(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color.w *= soften_alpha;
    return color;
  }

  f32x4 feather(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color.w *= feather_alpha;
    return color;
  }

  f32x4 stroke(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[])
  {
    var feather_color = shade(info, samplers, textures);
    feather_color.w *= ring_mask;
    return feather_color;
  }

  f32 sdf(SdfFragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 norm_sdf_uv = info.sdf_uv + 0.5;
    return textures[NonUniformResourceIndex(sdf_map)]
      .Sample(samplers[NonUniformResourceIndex(sdf_sampler)], norm_sdf_uv)
      .x;
  }
};

struct SdfNoiseMaterial : SdfMaterial
{
  f32x4        intensity;
  SamplerIndex sdf_sampler;
  TextureIndex sdf_map;

  f32x4 shade(SdfFragmentInfo info, SamplerState samplers[],
              Texture2D textures[])
  {
    f32x4 n = noise2D(info.screen_pos.xy) * this.intensity;
    return n;
  }

  f32x4 flood(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color *= edge_mask;
    return color;
  }

  f32x4 soften(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color *= soften_alpha;
    return color;
  }

  f32x4 feather(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color *= feather_alpha;
    return color;
  }

  f32x4 stroke(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[])
  {
    var feather_color = shade(info, samplers, textures);
    return ring_mask * feather_color;
  }

  f32 sdf(SdfFragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 norm_sdf_uv = info.sdf_uv + 0.5;
    return textures[NonUniformResourceIndex(sdf_map)]
      .Sample(samplers[NonUniformResourceIndex(sdf_sampler)], norm_sdf_uv)
      .x;
  }
};

struct SdfMeshGradientMaterial : SdfMaterial
{
  f32x4        colors[4];
  f32x2        min;
  f32x2        max;
  f32          aspect_ratio;    // x/y
  f32          frequency;       // = 5.
  f32          amplitude;       // = 30.
  f32          time;
  SamplerIndex sdf_sampler;
  TextureIndex sdf_map;

  f32x2x2 rot(f32 r)
  {
    f32 x = cos(r);
    f32 y = sin(r);
    return f32x2x2(x, -y, y, x);
  }

  f32x2 distort(f32x2 t)
  {
    f32 degree = noise2D(f32x2(time * 0.1, t.x * t.y));
    t.y *= 1.0 / aspect_ratio;
    t = mul(t, rot(radians((degree - 0.25) * 720.0 + 180.0)));
    t.y *= aspect_ratio;
    f32 speed = time * 2.0;
    t.x += sin(t.y * frequency + speed) / amplitude;
    t.y += sin(t.x * frequency * 1.5 + speed) / (amplitude * 0.5);
    return t;
  }

  f32x4 shade(SdfFragmentInfo info, SamplerState samplers[],
              Texture2D textures[])
  {
    f32x2 t        = distort(info.bbox_rel_pos);
    t.x            = smoothstep(min.x, max.x, t.x);
    t.y            = smoothstep(min.y, max.y, t.y);
    f32x4 y0_layer = lerp(colors[0], colors[1], t.x);
    f32x4 y1_layer = lerp(colors[2], colors[3], t.x);
    f32x4 layer    = lerp(y0_layer, y1_layer, t.y);
    return layer;
  }

  f32x4 flood(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color *= edge_mask;
    return color;
  }

  f32x4 soften(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color *= soften_alpha;
    return color;
  }

  f32x4 feather(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[])
  {
    var color = shade(info, samplers, textures);
    color *= feather_alpha;
    return color;
  }

  f32x4 stroke(SdfFragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[])
  {
    var feather_color = shade(info, samplers, textures);
    return ring_mask * feather_color;
  }

  f32 sdf(SdfFragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 norm_sdf_uv = info.sdf_uv + 0.5;
    return textures[NonUniformResourceIndex(sdf_map)]
      .Sample(samplers[NonUniformResourceIndex(sdf_sampler)], norm_sdf_uv)
      .x;
  }
};
