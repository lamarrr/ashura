#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#include "core.glsl"

struct Params
{
  ViewTransform transform;
  uint          albedo_map;
  uint          first_vertex;
};

struct Vertex
{
  vec4 pos;
  vec4 tint;
  vec2 uv;
};

layout(set = 0, binding = 0) readonly buffer VertexBuffer
{
  Vertex vtx_buffer[];
};

layout(set = 1, binding = 0) readonly buffer ParamsBuffer
{
  Params params[];
};

layout(set = 2, binding = 0) uniform sampler2D textures[];

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_tint;
layout(location = 2) flat out uint o_instance;

void main()
{
  Params p    = params[gl_InstanceIndex];
  Vertex vtx  = vtx_buffer[p.first_vertex + gl_VertexIndex];
  gl_Position = to_mvp(p.transform) * vtx.pos;
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
  o_color  = i_tint * texture(textures[nonuniformEXT(p.albedo_map)], i_uv);
}

#endif
