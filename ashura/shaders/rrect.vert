#version 450
#extension GL_GOOGLE_include_directive : require

layout(set = 0, binding = 0) uniform Params
{
  mat4  m;
  mat4  v;
  mat4  p;
  vec4  tint_tl;
  vec4  tint_tr;
  vec4  tint_bl;
  vec4  tint_br;
  vec4  border_color_tl;
  vec4  border_color_tr;
  vec4  border_color_bl;
  vec4  border_color_br;
  vec4  border_radii;
  float border_thickness;
  vec2  uv0;
  vec2  uv1;
}
params;

layout(set = 1, binding = 0) uniform sampler2D base_color;

void main()
{
}