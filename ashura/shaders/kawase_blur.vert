#version 450

layout(location = 0) in vec2 vert;

void main()
{
  gl_Position = vec4(vert, 1, 1);
}
