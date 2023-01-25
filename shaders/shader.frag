#version 450

layout(location = 0) in vec2 in_st;
layout(location = 1) in vec4 in_color;

layout(set = 0, binding = 0) uniform sampler2D skin_texture;

layout(location = 0) out vec4 out_color;

void main() { out_color = in_color * texture(skin_texture, in_st).rgba; }