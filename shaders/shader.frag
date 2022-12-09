#version 450

layout(location = 0) in vec2 in_st;

layout(push_constant) uniform PushConstants {
  mat4 transform;
  vec4 overlay;
}
push_constants;

layout(set = 1, binding = 0) uniform sampler2D skin_texture;

layout(location = 0) out vec4 out_color;

void main() {
  vec4 skin_color = texture(skin_texture, in_st).rgba;
  out_color = push_constants.overlay.a * push_constants.overlay +
              (1 - push_constants.overlay.a) * skin_color;
}