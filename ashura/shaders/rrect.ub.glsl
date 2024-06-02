#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "core.glsl"

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_LEFT 2
#define BOTTOM_RIGHT 3

/// @param radius relative to height
struct Params
{
  vec4  transform[4];
  vec4  tint[4];
  vec4  radii;
  vec4  uv;
  float aspect_ratio;
  float stroke;
  float thickness;
  float edge_smoothness;
  uint  albedo;
};

layout(set = 0, binding = 0) readonly buffer ParamBuffer
{
  Params params[];
};

layout(set = 1, binding = 0) uniform sampler2D textures[];

const vec2 VERTEX_BUFFER[] = {vec2(-1, -1), vec2(1, -1), vec2(1, 1),
                              vec2(-1, 1)};

#ifdef VERTEX_SHADER

layout(location = 0) flat out uint o_idx;
layout(location = 1) out vec2 o_uv;

void main()
{
  Params p    = params[gl_InstanceIndex];
  vec2   pos  = VERTEX_BUFFER[gl_VertexIndex];
  vec2   uv   = (pos + 1.0) * 0.5;
  o_idx       = gl_InstanceIndex;
  o_uv        = uv;
  gl_Position = to_mat4(p.transform) * vec4(pos, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) flat in uint i_idx;
layout(location = 1) in vec2 i_uv;

layout(location = 0) out vec4 o_color;

void main()
{
  Params p    = params[i_idx];
  bool   left = i_uv.x < 0.5;
  bool   top  = i_uv.y < 0.5;
  uint   corner =
      left ? (top ? TOP_LEFT : BOTTOM_LEFT) : (top ? TOP_RIGHT : BOTTOM_RIGHT);
  float radius      = p.radii[corner];
  vec2  pos         = i_uv * vec2(p.aspect_ratio, 1);
  vec2  half_extent = vec2(p.aspect_ratio, 1) * 0.5;
  float dist        = rrect_sdf(pos, half_extent, radius);
  vec2  tex_uv      = mix(p.uv.xy, p.uv.zw, i_uv);
  vec4  color       = texture(textures[nonuniformEXT(p.albedo)], tex_uv) *
               bilerp(p.tint, i_uv, 0.5);
  float fill_alpha   = 1 - smoothstep(0, p.edge_smoothness, dist);
  float stroke_alpha = 1 - smoothstep(0, p.thickness, abs(dist));
  float alpha        = mix(stroke_alpha, fill_alpha, p.stroke);
  o_color            = vec4(color.rgb, color.a * alpha);
}
#endif