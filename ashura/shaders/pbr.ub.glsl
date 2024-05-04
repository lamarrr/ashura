#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#include "core.glsl"
#include "light.glsl"
#include "pbr.glsl"

struct Params
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
  uint          albedo_map;
  uint          metallic_map;
  uint          roughness_map;
  uint          normal_map;
  uint          occlusion_map;
  uint          emissive_map;
  uint          clearcoat_map;
  uint          clearcoat_roughness_map;
  uint          clearcoat_normal_map;
  uint          first_light;
  uint          num_lights;
};

struct Vertex
{
  float x;
  float y;
  float z;
  float u;
  float v;
};

layout(set = 0, binding = 0) readonly buffer VtxBuffer
{
  Vertex data[];
}
vtx_buffers[];

layout(set = 1, binding = 0) readonly buffer IdxBuffer
{
  uint data[];
}
idx_buffers[];

layout(set = 2, binding = 0) readonly buffer ParamsBuffer
{
  Params params[];
};

layout(set = 2, binding = 1) readonly buffer Lights
{
  PunctualLight b_lights[];
};

layout(set = 3, binding = 0) uniform sampler2D u_tex[];

#ifdef VERTEX_SHADER

layout(location = 0) out vec3 o_world_pos;
layout(location = 1) out vec2 o_uv;
layout(location = 2) flat out uint o_instance;

void main()
{
  Params p = params[gl_InstanceIndex];
  uint idx = idx_buffers[nonuniformEXT(gl_InstanceIndex)].data[gl_VertexIndex];
  Vertex vtx       = vtx_buffers[nonuniformEXT(gl_InstanceIndex)].data[idx];
  vec3   i_pos     = vec3(vtx.x, vtx.y, vtx.z);
  vec2   i_uv      = vec2(vtx.u, vtx.v);
  vec4   world_pos = affine4(p.transform.model) * vec4(i_pos, 1);
  vec4   screen_pos =
      to_mat4(p.transform.projection) * affine4(p.transform.view) * world_pos;
  gl_Position = screen_pos;
  o_world_pos = world_pos.xyz;
  o_uv        = i_uv;
  o_instance  = gl_InstanceIndex;
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec3 i_world_pos;
layout(location = 1) in vec2 i_uv;
layout(location = 2) flat in uint i_instance;

layout(location = 0) out vec4 o_color;

void main()
{
  Params p = params[i_instance];
  vec3   albedo =
      p.albedo.xyz * texture(u_tex[nonuniformEXT(p.albedo_map)], i_uv).rgb;
  float metallic =
      p.metallic * texture(u_tex[nonuniformEXT(p.metallic_map)], i_uv).r;
  float roughness =
      p.roughness * texture(u_tex[nonuniformEXT(p.roughness_map)], i_uv).r;
  vec3  N = p.normal * texture(u_tex[nonuniformEXT(p.normal_map)], i_uv).rgb;
  float occlusion =
      p.occlusion * texture(u_tex[nonuniformEXT(p.occlusion_map)], i_uv).r;
  vec3 emissive =
      p.emissive.xyz * texture(u_tex[nonuniformEXT(p.emissive_map)], i_uv).rgb;
  float ior = p.ior;
  float clearcoat =
      p.clearcoat * texture(u_tex[nonuniformEXT(p.clearcoat_map)], i_uv).r;
  float clearcoat_roughness =
      p.clearcoat_roughness *
      texture(u_tex[nonuniformEXT(p.clearcoat_roughness_map)], i_uv).r;
  vec3 clearcoat_normal =
      p.clearcoat_normal *
      texture(u_tex[nonuniformEXT(p.clearcoat_normal_map)], i_uv).rgb;
  vec3  V   = normalize(p.view_position.xyz - i_world_pos);
  float NoV = dot(N, V);

  // convert from perceptual roughness to actual roughness
  roughness           = pow(roughness, 2);
  clearcoat_roughness = pow(clearcoat_roughness, 2);

  albedo *= occlusion;

  vec3 luminance = emissive;

  for (uint i = p.first_light; i < p.num_lights; i++)
  {
    // irradiance - light from source
    // radiance - reaction of the object to the light source
    PunctualLight light = b_lights[i];
    vec3          Lu    = light.position.xyz - i_world_pos;
    vec3          L     = normalize(Lu);
    vec3          H     = normalize(L + V);
    float         HoV   = dot(H, V);
    float         NoV   = dot(N, V);
    float         NoH   = dot(N, H);
    float         NoL   = clamp(dot(N, L), 0.0, 1.0);
    float         attenuation =
        get_square_falloff_attenuation(Lu, 1 / max(light.radius, EPSILON)) *
        get_spot_angle_attenuation(L, light.direction.xyz, light.inner_angle,
                                   light.outer_angle);
    vec3 irradiance   = light.intensity * attenuation * NoL * light.color.xyz;
    vec3 metal_brdf_v = metal_brdf(albedo, HoV, roughness, NoL, NoV, NoH, N, H);
    vec3 dielectric_v =
        dielectric_brdf(albedo, roughness, NoL, NoV, NoH, N, H, HoV, ior);
    vec3  radiance       = brdf(dielectric_v, metal_brdf_v, metallic);
    float clearcoat_brdf = specular_brdf(
        clearcoat_roughness, dot(clearcoat_normal, L), dot(clearcoat_normal, V),
        dot(clearcoat_normal, H), clearcoat_normal, H);
    radiance = fresnel_coat(clearcoat_normal, ior, clearcoat, radiance,
                            vec3(clearcoat_brdf), dot(clearcoat_normal, V));
    luminance += radiance;
  }

  // TODO(lamarrr): emissive bloom
  o_color = vec4(luminance, 1);
}
#endif