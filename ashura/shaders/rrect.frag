#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT 3

layout(location = 0) in vec2 i_pos;
layout(location = 1) in vec4 i_color;

layout(location = 0) out vec4 o_color;

layout(std140, set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  vec4          radii;
  vec4          uv;
  vec4          tint[4];
  vec2          aspect_ratio;
}
u_params;

layout(set = 1, binding = 0) uniform sampler2D u_albedo;

// https://iquilezles.org/articles/distfunctions/
// length(...+ border_radius) - border_radius -> gives the rounding of the
// border
float rrect_sdf(vec2 pos, vec2 half_extent, float border_radius)
{
  return length(max(abs(pos) - half_extent + border_radius, 0)) - border_radius;
}

void main()
{
  bool left = i_pos.x < 0;
  bool top  = i_pos.y < 0;
  uint corner =
      left ? (top ? TOP_LEFT : BOTTOM_LEFT) : (top ? TOP_RIGHT : BOTTOM_RIGHT);
  float radius      = u_params.radii[corner];
  vec2  half_extent = u_params.aspect_ratio;
  float dist        = rrect_sdf(i_pos, half_extent, radius);
  // 0.01 -> very small number to make the edge smooth, but not too small which
  // would lead to hard edges, larger values lead to softer edges
  float alpha = 1 - smoothstep(0, 0.015, dist);
  vec2  xy    = i_pos * 0.5 + 0.5;
  vec2  uv    = mix(u_params.uv.xy, u_params.uv.zw, xy);
  o_color     = mix(vec4(1, 1, 1, 0), i_color * texture(u_albedo, uv), alpha);
}
