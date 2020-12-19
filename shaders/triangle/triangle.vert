#version 450

layout(location = 0) in vec2 iposition;
layout(location = 1) in vec3 icolor;

layout(location = 0) out vec4 ofragment_color;

// down is positive y

void main() {
  //
  gl_Position = vec4(iposition, 0.0, 1.0);
  ofragment_color = icolor;
}