#version 450
#extension GL_GOOGLE_include_directive : require

#include "kawase.glsl"

layout(location = 0) in vec2 vert;
layout(location = 0) out vec2 o_vert;

void main()
{
  o_vert = vert;
  gl_Position = vec4(vert, 1, 1);
}
