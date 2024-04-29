#ifndef PBR_GLSL
#define PBR_GLSL

#extension GL_GOOGLE_include_directive : require
#include "core.glsl"

#define MAX_PBR_AMBIENT_LIGHTS 1
#define MAX_PBR_DIRECTIONAL_LIGHTS 2
#define MAX_PBR_POINT_LIGHTS 2
#define MAX_PBR_SPOT_LIGHTS 2

// GLTF-PBR,
// SEE:https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#complete-model
// - V is the normalized vector from the shading location to the eye
// - L is the normalized vector from the shading location to the light
// - N is the surface normal in the same space as the above values
// - H is the half vector, where H = normalize(L + V)
// Also, SEE:
// https://github.com/google/filament/blob/main/shaders/src/brdf.fs#L136
// https://google.github.io/filament/Filament.html#materialsystem/specularbrdf
//
vec4 conductor_fresnel(vec4 albedo, vec4 bsdf, float HoV)
{
  return bsdf * (albedo + (1 - albedo) * pow(1 - abs(HoV), 5));
}

float d_GGX(float roughness, float NoH, const vec3 N, const vec3 H)
{
  float a = NoH * roughness;
  float k = roughness / (1.0 - NoH * NoH + a * a);
  return k * k * (1.0 / PI);
}

float v_SmithGGXCorrelatedFast(float roughness, float NoV, float NoL)
{
  float GGXV = NoL * (NoV * (1.0 - roughness) + roughness);
  float GGXL = NoV * (NoL * (1.0 - roughness) + roughness);
  return 0.5 / (GGXV + GGXL);
}

float specular_brdf(float roughness, float NoL, float NoV, float NoH, vec3 N,
                    vec3 H)
{
  float V = v_SmithGGXCorrelatedFast(roughness, NoL, NoV);
  float D = d_GGX(roughness, NoH, N, H);
  return V * D;
}

float diffuse_brdf(Vec4 albedo)
{
  return (1 / PI) * albedo;
}

vec4 metal_brdf(vec4 albedo, float roughness, float NoL, float NoV, float HoL,
                float HoV)
{
  return conductor_fresnel(albedo, specular_brdf(roughness, NoL, NoV, HoL, HoV),
                           HoV);
}

vec4 fresnel_mix(float ior, vec4 albedo, vec4 layer, float HoV)
{
  albedo       = pow((1 - ior) / (1 + ior), 2);
  vec4 fresnel = albedo + (1 - albedo) * pow(1 - abs(HoV), 5);
  return mix(albedo, layer, fresnel);
}

// ior - index of refraction: 1.5 default
// roughness = perceptualRoughness * perceptualRoughness
vec4 dielectric_brdf(float ior, vec4 albedo, float roughness, float HoV,
                     float NoL, float NoV, float NoH, vec3 N, vec3 H)
{
  return fresnel_mix(ior, diffuse_brdf(albedo),
                     specular_brdf(roughness, HoV, NoL, NoV, NoH, N, H), HoV);
}

/// this is the base BRDF model
/// all extensions affect either of these 3 components, appending or prepending
/// or intercepting their values
vec4 brdf(vec4 dielectric_brdf, vec4 metal_brdf, vec4 metallic)
{
  return mix(dielectric_brdf, metal_brdf, metallic);
}

/// apply fresnel coat to a brdf
/// material = brdf(dielectric, metal_brdf, metal_factor)
/// clearcoat_brdf = specular_brdf(normal, r*r)
/// coated_material = fresnel_coat(....)
vec4 fresnel_coat(vec3 clearcoat_normal, float ior, float coat_weight,
                  vec4 base_material, vec4 layer, float NoV)
{
  vec4 f0 = pow((1 - ior) / (1 + ior), 2);
  vec4 fr = f0 + (1 - f0) * pow(1 - abs(NoV), 5);        // N = normal
  return mix(base_material, layer, coat_weight * fr);
}

vec4 pbr_coated(float perceptualRoughness)
{
  // TODO(lamarrr): don't forget to convert roughness from perceptualroughness
  float roughness = perceptualRoughness * perceptualRoughness;
}

#endif