/// SPDX-License-Identifier: MIT

import "modules/pbr.slang";
import "modules/core.slang";
import "modules/light.slang";

struct PunctualLight
{
  float4 direction;    // xyz
  float4 position;     // xyz
  float4 color;
  float  inner_angle;
  float  outer_angle;
  float  intensity;
  float  radius;
};

struct Params
{
  row_major float4x4 transform;
  float4             eye_position;
  float4             albedo;    // only xyz
  float              metallic;
  float              roughness;
  float              normal;
  float              occlusion;
  float4             emissive;    // only xyz
  float              ior;
  float              clearcoat;
  float              clearcoat_roughness;
  float              clearcoat_normal;
  uint32_t           sampler;
  uint32_t           albedo_map;
  uint32_t           metallic_map;
  uint32_t           roughness_map;
  uint32_t           normal_map;
  uint32_t           occlusion_map;
  uint32_t           emissive_map;
  uint32_t           clearcoat_map;
  uint32_t           clearcoat_roughness_map;
  uint32_t           clearcoat_normal_map;
  uint32_t           first_vertex;
  uint32_t           first_light;
};

struct Material
{
  float3 albedo              = float3(1);
  float  metallic            = 0;
  float  roughness           = 0;
  float3 N                   = 0;
  float  occlusion           = 0;
  float3 emissive            = 0;
  float  ior                 = 1.5;
  float  clearcoat           = 0;
  float  clearcoat_roughness = 0;
  float3 clearcoat_normal    = float3(0);
};

struct Vertex
{
  float4 pos;
  float2 uv;
};

[[vk::binding(0, 0)]]
StructuredBuffer<Vertex> vtx_buffer;

[[vk::binding(1, 0)]]
StructuredBuffer<uint32_t> idx_buffer;

[[vk::binding(2, 0)]]
StructuredBuffer<Params> params;

[[vk::binding(3, 0)]]
StructuredBuffer<PunctualLight> lights;

[[vk::binding(4, 0)]]
SamplerState samplers[];

[[vk::binding(5, 0)]]
Texture2D textures[];

[[vk::push_constant]]
float4x4 world_to_view;

struct VertexOutput
{
  float4                   world_pos : WORLD_POS;
  float2                   uv : UV_COORD;
  nointerpolation uint32_t idx : INSTANCE_IDX;
  float4                   position : SV_Position;
};

[[shader("vertex")]]
VertexOutput main(uint32_t instance_idx : SV_InstanceID,
                  uint32_t vertex_idx : SV_VertexID)
{
  Params   p         = params[instance_idx];
  uint32_t idx       = idx_buffer[vertex_idx];
  Vertex   vtx       = vtx_buffer[p.first_vertex + idx];
  float4   world_pos = mul(p.transform, vtx.pos);
  return VertexOutput(world_pos, vtx.uv, instance_idx,
                      mul(world_to_view, world_pos));
}

Material load_material(SamplerState smp, float2 uv, Params p)
{
  // clang-format off
  Material m;
  m.albedo = textures[NonUniformResourceIndex(p.albedo_map)].Sample(smp, uv).rgb;
  m.metallic = textures[NonUniformResourceIndex(p.metallic_map)].Sample(smp, uv).r;
  m.roughness = textures[NonUniformResourceIndex(p.roughness_map)].Sample(smp, uv).r;
  m.N = textures[NonUniformResourceIndex(p.normal_map)].Sample(smp, uv).rgb;
  m.occlusion = textures[NonUniformResourceIndex(p.occlusion_map)].Sample(smp, uv).r;
  m.emissive = textures[NonUniformResourceIndex(p.emissive_map)].Sample(smp, uv).rgb;
  m.clearcoat = textures[NonUniformResourceIndex(p.clearcoat_map)].Sample(smp, uv).r;
  m.clearcoat_roughness = textures[NonUniformResourceIndex(p.clearcoat_roughness_map)].Sample(smp, uv).r;
  m.clearcoat_normal = textures[NonUniformResourceIndex(p.clearcoat_normal_map)].Sample(smp, uv).rgb;
  // clang-format on

  m.albedo *= p.albedo.xyz;
  m.metallic *= p.metallic;
  m.roughness *= p.roughness;
  m.N *= p.normal;
  m.occlusion *= p.occlusion;
  m.emissive *= p.emissive.xyz;
  m.ior = p.ior;
  m.clearcoat *= p.clearcoat;
  m.clearcoat_roughness *= p.clearcoat_roughness;
  m.clearcoat_normal *= p.clearcoat_normal;

  return m;
}

static constexpr uint32_t NUM_OBJECT_LIGHTS = 4;

[[shader("fragment")]]
float4 main(float4 i_world_pos : WORLD_POS, float2 i_uv : UV_COORD,
            uint32_t i_idx : INSTANCE_IDX)
{
  Params   p = params[i_idx];
  Material m = load_material(samplers[p.sampler], i_uv, p);

  // convert from perceptual roughness to actual roughness
  m.roughness           = pow(m.roughness, 2);
  m.clearcoat_roughness = pow(m.clearcoat_roughness, 2);

  // [ ] is this correct?
  m.albedo *= m.occlusion;

  float3 luminance = m.emissive;

  float3 V   = normalize(p.eye_position.xyz - i_world_pos.xyz);
  float  NoV = dot(m.N, V);

  for (uint32_t i = 0; i < NUM_OBJECT_LIGHTS; i++)
  {
    // irradiance - light from source
    // radiance - reaction of the object to the light source
    PunctualLight light = lights[p.first_light + i];
    float3        Lu    = light.position.xyz - i_world_pos.xyz;
    float3        L     = normalize(Lu);
    float3        H     = normalize(L + V);
    float         HoV   = dot(H, V);
    float         NoV   = dot(m.N, V);
    float         NoH   = dot(m.N, H);
    float         NoL   = clamp(dot(m.N, L), 0.0, 1.0);
    float         attenuation =
      square_falloff_attenuation(Lu, 1 / max(light.radius, EPSILON)) *
      spot_angle_attenuation(L, light.direction.xyz, light.inner_angle,
                             light.outer_angle);
    float3 irradiance = light.intensity * attenuation * NoL * light.color.xyz;
    float3 metal_brdf_v =
      metal_brdf(m.albedo, HoV, m.roughness, NoL, NoV, NoH, m.N, H);
    float3 dielectric_v =
      dielectric_brdf(m.albedo, m.roughness, NoL, NoV, NoH, m.N, H, HoV, m.ior);
    float3 radiance = brdf(dielectric_v, metal_brdf_v, m.metallic);
    float  clearcoat_brdf =
      specular_brdf(m.clearcoat_roughness, dot(m.clearcoat_normal, L),
                    dot(m.clearcoat_normal, V), dot(m.clearcoat_normal, H),
                    m.clearcoat_normal, H);
    radiance = fresnel_coat(m.clearcoat_normal, m.ior, m.clearcoat, radiance,
                            float3(clearcoat_brdf), dot(m.clearcoat_normal, V));
    luminance += radiance;
  }

  // [ ] emissive bloom, separate channel?
  return float4(luminance, 1);
}
