#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

#define TOP_LEFT 0
#define TOP_RIGHT 1
#define BOTTOM_RIGHT 2
#define BOTTOM_LEFT 3

layout(location = 0) in vec2 i_pos;

layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  float         radii[4];
  vec2          uv[2];
  vec4          tint;
  vec4          border_color;
  float         border_thickness;
  float         border_softness;
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
  uint edge =
      left ? (top ? TOP_LEFT : BOTTOM_LEFT) : (top ? TOP_RIGHT : BOTTOM_RIGHT);
  float radius       = u_params.radii[edge];
  float half_extent  = 1 - u_params.border_thickness;
  float dist         = rrect_sdf(i_pos, vec2(half_extent), radius);
  float alpha        = 1 - smoothstep(0.0, 1.0, dist);
  float border_alpha = 1 - smoothstep(0.0, u_params.border_softness, abs(dist));
  vec2  pos_rel      = (i_pos * 0.5) / half_extent + 0.5;
  vec2  uv           = mix(u_params.uv[0], u_params.uv[1], pos_rel);
  o_color            = alpha * texture(u_albedo, uv) * u_params.tint +
            border_alpha * u_params.border_color;
}
