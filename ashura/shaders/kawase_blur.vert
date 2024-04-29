#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 i_vert;
layout(location = 0) out vec2 o_vert;

void main()
{
  o_vert      = i_vert;
  gl_Position = vec4(i_vert, 1, 1);
}
