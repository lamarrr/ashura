/// SPDX-License-Identifier: MIT
#ifndef _CORE_GLSL_
#define _CORE_GLSL_

const float PI      = 3.1415926535897932384626433832795;
const float GAMMA   = 2.2;
const float EPSILON = 0.00000011920929;

vec3 pow_vec3(vec3 v, float p)
{
  return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 to_linear(vec3 v)
{
  return pow_vec3(v, GAMMA);
}

vec3 to_srgb(vec3 v)
{
  return pow_vec3(v, 1.0 / GAMMA);
}

vec4 bilerp(vec4 v[4], vec2 t)
{
  return mix(mix(v[0], v[1], t.x), mix(v[2], v[3], t.x), t.y);
}

/// SIGGRAPH 2015 - Bandwidth-Efficient Rendering, Marius Bjorge, ARM
/// (https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_notes.pdf)
/// KAWASE multi-tap downsampling
vec4 downsample(sampler smp, texture2D src, vec2 uv, vec2 radius)
{
  vec4 sum = texture(sampler2D(src, smp), uv) * vec4(4.0);
  sum += texture(sampler2D(src, smp), uv + radius);
  sum += texture(sampler2D(src, smp), uv - radius);
  sum += texture(sampler2D(src, smp), uv + vec2(radius.x, -radius.y));
  sum += texture(sampler2D(src, smp), uv + vec2(-radius.x, radius.y));
  return sum / 8.0;
}

/// SIGGRAPH 2015 - Bandwidth-Efficient Rendering, Marius Bjorge, ARM
/// KAWASE multi-tap upsampling
vec4 upsample(sampler smp, texture2D src, vec2 uv, vec2 radius)
{
  vec4 sum = texture(sampler2D(src, smp), uv + vec2(-radius.x * 2, 0));
  sum += texture(sampler2D(src, smp), uv + vec2(-radius.x, radius.y)) * 2.0;
  sum += texture(sampler2D(src, smp), uv + vec2(0, radius.y * 2));
  sum += texture(sampler2D(src, smp), uv + vec2(radius.x, radius.y)) * 2.0;
  sum += texture(sampler2D(src, smp), uv + vec2(radius.x * 2, 0));
  sum += texture(sampler2D(src, smp), uv + vec2(radius.x, -radius.y)) * 2.0;
  sum += texture(sampler2D(src, smp), uv + vec2(0, -radius.y * 2));
  sum += texture(sampler2D(src, smp), uv + vec2(-radius.x, -radius.y)) * 2.0;
  return sum / 12.0;
}

/// see: https://www.shadertoy.com/view/DlVcW1
float exponential_smoothmin(float a, float b, float k)
{
  k *= 1.0;
  float r = exp2(-a / k) + exp2(-b / k);
  return -k * log2(r);
}

float root_smoothmin(float a, float b, float k)
{
  k *= 2.0;
  float x = b - a;
  return 0.5 * (a + b - sqrt(x * x + k * k));
}

float sigmoid_smoothmin(float a, float b, float k)
{
  k *= log(2.0);
  float x = b - a;
  return a + x / (1.0 - exp2(x / k));
}

float quadratic_smoothmin(float a, float b, float k)
{
  k *= 4.0;
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * k * (1.0 / 4.0);
}

float cubic_smoothmin(float a, float b, float k)
{
  k *= 6.0;
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

float quartic_smoothmin(float a, float b, float k)
{
  k *= 16.0 / 3.0;
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * (4.0 - h) * k * (1.0 / 16.0);
}

float circular_smoothmin(float a, float b, float k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - k * 0.5 * (1.0 + h - sqrt(1.0 - h * (h - 2.0)));
}

float circular_geometrical_smoothmin(float a, float b, float k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  return max(k, min(a, b)) - length(max(k - vec2(a, b), 0.0));
}

// https://www.shadertoy.com/view/fsdyzB
// https://iquilezles.org/articles/distfunctions/
// https://iquilezles.org/articles/distfunctions2d/
// length(...+ corner_radii) - corner_radii -> gives the rounding of the
// corners
float rrect_sdf(vec2 pos, vec2 half_extent, float corner_radii)
{
  vec2 q = abs(pos) - half_extent + corner_radii;
  return min(max(q.x, q.y), 0) + length(max(q, 0)) - corner_radii;
}

// SDF functions: https://iquilezles.org/articles/distfunctions2d/

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

float square_falloff_attenuation(vec3 pos_to_light, float light_inv_radius)
{
  float dist_square = dot(pos_to_light, pos_to_light);
  float factor      = dist_square * light_inv_radius * light_inv_radius;
  float smoothness  = max(1.0 - factor * factor, 0.0);
  return (smoothness * smoothness) / max(dist_square, 1e-4);
}

float spot_angle_attenuation(vec3 l, vec3 light_dir, float inner_angle,
                             float outer_angle)
{
  // the scale and offset computations can be done CPU-side
  float cos_outer   = cos(outer_angle);
  float spot_scale  = 1.0 / max(cos(inner_angle) - cos_outer, 1e-4);
  float spot_offset = -cos_outer * spot_scale;
  float cd          = dot(normalize(-light_dir), l);
  float attenuation = clamp(cd * spot_scale + spot_offset, 0.0, 1.0);
  return attenuation * attenuation;
}

struct PunctualLight
{
  vec4  direction;        // xyz
  vec4  position;         // xyz
  vec4  color;
  float inner_angle;
  float outer_angle;
  float intensity;
  float radius;
};

#endif