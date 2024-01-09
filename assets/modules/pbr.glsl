#version 450
#extension GL_ARB_shading_language_include : require
#define M_PI 3.1415926535897932384626433832795

// texture inputs defined by shader at compilation stage
// layout(set = 0, binding = 0) uniform sampler2D sampler;
// layout(set = 0, binding = 1) uniform texture2D base_color;
// layout(set = 0, binding = 2) uniform texture2D metallic;
// layout(set = 0, binding = 3) uniform texture2D roughness;
// layout(set = 0, binding = 4) uniform texture2D normal;
// layout(set = 0, binding = 5) uniform texture2D occlusion;

// GLTF-PBR,
// SEE:https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#complete-model
// - V is the normalized vector from the shading location to the eye
// - L is the normalized vector from the shading location to the light
// - N is the surface normal in the same space as the above values
// - H is the half vector, where H = normalize(L + V)
vec4 conductor_fresnel(vec4 base_color, vec4 bsdf, float VdotH)
{
  return bsdf * (base_color + (1 - base_color) * pow(1 - abs(VdotH), 5));
}

f32 specular_brdf(float alpha)
{
  return V * D;
}

f32 diffuse_brdf(Vec4 base_color)
{
  return (1 / M_PI) * base_color;
}

vec4 metal_brdf(vec4 base_color, float roughness)
{
  return conductor_fresnel(base_color, specular_brdf(roughness * roughness));
}

vec4 fresnel_mix(float index_of_refraction, vec4 base_color, vec4 layer,
                 float VdotH)
{
  base_color   = pow((1 - index_of_refraction) / (1 + index_of_refraction), 2);
  vec4 fresnel = base_color + (1 - base_color) * pow(1 - abs(VdotH), 5);
  return mix(base_color, layer, fresnel);
}

// ior: 1.5 default
vec4 dielectric_brdf(float index_of_refraction, vec4 base_color,
                     float roughness)
{
  return fresnel_mix(index_of_refraction, diffuse_brdf(base_color),
                     specular_brdf(roughness * roughness));
}

/// this is the base BRDF model
/// all extensions affect either of these 3 components, appending or prepending
/// or intercepting their values
vec4 brdf(vec4 dielectric_brdf, vec4 metal_brdf, vec4 metallic)
{
  return mix(dielectric_brdf, metal_brdf, metallic);
}

// material = brdf(...)
// clearcoat_brdf = specular_brdf(normal, r*r)
// coated_material = fresnel_coat(....)
vec4 fresnel_coat(vec3 clearcoat_normal, float index_of_refraction,
                  float coat_weight, vec4 base_material, vec4 layer)
{
  vec4 f0 = pow((1 - index_of_refraction) / (1 + index_of_refraction), 2);
  vec4 fr = f0 + (1 - f0) * pow(1 - abs(NdotV), 5);        // N = normal
  return mix(base_material, layer, coat_weight * fr);
}

vec4 pbr_ext(vec4 base_color, vec3 metallic, vec3 roughness, vec3 normal,
             vec3 occlusion, vec3 emissive, vec3 anisotropy, vec3 clearcoat,
             vec3 clearcoat_roughness, vec3 clearcoat_normal, vec3 iridescence,
             vec3 iridescence_thickness, vec3 sheen_color, vec3 sheen_roughness,
             vec3 specular, vec3 specular_color, vec3 transmission)
{
}
