/// SPDX-License-Identifier: MIT
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "core.glsl"

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_LEFT 2
#define BOTTOM_RIGHT 3

struct Params
{
  mat4x4 transform;
  vec4   tint[4];
  vec4   radii;
  vec4   uv;
  float  tiling;
  float  aspect_ratio;
  float  stroke;
  float  thickness;
  float  edge_smoothness;
  uint   isampler;
  uint   albedo;
};

layout(set = 0, binding = 0, row_major) readonly buffer ParamBuffer
{
  Params params[];
};

layout(set = 1, binding = 0) uniform sampler samplers[];

layout(set = 2, binding = 0) uniform texture2D textures[];

const vec2 VERTEX_BUFFER[] = {vec2(-1, -1), vec2(1, -1), vec2(1, 1),
                              vec2(-1, 1)};

#ifdef VERTEX_SHADER

layout(location = 0) flat out uint o_idx;
layout(location = 1) out vec2 o_pos;

layout(push_constant, row_major) mat4x4 world_to_view;

void main()
{
  Params p    = params[gl_InstanceIndex];
  vec2   pos  = VERTEX_BUFFER[gl_VertexIndex];
  o_idx       = gl_InstanceIndex;
  o_pos       = pos;
  gl_Position = world_to_view * p.transform * vec4(pos, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) flat in uint i_idx;
layout(location = 1) in vec2 i_pos;

layout(location = 0) out vec4 o_color;

void main()
{
  Params p    = params[i_idx];
  bool   left = i_pos.x < 0;
  bool   top  = i_pos.y < 0;
  uint   corner =
      left ? (top ? TOP_LEFT : BOTTOM_LEFT) : (top ? TOP_RIGHT : BOTTOM_RIGHT);
  float radius      = p.radii[corner];
  vec2  pos         = i_pos * vec2(p.aspect_ratio, 1);
  vec2  half_extent = vec2(p.aspect_ratio, 1);
  float dist        = rrect_sdf(pos, half_extent, radius);
  vec2  uv          = (i_pos + 1.0) * 0.5;
  vec2  tex_uv      = mix(p.uv.xy, p.uv.zw, uv);
  vec4  color       = texture(sampler2D(textures[nonuniformEXT(p.albedo)],
                                        samplers[nonuniformEXT(p.isampler)]),
                              tex_uv * p.tiling) *
               bilerp(p.tint, uv);
  float fill_alpha = 1 - smoothstep(0, p.edge_smoothness, dist);
  float stroke_alpha =
      1 - smoothstep(p.thickness, p.thickness + p.edge_smoothness,
                     abs(dist + p.thickness));
  float alpha = mix(fill_alpha, stroke_alpha, p.stroke);
  o_color     = vec4(color.rgb, color.a * alpha);
}
#endif