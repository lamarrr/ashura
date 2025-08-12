/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/quad.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

namespace sdf
{

struct FragmentInfo
{
  /// @brief the fragment's world-space position
  f32x4 world_pos;

  /// @brief the fragment's screen-space position
  f32x4 screen_pos;

  /// @brief the fragment's relative position within the bounding box
  f32x4 rel_pos;

  /// @brief the bounding-box's half-extent
  f32x2 half_bbox_extent;

  /// @brief the object's half-extent
  f32x2 half_extent;

  /// @brief the fragment's position within the bounding box
  f32x2 bbox_pos;
};

interface Material
{
  f32x4 flood(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[]);

  f32x4 soften(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[]);

  f32x4 feather(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[]);

  f32x4 stroke(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[]);

  f32 sdf(FragmentInfo info, SamplerState samplers[], Texture2D textures[]);
}

struct FlatMaterial : Material
{
  quad::FlatMaterial tint;
  u32                map_id;
  u32                sampler_id;

  f32x4 flood(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var color = tint.shade(i, samplers, textures);
    color.w *= edge_mask;
    return color;
  }

  f32x4 soften(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var color = tint.shade(i, samplers, textures);
    color.w *= soften_alpha;
    return color;
  }

  f32x4 feather(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var color = tint.shade(i, samplers, textures);
    color.w *= feather_alpha;
    return color;
  }

  f32x4 stroke(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var feather_color = tint.shade(i, samplers, textures);
    feather_color.w *= ring_mask;
    return feather_color;
  }

  f32 sdf(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 uv = info.rel_pos.xy + 0.5;
    return textures[NonUniformResourceIndex(map_id)]
      .Sample(samplers[NonUniformResourceIndex(sampler_id)], uv)
      .x;
  }
};

struct NoiseMaterial : Material
{
  quad::NoiseMaterial noise;
  SamplerId           sampler_id;
  TextureId           map_id;

  f32x4 flood(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var color = noise.shade(i, samplers, textures);
    color *= edge_mask;
    return color;
  }

  f32x4 soften(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var color = noise.shade(i, samplers, textures);
    color *= soften_alpha;
    return color;
  }

  f32x4 feather(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var color = noise.shade(i, samplers, textures);
    color *= feather_alpha;
    return color;
  }

  f32x4 stroke(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[])
  {
    var i = quad::FragmentInfo(info.world_pos, info.screen_pos, info.rel_pos);
    var feather_color = noise.shade(i, samplers, textures);
    return ring_mask * feather_color;
  }

  f32 sdf(FragmentInfo info, SamplerState samplers[], Texture2D textures[])
  {
    f32x2 uv = info.rel_pos.xy + 0.5;
    return textures[NonUniformResourceIndex(map_id)]
      .Sample(samplers[NonUniformResourceIndex(sampler_id)], uv)
      .x;
  }
};

#ifndef NUM_COMPOSITE_SDFS
#  define NUM_COMPOSITE_SDFS 4
#endif

interface CompositeMaterial
{
  f32x4 flood(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[]);

  f32x4 soften(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[]);

  f32x4 feather(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[]);

  f32x4 stroke(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[]);

  f32 sdf(FragmentInfo info, u32 i, SamplerState samplers[],
          Texture2D textures[]);
}

struct BlendedMaterial<M : Material> : CompositeMaterial
{
  M materials[NUM_COMPOSITE_SDFS];

  f32x4 flood(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
              SamplerState samplers[], Texture2D textures[])
  {
    return materials[0].flood(info, edge_signed_distance, edge_mask, samplers,
                              textures);
  }

  f32x4 soften(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 soften_signed_dist, f32 soften_alpha,
               SamplerState samplers[], Texture2D textures[])
  {
    return materials[0].soften(info, edge_signed_distance, edge_mask,
                               soften_signed_dist, soften_alpha, samplers,
                               textures);
  }

  f32x4 feather(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
                f32 feather_signed_dist, f32 feather_alpha,
                SamplerState samplers[], Texture2D textures[])
  {
    return materials[0].feather(info, edge_signed_distance, edge_mask,
                                feather_signed_dist, feather_alpha, samplers,
                                textures);
  }

  f32x4 stroke(FragmentInfo info, f32 edge_signed_distance, f32 edge_mask,
               f32 ring_signed_dist, f32 ring_mask, SamplerState samplers[],
               Texture2D textures[])
  {
    return materials[0].stroke(info, edge_signed_distance, edge_mask,
                               ring_signed_dist, ring_mask, samplers, textures);
  }

  f32 sdf(FragmentInfo info, u32 i, SamplerState samplers[],
          Texture2D textures[])
  {
    return materials[i].sdf(info, samplers, textures);
  }
};

}    // namespace sdf
