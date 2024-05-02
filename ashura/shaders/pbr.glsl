#ifndef _PBR_GLSL_
#define _PBR_GLSL_

#extension GL_GOOGLE_include_directive : require
#include "core.glsl"

// GLTF-PBR,
// SEE:https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#complete-model
// - V is the normalized vector from the shading location to the eye
// - L is the normalized vector from the shading location to the light
// - N is the surface normal in the same space as the above values
// - H is the half vector, where H = normalize(L + V)
// Also, SEE:
// https://github.com/google/filament/blob/main/shaders/src/brdf.fs#L136
// https://google.github.io/filament/Filament.html#materialsystem/specularbrdf
// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/
//
vec3 conductor_fresnel(vec3 albedo, vec3 bsdf, float HoV)
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

vec3 diffuse_brdf(vec3 albedo)
{
  return (1 / PI) * albedo;
}

vec3 metal_brdf(vec3 albedo, float HoV, float roughness, float NoL, float NoV,
                float NoH, vec3 N, vec3 H)
{
  return conductor_fresnel(
      albedo, vec3(specular_brdf(roughness, NoL, NoV, NoH, N, H)), HoV);
}

vec3 fresnel_mix(vec3 albedo, vec3 layer, float HoV, float ior)
{
  float f0 = pow((1 - ior) / (1 + ior), 2);
  float fr = f0 + (1 - f0) * pow(1 - abs(HoV), 5);
  return mix(albedo, layer, fr);
}

// ior - index of refraction: 1.5 default
// roughness = perceptualRoughness * perceptualRoughness
vec3 dielectric_brdf(vec3 albedo, float roughness, float NoL, float NoV,
                     float NoH, vec3 N, vec3 H, float HoV, float ior)
{
  return fresnel_mix(diffuse_brdf(albedo),
                     vec3(specular_brdf(roughness, NoL, NoV, NoH, N, H)), HoV,
                     ior);
}

/// apply fresnel coat to a brdf
/// material = brdf(dielectric, metal_brdf, metal_factor)
/// clearcoat_brdf = specular_brdf(normal, r*r)
/// coated_material = fresnel_coat(....)
vec3 fresnel_coat(vec3 clearcoat_normal, float ior, float coat_weight,
                  vec3 base_material, vec3 layer, float NoV)
{
  float f0 = pow((1 - ior) / (1 + ior), 2);
  float fr = f0 + (1 - f0) * pow(1 - abs(NoV), 5);        // N = normal
  return mix(base_material, layer, coat_weight * fr);
}

/// this is the base BRDF model
/// all extensions affect either of these 3 components, appending or prepending
/// or intercepting their values
vec3 brdf(vec3 dielectric_brdf, vec3 metal_brdf, float metallic)
{
  return mix(dielectric_brdf, metal_brdf, metallic);
}

struct PBRParams
{
  ViewTransform transform;
  vec4          view_position;
  vec4          albedo;        // only xyz
  float         metallic;
  float         roughness;
  float         normal;
  float         occlusion;
  vec4          emissive;        // only xyz
  float         ior;             // 1.5 default
  float         clearcoat;
  float         clearcoat_roughness;
  float         clearcoat_normal;
};

#endif