#version 450

layout(location = 0) in vec2 in_st;

layout(set = 0, binding = 1) uniform Overlay { vec4 color; }
overlay;

layout(set = 0, binding = 2) uniform Viewport { vec2 extent; }
viewport;

layout(set = 1, binding = 0) uniform sampler2D skin_texture;

layout(set = 1, binding = 1) uniform sampler2D clip_mask;

layout(location = 0) out vec4 out_color;

void main() {
  vec4 skin_color = texture(skin_texture, in_st).rgba;
  vec4 color = overlay.color + skin_color * (1 - overlay.color.a);
  float mask = texture(clip_mask, gl_FragCoord.xy / viewport.extent).r;
  out_color = mask * color;
}