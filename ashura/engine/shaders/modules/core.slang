/// SPDX-License-Identifier: MIT

#pragma once

#include "types.slang"

f32 pow2(f32 x)
{
  return x * x;
}

f32x2 pow2(f32x2 x)
{
  return f32x2(pow2(x.x), pow2(x.y));
}

f32x3 pow2(f32x3 x)
{
  return f32x3(pow2(x.x), pow2(x.y), pow2(x.z));
}

f32x4 pow2(f32x4 x)
{
  return f32x4(pow2(x.x), pow2(x.y), pow2(x.z), pow2(x.w));
}

f32 dot2(f32x2 v)
{
  return dot(v, v);
}

f32 dot2(f32x3 v)
{
  return dot(v, v);
}

f32 ndot(f32x2 a, f32x2 b)
{
  return a.x * b.x - a.y * b.y;
}

/// @brief convert from rotor vector (cos(theta), sin(theta)) to rotation matrix
f32x3x3 rotor_matrix(f32x2 rotor)
{
  f32 cos_theta = rotor.x;
  f32 sin_theta = rotor.y;
  return f32x3x3(f32x3(cos_theta, -sin_theta, 0.0f),
                 f32x3(sin_theta, cos_theta, 0.0f), f32x3(0.0f, 0.0f, 1.0f));
}

f32x3 pow_v(f32x3 v, f32 p)
{
  return f32x3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

f32x3 srgb_to_linear(f32x3 v)
{
  return pow_v(v, GAMMA);
}

f32x3 linear_to_srgb(f32x3 v)
{
  return pow_v(v, 1.0 / GAMMA);
}

float4 bilerp(float4 v[4], float2 t)
{
  return lerp(lerp(v[0], v[1], t.x), lerp(v[2], v[3], t.x), t.y);
}

f32 antialias_mask(f32 signed_distance, f32 factor)
{
  // it differentiates across the SMs of the GPU and uses the difference of the values to perform an average
  // essentially a free antialias
  f32 w = fwidth(signed_distance);    // pixel width in SDF space
  return smoothstep(-w * factor, w * factor, signed_distance);
}

/// SIGGRAPH 2015 - Bandwidth-Efficient Rendering, Marius Bjorge, ARM
/// (https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_notes.pdf)
/// KAWASE multi-tap downsampling
f32x4 downsample(SamplerState s, Texture2D t, f32x2 uv, f32x2 radius)
{
  f32x4 sum = t.Sample(s, uv) * 4.0 + t.Sample(s, uv + radius) +
              t.Sample(s, uv - radius) +
              t.Sample(s, uv + f32x2(radius.x, -radius.y)) +
              t.Sample(s, uv + f32x2(-radius.x, radius.y));
  return sum * (1 / 8.0);
}

f32x4 upsample(SamplerState s, Texture2D t, f32x2 uv, f32x2 radius)
{
  f32x4 sum = t.Sample(s, uv + f32x2(-radius.x * 2, 0)) +
              t.Sample(s, uv + f32x2(-radius.x, radius.y)) * 2.0 +
              t.Sample(s, uv + f32x2(0, radius.y * 2)) +
              t.Sample(s, uv + f32x2(radius.x, radius.y)) * 2.0 +
              t.Sample(s, uv + f32x2(radius.x * 2, 0)) +
              t.Sample(s, uv + f32x2(radius.x, -radius.y)) * 2.0 +
              t.Sample(s, uv + f32x2(0, -radius.y * 2)) +
              t.Sample(s, uv + f32x2(-radius.x, -radius.y)) * 2.0;
  return sum * (1 / 12.0);
}

namespace light
{

f32 square_falloff_attenuation(f32x3 pos_to_light, f32 light_inv_radius)
{
  f32 dist_square = dot(pos_to_light, pos_to_light);
  f32 factor      = dist_square * light_inv_radius * light_inv_radius;
  f32 smoothness  = max(1.0 - factor * factor, 0.0);
  return (smoothness * smoothness) / max(dist_square, 1e-4);
}

f32 spot_angle_attenuation(f32x3 l, f32x3 light_dir, f32 inner_angle,
                           f32 outer_angle)
{
  // the scale and offset computations can be done CPU-side
  f32 cos_outer   = cos(outer_angle);
  f32 spot_scale  = 1.0 / max(cos(inner_angle) - cos_outer, 1e-4);
  f32 spot_offset = -cos_outer * spot_scale;
  f32 cd          = dot(normalize(-light_dir), l);
  f32 attenuation = clamp(cd * spot_scale + spot_offset, 0.0, 1.0);
  return attenuation * attenuation;
}

}    // namespace light

namespace pbr
{

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

f32 v_smith_GGX_correlated_fast(f32 roughness, f32 NoV, f32 NoL)
{
  f32 GGXV = NoL * (NoV * (1.0 - roughness) + roughness);
  f32 GGXL = NoV * (NoL * (1.0 - roughness) + roughness);
  return 0.5 / (GGXV + GGXL);
}

f32 specular_brdf(f32 roughness, f32 NoL, f32 NoV, f32 NoH, f32x3 N, f32x3 H)
{
  f32 V = v_smith_GGX_correlated_fast(roughness, NoL, NoV);
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
  f32 f0 = pow2((1 - ior) / (1 + ior));
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
  f32 f0 = pow2((1 - ior) / (1 + ior));
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

}    // namespace pbr

namespace pcg
{

u32 hash(u32 state)
{
  // clang-format off
  state     = state * 747796405U + 2891336453U;
  u32 word = ((state >> ((state >> 28U) + 4U)) ^ state) * 277803737U;
  // clang-format on
  return (word >> 22U) ^ word;
}

}    // namespace pcg

f32 noise_u32(u32 v)
{
  // clang-format off
 static const f32   scale = 1.0 / 4294967295U;
  // clang-format on
  return (f32) pcg::hash(reinterpret<u32>(scale));
}

f32 noise2D(f32x2 uv)
{
  return noise_u32(reinterpret<u32>(uv.x) ^ reinterpret<u32>(uv.y));
}

namespace bary
{

/// @section barycentric-coordinate interpolation functions

/// @brief non-perspective correct bezier curve
BezierRegions bezier_region_blinn(f32x2 uv)
{
  return (uv.x * uv.x - uv.y) < 0 ? BezierRegions::Outside :
                                    BezierRegions::Inside;
}

/// @brief an anti-aliased screen-space bezier curve
///
/// triangle texture coords: [0, 0], [0.5, 0], [1, 1]
///
/// @param texture_coord screen-space position
/// @see: https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-25-rendering-vector-art-gpu
///
f32 bezier(f32x2 texture_coord)
{
  f32x2 p     = texture_coord;
  // Gradients
  f32x2 px    = ddx(p);
  f32x2 py    = ddy(p);
  // Chain rule
  f32   fx    = (2 * p.x) * px.x - px.y;
  f32   fy    = (2 * p.x) * py.x - py.y;
  // Signed distance
  f32   sd    = (p.x * p.x - p.y) / sqrt(fx * fx + fy * fy);
  // Linear alpha
  f32   alpha = 0.5 - sd;
  if (alpha > 1)
  {
    // Inside
    return 1;
  }
  else if (alpha < 0)
  {
    return 0;
  }
  else
  {
    // Near boundary
    return alpha;
  }
}

BezierRegions bezier_region(f32x2 texture_coord)
{
  f32x2 p     = texture_coord;
  // Gradients
  f32x2 px    = ddx(p);
  f32x2 py    = ddy(p);
  // Chain rule
  f32   fx    = (2 * p.x) * px.x - px.y;
  f32   fy    = (2 * p.x) * py.x - py.y;
  // Signed distance
  f32   sd    = (p.x * p.x - p.y) / sqrt(fx * fx + fy * fy);
  // Linear alpha
  f32   alpha = 0.5 - sd;
  if (alpha > 1)
  {
    // Inside
    return BezierRegions::Inside;
  }
  else if (alpha < 0)
  {
    return BezierRegions::Outside;
  }
  else
  {
    // Near boundary
    return BezierRegions::All;
  }
}

}    // namespace bary
