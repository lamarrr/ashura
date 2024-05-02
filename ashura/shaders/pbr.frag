#version 450
#extension GL_GOOGLE_include_directive : require
#define NUM_LIGHTS 4
#include "core.glsl"
#include "light.glsl"
#include "pbr.glsl"

layout(location = 0) in vec3 i_pos;
layout(location = 1) out vec3 i_world_pos;
layout(location = 2) out vec2 i_uv;

layout(set = 0, binding = 0) uniform Params
{
  PBRParams p;
};

layout(set = 1, binding = 0) uniform Lights
{
  PunctualLight p_lights[NUM_LIGHTS];
};

layout(set = 2, binding = 0) uniform sampler2D u_albedo;
layout(set = 2, binding = 1) uniform sampler2D u_metallic;
layout(set = 2, binding = 2) uniform sampler2D u_roughness;
layout(set = 2, binding = 3) uniform sampler2D u_normal;
layout(set = 2, binding = 4) uniform sampler2D u_occlusion;
layout(set = 2, binding = 5) uniform sampler2D u_emissive;
layout(set = 2, binding = 6) uniform sampler2D u_clearcoat;
layout(set = 2, binding = 7) uniform sampler2D u_clearcoat_roughness;
layout(set = 2, binding = 8) uniform sampler2D u_clearcoat_normal;

layout(location = 0) out vec4 o_color;

void main()
{
  vec3  albedo    = p.albedo.xyz * texture(u_albedo, i_uv).rgb;
  float metallic  = p.metallic * texture(u_metallic, i_uv).r;
  float roughness = p.roughness * texture(u_roughness, i_uv).r;
  vec3  N         = p.normal * texture(u_normal, i_uv).rgb;
  float occlusion = p.occlusion * texture(u_occlusion, i_uv).r;
  vec3  emissive  = p.emissive.xyz * texture(u_emissive, i_uv).rgb;
  float ior       = p.ior;
  float clearcoat = p.clearcoat * texture(u_clearcoat, i_uv).r;
  float clearcoat_roughness =
      p.clearcoat_roughness * texture(u_clearcoat_roughness, i_uv).r;
  vec3 clearcoat_normal =
      p.clearcoat_normal * texture(u_clearcoat_normal, i_uv).rgb;
  vec3  V   = normalize(p.view_position.xyz - i_pos);
  float NoV = dot(N, V);

  // convert from perceptual roughness to actual roughness
  roughness           = pow(roughness, 2);
  clearcoat_roughness = pow(clearcoat_roughness, 2);

  albedo *= occlusion;

  vec3 luminance = vec3(0);

  for (uint i = 0; i < NUM_LIGHTS; i++)
  {
    // irradiance - light from source
    // radiance - reaction of the object to the light source
    PunctualLight light = p_lights[i];
    vec3          Lu    = light.position.xyz - i_world_pos;
    vec3          L     = normalize(Lu);
    vec3          H     = normalize(L + V);
    float         HoV   = dot(H, V);
    float         NoV   = dot(N, V);
    float         NoH   = dot(N, H);
    float         NoL   = clamp(dot(N, L), 0.0, 1.0);
    float         attenuation =
        get_square_falloff_attenuation(Lu, max(1 / light.radius, EPSILON)) *
        get_spot_angle_attenuation(L, light.direction.xyz, light.inner_angle,
                                   light.outer_angle);
    vec3 irradiance   = light.intensity * attenuation * NoL * light.color.xyz;
    vec3 metal_brdf_v = metal_brdf(albedo, HoV, roughness, NoL, NoV, NoH, N, H);
    vec3 dielectric_v =
        dielectric_brdf(albedo, roughness, NoL, NoV, NoH, N, H, HoV, ior);
    vec3  radiance       = brdf(dielectric_v, metal_brdf_v, metallic);
    float clearcoat_brdf = specular_brdf(
        clearcoat_roughness, dot(clearcoat_normal, L), dot(clearcoat_normal, V),
        dot(clearcoat_normal, H), clearcoat_normal, H);
    radiance = fresnel_coat(clearcoat_normal, ior, clearcoat, radiance,
                            vec3(clearcoat_brdf), dot(clearcoat_normal, V));
    luminance += radiance;
  }

  // TODO(lamarrr): emissive bloom
  o_color = vec4(luminance, 1);
}
