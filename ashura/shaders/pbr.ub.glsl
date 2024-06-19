#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#include "core.glsl"

layout(constant_id = 0) const uint NUM_OBJECT_LIGHTS = 4;

struct Params
{
  mat4x4 model;
  mat4x4 view;
  mat4x4 projection;
  vec4   eye_position;
  vec4   albedo;        // only xyz
  float  metallic;
  float  roughness;
  float  normal;
  float  occlusion;
  vec4   emissive;        // only xyz
  float  ior;
  float  clearcoat;
  float  clearcoat_roughness;
  float  clearcoat_normal;
  uint   isampler;
  uint   albedo_map;
  uint   metallic_map;
  uint   roughness_map;
  uint   normal_map;
  uint   occlusion_map;
  uint   emissive_map;
  uint   clearcoat_map;
  uint   clearcoat_roughness_map;
  uint   clearcoat_normal_map;
  uint   first_vertex;
  uint   first_light;
};

struct Vertex
{
  vec4 pos;
  vec2 uv;
};

layout(set = 0, binding = 0) readonly buffer VtxBuffer
{
  Vertex vtx_buffer[];
};

layout(set = 1, binding = 0) readonly buffer IdxBuffer
{
  uint idx_buffer[];
};

layout(set = 2, binding = 0, row_major) readonly buffer ParamsBuffer
{
  Params params[];
};

layout(set = 3, binding = 0) readonly buffer Lights
{
  PunctualLight lights[];
};

layout(set = 4, binding = 0) uniform sampler samplers[];

layout(set = 5, binding = 0) uniform texture2D textures[];

#ifdef VERTEX_SHADER

layout(location = 0) out vec4 o_world_pos;
layout(location = 1) out vec2 o_uv;
layout(location = 2) flat out uint o_idx;

void main()
{
  Params p         = params[gl_InstanceIndex];
  uint   idx       = idx_buffer[gl_VertexIndex];
  Vertex vtx       = vtx_buffer[p.first_vertex + idx];
  vec4   world_pos = p.model * vec4(vtx.pos.xyz, 1);
  vec4   pos       = p.projection * p.view * world_pos;
  o_world_pos      = world_pos;
  o_uv             = vtx.uv;
  o_idx            = gl_InstanceIndex;
  gl_Position      = pos;
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec4 i_world_pos;
layout(location = 1) in vec2 i_uv;
layout(location = 2) flat in uint i_idx;

layout(location = 0) out vec4 o_color;

void main()
{
  Params p      = params[i_idx];
  vec3   albedo = texture(sampler2D(textures[nonuniformEXT(p.albedo_map)],
                                    samplers[nonuniformEXT(p.isampler)]),
                          i_uv)
                    .rgb;
  float metallic = texture(sampler2D(textures[nonuniformEXT(p.metallic_map)],
                                     samplers[nonuniformEXT(p.isampler)]),
                           i_uv)
                       .r;
  float roughness = texture(sampler2D(textures[nonuniformEXT(p.roughness_map)],
                                      samplers[nonuniformEXT(p.isampler)]),
                            i_uv)
                        .r;
  vec3 N = texture(sampler2D(textures[nonuniformEXT(p.normal_map)],
                             samplers[nonuniformEXT(p.isampler)]),
                   i_uv)
               .rgb;
  float occlusion = texture(sampler2D(textures[nonuniformEXT(p.occlusion_map)],
                                      samplers[nonuniformEXT(p.isampler)]),
                            i_uv)
                        .r;
  vec3 emissive = texture(sampler2D(textures[nonuniformEXT(p.emissive_map)],
                                    samplers[nonuniformEXT(p.isampler)]),
                          i_uv)
                      .rgb;
  float clearcoat = texture(sampler2D(textures[nonuniformEXT(p.clearcoat_map)],
                                      samplers[nonuniformEXT(p.isampler)]),
                            i_uv)
                        .r;
  float clearcoat_roughness =
      texture(sampler2D(textures[nonuniformEXT(p.clearcoat_roughness_map)],
                        samplers[nonuniformEXT(p.isampler)]),
              i_uv)
          .r;
  vec3 clearcoat_normal =
      texture(sampler2D(textures[nonuniformEXT(p.clearcoat_normal_map)],
                        samplers[nonuniformEXT(p.isampler)]),
              i_uv)
          .rgb;

  albedo *= p.albedo.xyz;
  metallic *= p.metallic;
  roughness *= p.roughness;
  N *= p.normal;
  occlusion *= p.occlusion;
  emissive *= p.emissive.xyz;
  float ior = p.ior;
  clearcoat *= p.clearcoat;
  clearcoat_roughness *= p.clearcoat_roughness;
  clearcoat_normal *= p.clearcoat_normal;

  // convert from perceptual roughness to actual roughness
  roughness           = pow(roughness, 2);
  clearcoat_roughness = pow(clearcoat_roughness, 2);

  // TODO(lamarrr): is this correct?
  albedo *= occlusion;

  vec3 luminance = emissive;

  vec3  V   = normalize(p.eye_position.xyz - i_world_pos.xyz);
  float NoV = dot(N, V);

  for (uint i = 0; i < NUM_OBJECT_LIGHTS; i++)
  {
    // irradiance - light from source
    // radiance - reaction of the object to the light source
    PunctualLight light = lights[p.first_light + i];
    vec3          Lu    = light.position.xyz - i_world_pos.xyz;
    vec3          L     = normalize(Lu);
    vec3          H     = normalize(L + V);
    float         HoV   = dot(H, V);
    float         NoV   = dot(N, V);
    float         NoH   = dot(N, H);
    float         NoL   = clamp(dot(N, L), 0.0, 1.0);
    float         attenuation =
        square_falloff_attenuation(Lu, 1 / max(light.radius, EPSILON)) *
        spot_angle_attenuation(L, light.direction.xyz, light.inner_angle,
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