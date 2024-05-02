#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) out vec2 o_pos;

const uint INDEX_BUFFER[]  = {0, 1, 2, 2, 3, 0};
const vec2 VERTEX_BUFFER[] = {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)};

void main()
{
  uint vertex = INDEX_BUFFER[gl_VertexIndex];
  vec2 i_pos  = VERTEX_BUFFER[vertex];
  gl_Position = vec4(i_pos, 0, 1);
  o_pos       = i_pos;
}