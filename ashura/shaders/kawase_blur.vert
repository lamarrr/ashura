#version 450
#extension GL_GOOGLE_include_directive : require

#include "kawase.glsl"

layout(location = 0) in vec2 in_vert;
layout(location = 0) out vec2 out_vert;

void main()
{
  out_vert    = in_vert;
  gl_Position = vec4(in_vert, 1, 1);
}
