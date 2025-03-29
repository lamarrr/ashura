/// SPDX-License-Identifier: MIT

#pragma once

#include "core.slang"
#include "types.slang"

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
f32x3 conductor_fresnel(f32x3 albedo, f32x3 bsdf, f32 HoV)
{
  return bsdf * (albedo + (1 - albedo) * pow(1 - abs(HoV), 5));
}

f32 d_GGX(f32 roughness, f32 NoH, f32x3 N, f32x3 H)
{
  f32 a = NoH * roughness;
  f32 k = roughness / (1.0 - NoH * NoH + a * a);
  return k * k * (1.0 / PI);
}

f32 v_SmithGGXCorrelatedFast(f32 roughness, f32 NoV, f32 NoL)
{
  f32 GGXV = NoL * (NoV * (1.0 - roughness) + roughness);
  f32 GGXL = NoV * (NoL * (1.0 - roughness) + roughness);
  return 0.5 / (GGXV + GGXL);
}

f32 specular_brdf(f32 roughness, f32 NoL, f32 NoV, f32 NoH, f32x3 N, f32x3 H)
{
  f32 V = v_SmithGGXCorrelatedFast(roughness, NoL, NoV);
  f32 D = d_GGX(roughness, NoH, N, H);
  return V * D;
}

f32x3 diffuse_brdf(f32x3 albedo)
{
  return (1 / PI) * albedo;
}

f32x3 metal_brdf(f32x3 albedo, f32 HoV, f32 roughness, f32 NoL, f32 NoV,
                 f32 NoH, f32x3 N, f32x3 H)
{
  return conductor_fresnel(
    albedo, f32x3(specular_brdf(roughness, NoL, NoV, NoH, N, H)), HoV);
}

f32x3 fresnel_mix(f32x3 albedo, f32x3 layer, f32 HoV, f32 ior)
{
  f32 f0 = pow((1 - ior) / (1 + ior), 2);
  f32 fr = f0 + (1 - f0) * pow(1 - abs(HoV), 5);
  return lerp(albedo, layer, fr);
}

// ior - index of refraction: 1.5 default
// roughness = perceptualRoughness * perceptualRoughness
f32x3 dielectric_brdf(f32x3 albedo, f32 roughness, f32 NoL, f32 NoV, f32 NoH,
                      f32x3 N, f32x3 H, f32 HoV, f32 ior)
{
  return fresnel_mix(diffuse_brdf(albedo),
                     f32x3(specular_brdf(roughness, NoL, NoV, NoH, N, H)), HoV,
                     ior);
}

/// apply fresnel coat to a brdf
/// material = brdf(dielectric, metal_brdf, metal_factor)
/// clearcoat_brdf = specular_brdf(normal, r*r)
/// coated_material = fresnel_coat(....)
f32x3 fresnel_coat(f32x3 clearcoat_normal, f32 ior, f32 coat_weight,
                   f32x3 base_material, f32x3 layer, f32 NoV)
{
  f32 f0 = pow((1 - ior) / (1 + ior), 2);
  f32 fr = f0 + (1 - f0) * pow(1 - abs(NoV), 5);    // N = normal
  return lerp(base_material, layer, coat_weight * fr);
}

/// this is the base BRDF model
/// all extensions affect either of these 3 components, appending or prepending
/// or intercepting their values
f32x3 brdf(f32x3 dielectric_brdf, f32x3 metal_brdf, f32 metallic)
{
  return lerp(dielectric_brdf, metal_brdf, metallic);
}
