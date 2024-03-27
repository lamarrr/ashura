#version 450
#extension GL_GOOGLE_include_directive : require

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
