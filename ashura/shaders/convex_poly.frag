#version 450

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_color;

layout(set = 0, binding = 0) uniform sampler2D skin_texture;

layout(location = 0) out vec4 o_color;

void main()
{
  o_color = i_color * texture(skin_texture, i_uv).rgba;
}