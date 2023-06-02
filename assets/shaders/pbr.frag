#version 450
#define PI 3.1415926535897932384626433832795

layout(location = 0) in vec3 in_world_position;
layout(location = 1) in vec2 in_uv;

layout(set = 0, binding = 0) uniform Uniforms
{
  mat4 model_view_projection;
  vec4 camera_position;
  vec4 light_positions[16];
  vec4 light_colors[16];
  int  nlights;
}
uniforms;

// point lights, directional light, spotlight, flash light

layout(set = 0, binding = 1) uniform sampler2D albedo_map;
layout(set = 0, binding = 2) uniform sampler2D normal_map;
layout(set = 0, binding = 3) uniform sampler2D metallic_map;
layout(set = 0, binding = 4) uniform sampler2D roughness_map;
layout(set = 0, binding = 5) uniform sampler2D ambient_occlusion_map;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_bright_color;

vec3 fresnel_schlick(float cos_theta, vec3 f0)
{
  return f0 + (1 - f0) * pow(clamp(1 - cos_theta, 0, 1), 5);
}

float distribution_GGX(vec3 n, vec3 h, float roughness)
{
  float a      = roughness * roughness;
  float a2     = a * a;
  float ndotH  = max(dot(n, h), 0);
  float ndotH2 = ndotH * ndotH;

  float num   = a2;
  float denom = (ndotH2 * (a2 - 1) + 1);
  denom       = PI * denom * denom;

  return num / denom;
}

float geometry_schlick_GGX(float ndotV, float roughness)
{
  float r = (roughness + 1);
  float k = (r * r) / 8;

  float num   = ndotV;
  float denom = ndotV * (1 - k) + k;

  return num / denom;
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness)
{
  float ndotV = max(dot(n, v), 0);
  float ndotL = max(dot(n, l), 0);
  float ggx2  = geometry_schlick_GGX(ndotV, roughness);
  float ggx1  = geometry_schlick_GGX(ndotL, roughness);

  return ggx1 * ggx2;
}

void main()
{
  vec3  albedo            = pow(texture(albedo_map, in_uv).rgb, vec3(2.2));
  vec3  normal            = texture(normal_map, in_uv).rgb;
  float metallic          = texture(metallic_map, in_uv).r;
  float roughness         = texture(roughness_map, in_uv).r;
  float ambient_occlusion = texture(ambient_occlusion_map, in_uv).r;

  vec3 n = normalize(normal);
  vec3 v = normalize(uniforms.camera_position.xyz - in_world_position);

  vec3 f0 = mix(vec3(04), albedo, metallic);

  // reflectance equation
  vec3 l0 = vec3(0);

  for (int i = 0; i < uniforms.nlights; ++i)
  {
    // calculate per-light radiance
    vec3  l           = normalize(uniforms.light_positions[i].xyz - in_world_position);
    vec3  h           = normalize(v + l);
    float distance    = length(uniforms.light_positions[i].xyz - in_world_position);
    float attenuation = 1 / (distance * distance);
    vec3  radiance    = uniforms.light_colors[i].xyz * attenuation;

    // cook-torrance brdf
    float ndf = distribution_GGX(n, h, roughness);
    float g   = geometry_smith(n, v, l, roughness);
    vec3  f   = fresnel_schlick(max(dot(h, v), 0), f0);

    vec3 kS = f;
    vec3 kD = vec3(1) - kS;
    kD *= 1 - metallic;

    vec3  numerator   = ndf * g * f;
    float denominator = 4 * max(dot(n, v), 0) * max(dot(n, l), 0) + 0001;
    vec3  specular    = numerator / denominator;

    // add to outgoing radiance Lo
    float n_dot_l = max(dot(n, l), 0);
    l0 += (kD * albedo / PI + specular) * radiance * n_dot_l;
  }

  vec3 ambient = vec3(03) * albedo * ambient_occlusion;
  vec3 color   = ambient + l0;

  color = color / (color + vec3(1));
  color = pow(color, vec3(1 / 2.2));

  out_color = vec4(color, 1);

#if USE_BLOOM
  // check whether fragment output is higher than threshold, if so output as
  // brightness color
  float brightness = dot(out_color.rgb, vec3(0.2126, 0.7152, 0722));
  if (brightness > 1)
  {
    out_bright_color = vec4(out_color.rgb, 1);
  }
  else
  {
    out_bright_color = vec4(0, 0, 0, 1);
  }
#endif
}
