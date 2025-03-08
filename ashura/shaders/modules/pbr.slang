/// SPDX-License-Identifier: MIT

#pragma once

import "core.slang";

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
float3 conductor_fresnel(float3 albedo, float3 bsdf, float HoV)
{
  return bsdf * (albedo + (1 - albedo) * pow(1 - abs(HoV), 5));
}

float d_GGX(float roughness, float NoH, float3 N, float3 H)
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

float specular_brdf(float roughness, float NoL, float NoV, float NoH, float3 N,
                    float3 H)
{
  float V = v_SmithGGXCorrelatedFast(roughness, NoL, NoV);
  float D = d_GGX(roughness, NoH, N, H);
  return V * D;
}

float3 diffuse_brdf(float3 albedo)
{
  return (1 / PI) * albedo;
}

float3 metal_brdf(float3 albedo, float HoV, float roughness, float NoL,
                  float NoV, float NoH, float3 N, float3 H)
{
  return conductor_fresnel(
    albedo, float3(specular_brdf(roughness, NoL, NoV, NoH, N, H)), HoV);
}

float3 fresnel_mix(float3 albedo, float3 layer, float HoV, float ior)
{
  float f0 = pow((1 - ior) / (1 + ior), 2);
  float fr = f0 + (1 - f0) * pow(1 - abs(HoV), 5);
  return lerp(albedo, layer, fr);
}

// ior - index of refraction: 1.5 default
// roughness = perceptualRoughness * perceptualRoughness
float3 dielectric_brdf(float3 albedo, float roughness, float NoL, float NoV,
                       float NoH, float3 N, float3 H, float HoV, float ior)
{
  return fresnel_mix(diffuse_brdf(albedo),
                     float3(specular_brdf(roughness, NoL, NoV, NoH, N, H)), HoV,
                     ior);
}

/// apply fresnel coat to a brdf
/// material = brdf(dielectric, metal_brdf, metal_factor)
/// clearcoat_brdf = specular_brdf(normal, r*r)
/// coated_material = fresnel_coat(....)
float3 fresnel_coat(float3 clearcoat_normal, float ior, float coat_weight,
                    float3 base_material, float3 layer, float NoV)
{
  float f0 = pow((1 - ior) / (1 + ior), 2);
  float fr = f0 + (1 - f0) * pow(1 - abs(NoV), 5);    // N = normal
  return lerp(base_material, layer, coat_weight * fr);
}

/// this is the base BRDF model
/// all extensions affect either of these 3 components, appending or prepending
/// or intercepting their values
float3 brdf(float3 dielectric_brdf, float3 metal_brdf, float metallic)
{
  return lerp(dielectric_brdf, metal_brdf, metallic);
}
