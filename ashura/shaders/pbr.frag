#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"
#include "pbr.glsl"

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_uv;

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  vec4          base_color_factor;
  float         metallic_factor;
  float         roughness_factor;
  float         normal_scale;
  float         occlusion_strength;
  vec4          emissive_factor;
  float         emissive_strength;
}
u_params;

layout(set = 1, binding = 0) uniform LightParams
{
  vec4             ambient_light;
  DirectionalLight directional_lights[MAX_PBR_DIRECTIONAL_LIGHTS];
  PointLight       point_lights[MAX_PBR_POINT_LIGHTS];
  SpotLight        spot_lights[MAX_PBR_SPOT_LIGHTS];
}
u_light_params;

layout(set = 2, binding = 0) uniform sampler2D u_albedo;
layout(set = 2, binding = 1) uniform sampler2D u_metallic;
layout(set = 2, binding = 2) uniform sampler2D u_roughness;
layout(set = 2, binding = 3) uniform sampler2D u_normal;
layout(set = 2, binding = 4) uniform sampler2D u_occlusion;
layout(set = 2, binding = 5) uniform sampler2D u_emissive;

layout(location = 0) out vec4 o_color;

void main()
{
  /*
  vec3  albedo            = pow(texture(u_albedo_map, i_uv).rgb, vec3(2.2));
  vec3  normal            = texture(u_normal_map, i_uv).rgb;
  float metallic          = texture(u_metallic_map, i_uv).r;
  float roughness         = texture(u_roughness_map, i_uv).r;
  float ambient_occlusion = texture(u_ambient_occlusion_map, i_uv).r;

  vec3 n = normalize(normal);
  vec3 v = normalize(u_params.camera_position.xyz - i_world_position);

  vec3 f0 = mix(vec3(04), albedo, metallic);

  // reflectance equation
  vec3 l0 = vec3(0);

  for (int i = 0; i < u_params.nlights; ++i)
  {
    // calculate per-light radiance
    vec3  l = normalize(u_params.light_positions[i].xyz - i_world_position);
    vec3  h = normalize(v + l);
    float distance = length(u_params.light_positions[i].xyz - i_world_position);
    float attenuation = 1 / (distance * distance);
    vec3  radiance    = u_params.light_colors[i].xyz * attenuation;

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

  color   = color / (color + vec3(1));
  color   = pow(color, vec3(1 / 2.2));
  o_color = vec4(color, 1);
  */
  o_color = vec4(1);

}
