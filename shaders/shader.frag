#version 450

layout(location = 0) in vec2 in_st;

layout(push_constant) uniform CanvasPushConstants {
  mat4 transform;
  vec4 color;
}
push_constants;

layout(set = 0, binding = 0) uniform sampler2D skin_texture;

layout(location = 0) out vec4 out_color;

void main() {
  out_color = push_constants.color * texture(skin_texture, in_st).rgba;
}