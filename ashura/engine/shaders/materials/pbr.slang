/// SPDX-License-Identifier: MIT

#pragma once

#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

struct PbrShading
{
  f32x3 albedo              = f32x3(0);
  f32   metallic            = 0;
  f32   roughness           = 0;
  f32x3 normal              = 0;
  f32x3 emission            = 0;
  f32   ior                 = 1.5;
  f32   clearcoat           = 0;
  f32   clearcoat_roughness = 0;
  f32x3 clearcoat_normal    = f32x3(0);
  f32   lit                 = 1;

  f32x3 radiance(f32x3 light_pos, f32x3 world_pos, f32x3 V, f32 NoV,
                 f32x3 irradiance)
  {
    f32x3 Lu  = light_pos.xyz - world_pos;
    f32x3 L   = normalize(Lu);
    f32x3 H   = normalize(L + V);
    f32   HoV = dot(H, V);
    f32   NoH = dot(normal, H);
    f32   NoL = clamp(dot(normal, L), 0.0, 1.0);

    f32x3 metal_brdf =
      pbr::metal_brdf(albedo, HoV, roughness, NoL, NoV, NoH, normal, H);
    f32x3 dielectric = pbr::dielectric_brdf(albedo, roughness, NoL, NoV, NoH,
                                            normal, H, HoV, ior);
    f32x3 radiance   = irradiance * pbr::brdf(dielectric, metal_brdf, metallic);
    f32   clearcoat_brdf = pbr::specular_brdf(
      clearcoat_roughness, dot(clearcoat_normal, L), dot(clearcoat_normal, V),
      dot(clearcoat_normal, H), clearcoat_normal, H);
    radiance =
      pbr::fresnel_coat(clearcoat_normal, ior, clearcoat, radiance,
                        f32x3(clearcoat_brdf), dot(clearcoat_normal, V));
  }
};

struct PbrFragmentInfo
{
  f32x4 world_pos;
  f32x4 screen_pos;
  f32x2 uv;
  u32   instance;
};

interface PbrMaterial
{
  PbrShading shade(PbrFragmentInfo info, SamplerState samplers[],
                   Texture2D textures[]);
}

struct PbrCoreMaterial : PbrMaterial
{
  f32x4        albedo;      // only xyz
  f32x4        emission;    // only xyz
  f32          metallic;
  f32          roughness;
  f32          normal;
  f32          occlusion;
  f32          ior;
  f32          clearcoat;
  f32          clearcoat_roughness;
  f32          clearcoat_normal;
  f32          lit;
  SamplerIndex sampler;
  TextureIndex albedo_map;
  TextureIndex metallic_map;
  TextureIndex roughness_map;
  TextureIndex normal_map;
  TextureIndex occlusion_map;
  TextureIndex emission_map;
  TextureIndex clearcoat_map;
  TextureIndex clearcoat_roughness_map;
  TextureIndex clearcoat_normal_map;

  PbrShading shade(PbrFragmentInfo info, SamplerState samplers[],
                   Texture2D textures[])
  {
    var s       = samplers[sampler];
    var norm_uv = info.uv + 0.5;

    PbrShading sh;
    // clang-format off
    sh.albedo = textures[NonUniformResourceIndex(albedo_map)].Sample(s, norm_uv).rgb * albedo.xyz;
    sh.metallic = textures[NonUniformResourceIndex(metallic_map)].Sample(s, norm_uv).r * metallic;
    sh.roughness = textures[NonUniformResourceIndex(roughness_map)].Sample(s, norm_uv).r * roughness;

    // convert from perceptual roughness to actual roughness
    sh.roughness           = pow2(sh.roughness);
    sh.clearcoat_roughness = pow2(sh.clearcoat_roughness);

    sh.normal = textures[NonUniformResourceIndex(normal_map)].Sample(s, norm_uv).rgb * normal;
  
    f32 occlusion = textures[NonUniformResourceIndex(occlusion_map)].Sample(s, norm_uv).r * occlusion;
    sh.albedo = occlusion * sh.albedo;


    sh.emission = textures[NonUniformResourceIndex(emission_map)].Sample(s, norm_uv).rgb * emission.xyz;
    sh.ior = ior;
    sh.clearcoat = textures[NonUniformResourceIndex(clearcoat_map)].Sample(s, norm_uv).r * clearcoat;
    sh.clearcoat_roughness = textures[NonUniformResourceIndex(clearcoat_roughness_map)].Sample(s, norm_uv).r * clearcoat_roughness;
    sh.clearcoat_normal = textures[NonUniformResourceIndex(clearcoat_normal_map)].Sample(s, norm_uv).rgb * clearcoat_normal;
    sh.lit  = lit;

    // clang-format on

    return sh;
  }
};

interface PbrLighting
{
  f32x4 position();
  f32x4 irradiance(f32x4 world_pos, f32x4 surface_normal);
}

struct PunctualLight : PbrLighting
{
  f32x4 direction;    // xyz
  f32x4 pos;          // xyz
  f32x4 color;
  f32   inner_angle;
  f32   outer_angle;
  f32   intensity;
  f32   radius;

  f32x4 position()
  {
    return pos;
  }

  f32x4 irradiance(f32x4 world_pos, f32x4 surface_normal)
  {
    f32x3 Lu  = (pos - world_pos).xyz;
    f32x3 L   = normalize(Lu);
    f32   NoL = clamp(dot(surface_normal.xyz, L), 0.0, 1.0);
    f32   attenuation =
      ::light::square_falloff_attenuation(Lu, 1 / max(radius, EPSILON)) *
      ::light::spot_angle_attenuation(L, direction.xyz, inner_angle,
                                      outer_angle);
    f32x3 irradiance = this.intensity * attenuation * NoL * color.xyz;
    return f32x4(irradiance, 1);
  }
};

// [ ] image-based light
// [ ] rectangular lights
// [ ] https://developer.nvidia.com/gpugems/gpugems/part-iii-materials/chapter-19-image-based-lighting
