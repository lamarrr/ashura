/// SPDX-License-Identifier: MIT

#pragma once
#include "modules/core.slang"
#include "modules/light.slang"
#include "modules/pbr.slang"
#include "modules/types.slang"

struct PunctualLight
{
  f32x4 direction;    // xyz
  f32x4 position;     // xyz
  f32x4 color;
  f32   inner_angle;
  f32   outer_angle;
  f32   intensity;
  f32   radius;
};

// [ ] update C++
struct PbrExtParam
{
  row_major float4x4 transform;
  f32x4              eye_position;
  f32x4              albedo;    // only xyz
  f32                metallic;
  f32                roughness;
  f32                normal;
  f32                occlusion;
  f32x4              emissive;    // only xyz
  f32                ior;
  f32                clearcoat;
  f32                clearcoat_roughness;
  f32                clearcoat_normal;
  u32                sampler;
  u32                albedo_map;
  u32                metallic_map;
  u32                roughness_map;
  u32                normal_map;
  u32                occlusion_map;
  u32                emissive_map;
  u32                clearcoat_map;
  u32                clearcoat_roughness_map;
  u32                clearcoat_normal_map;
  u32                first_light;
  u32                first_vertex;
};

struct PbrBasicParam;

struct PbrMaterial
{
  f32x3 albedo              = f32x3(1);
  f32   metallic            = 0;
  f32   roughness           = 0;
  f32x3 normal              = 0;
  f32   occlusion           = 0;
  f32x3 emissive            = 0;
  f32   ior                 = 1.5;
  f32   clearcoat           = 0;
  f32   clearcoat_roughness = 0;
  f32x3 clearcoat_normal    = f32x3(0);

  static PbrMaterial load(PbrExtParam p, SamplerState s, f32x2 uv)
  {
    PbrMaterial m;
    // clang-format off
    m.albedo = textures[NonUniformResourceIndex((uint) p.albedo_map)].Sample(s, uv).rgb;
    m.metallic = textures[NonUniformResourceIndex((uint) p.metallic_map)].Sample(s, uv).r;
    m.roughness = textures[NonUniformResourceIndex((uint) p.roughness_map)].Sample(s, uv).r;
    m.normal = textures[NonUniformResourceIndex((uint) p.normal_map)].Sample(s, uv).rgb;
    m.occlusion = textures[NonUniformResourceIndex((uint) p.occlusion_map)].Sample(s, uv).r;
    m.emissive = textures[NonUniformResourceIndex((uint) p.emissive_map)].Sample(s, uv).rgb;
    m.clearcoat = textures[NonUniformResourceIndex((uint) p.clearcoat_map)].Sample(s, uv).r;
    m.clearcoat_roughness = textures[NonUniformResourceIndex((uint) p.clearcoat_roughness_map)].Sample(s, uv).r;
    m.clearcoat_normal = textures[NonUniformResourceIndex((uint) p.clearcoat_normal_map)].Sample(s, uv).rgb;
    // clang-format on

    m.albedo *= p.albedo.xyz;
    m.metallic *= p.metallic;
    m.roughness *= p.roughness;
    m.normal *= p.normal;
    m.occlusion *= p.occlusion;
    m.emissive *= p.emissive.xyz;
    m.ior = p.ior;
    m.clearcoat *= p.clearcoat;
    m.clearcoat_roughness *= p.clearcoat_roughness;
    m.clearcoat_normal *= p.clearcoat_normal;

    return m;
  }
};

struct Vertex
{
  f32x4 pos;
  f32x2 uv;
};

[[vk::push_constant]]
ConstantBuffer<float4x4, Std430DataLayout> world_to_view;

[[vk::binding(0, 0)]]
StructuredBuffer<Vertex> vtx_buffer;

[[vk::binding(0, 1)]]
StructuredBuffer<u32> idx_buffer;

[[vk::binding(0, 2)]]
StructuredBuffer<PbrExtParam> params;

[[vk::binding(0, 3)]]
StructuredBuffer<PunctualLight> lights;

[[vk::binding(0, 4)]]
SamplerState samplers[];

[[vk::binding(0, 5)]]
Texture2D textures[];

struct VertexOutput
{
  f32x4                                   position : SV_Position;
  [[vk::location(0)]] nointerpolation u32 instance : INSTANCE;
  [[vk::location(1)]] f32x4               world_pos : WORLD_POS;
  [[vk::location(2)]] f32x2               uv : UV_COORD;
};

[[shader("vertex")]]
VertexOutput vs_main(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  var p         = params[instance];
  var idx       = idx_buffer[vertex];
  var vtx       = vtx_buffer[p.first_vertex + idx];
  var world_pos = mul(p.transform, vtx.pos);
  return VertexOutput(mul(world_to_view, world_pos), instance, world_pos,
                      vtx.uv);
}

static constexpr u32 NUM_OBJECT_LIGHTS = 4;

[[shader("fragment")]]
f32x4 fs_main([[vk::location(0)]] u32   instance : INSTANCE,
              [[vk::location(1)]] f32x4 world_pos : WORLD_POS,
              [[vk::location(2)]] f32x2 uv : UV_COORD)
{
  var p = params[instance];
  var m = PbrMaterial::load(p, samplers[p.sampler], uv);

  // convert from perceptual roughness to actual roughness
  m.roughness           = pow(m.roughness, 2);
  m.clearcoat_roughness = pow(m.clearcoat_roughness, 2);

  // [ ] is this correct?
  m.albedo *= m.occlusion;

  f32x3 luminance = m.emissive;

  f32x3 V   = normalize(p.eye_position.xyz - world_pos.xyz);
  f32   NoV = dot(m.normal, V);

  for (u32 i = 0; i < NUM_OBJECT_LIGHTS; i++)
  {
    // irradiance - light from source
    // radiance - reaction of the object to the light source
    var   light = lights[p.first_light + i];
    f32x3 Lu    = light.position.xyz - world_pos.xyz;
    f32x3 L     = normalize(Lu);
    f32x3 H     = normalize(L + V);
    f32   HoV   = dot(H, V);
    f32   NoV   = dot(m.normal, V);
    f32   NoH   = dot(m.normal, H);
    f32   NoL   = clamp(dot(m.normal, L), 0.0, 1.0);
    f32   attenuation =
      square_falloff_attenuation(Lu, 1 / max(light.radius, EPSILON)) *
      spot_angle_attenuation(L, light.direction.xyz, light.inner_angle,
                             light.outer_angle);
    f32x3 irradiance = light.intensity * attenuation * NoL * light.color.xyz;
    f32x3 metal_brdf_v =
      metal_brdf(m.albedo, HoV, m.roughness, NoL, NoV, NoH, m.normal, H);
    f32x3 dielectric_v = dielectric_brdf(m.albedo, m.roughness, NoL, NoV, NoH,
                                         m.normal, H, HoV, m.ior);
    f32x3 radiance     = brdf(dielectric_v, metal_brdf_v, m.metallic);
    f32   clearcoat_brdf =
      specular_brdf(m.clearcoat_roughness, dot(m.clearcoat_normal, L),
                    dot(m.clearcoat_normal, V), dot(m.clearcoat_normal, H),
                    m.clearcoat_normal, H);
    radiance = fresnel_coat(m.clearcoat_normal, m.ior, m.clearcoat, radiance,
                            f32x3(clearcoat_brdf), dot(m.clearcoat_normal, V));
    luminance += radiance;
  }

  // [ ] emissive bloom, separate channel?
  return f32x4(luminance, 1);
}
