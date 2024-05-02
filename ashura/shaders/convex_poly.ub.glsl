#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#include "core.glsl"

struct Params
{
  ViewTransform transform;
  uint          tex;
};

struct Vertex
{
  vec2 pos;
  vec2 uv;
  vec4 tint;
};

layout(set = 0, binding = 0) readonly buffer VertexBuffer
{
  Vertex data[];
}
vtx_buffers[];

layout(set = 0, binding = 1) readonly buffer IndexBuffer
{
  uint data[];
}
idx_buffers[];

layout(set = 0, binding = 2) readonly buffer Params
{
  Params params[];
};

layout(set = 0, binding = 3) uniform sampler2D u_tex[];

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_tint;
layout(location = 2) flat out uint o_instance;

void main()
{
  uint   idx  = idx_buffers[gl_InstanceIndex].data[gl_VertexIndex];
  Vertex vtx  = vtx_buffers[gl_InstanceIndex].data[idx];
  Params p    = params[gl_InstanceIndex];
  gl_Position = to_mvp(p.transform) * vec4(vtx.pos, 0, 1);
  o_uv        = vtx.uv;
  o_tint      = vtx.tint;
  o_instance  = gl_InstanceIndex;
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_tint;
layout(location = 2) flat in uint i_instance;

layout(location = 0) out vec4 o_color;

void main()
{
  Params p = params[i_instance];
  o_color  = i_tint * texture(u_tex[p.tex], i_uv);
}

#endif
