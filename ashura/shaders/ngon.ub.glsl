/// SPDX-License-Identifier: MIT
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#include "core.glsl"

struct Params
{
  mat4x4 transform;
  vec4   tint[4];
  vec4   uv;
  float  tiling;
  uint   isampler;
  uint   albedo;
  uint   first_index;
  uint   first_vertex;
};

layout(set = 0, binding = 0) readonly buffer VertexBuffer
{
  vec2 vtx_buffer[];
};

layout(set = 1, binding = 0) readonly buffer IndexBuffer
{
  uint idx_buffer[];
};

layout(set = 2, binding = 0, row_major) readonly buffer ParamsBuffer
{
  Params params[];
};

layout(set = 3, binding = 0) uniform sampler samplers[];

layout(set = 4, binding = 0) uniform texture2D textures[];

#ifdef VERTEX_SHADER

layout(location = 0) flat out uint o_idx;
layout(location = 1) out vec2 o_uv;

void main()
{
  Params p    = params[gl_InstanceIndex];
  uint   idx  = idx_buffer[p.first_index + gl_VertexIndex];
  vec2   pos  = vtx_buffer[p.first_vertex + idx];
  o_idx       = gl_InstanceIndex;
  o_uv        = (pos + 1.0) * 0.5;
  gl_Position = p.transform * vec4(pos, 0.0, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) flat in uint i_idx;
layout(location = 1) in vec2 i_uv;

layout(location = 0) out vec4 o_color;

void main()
{
  Params p      = params[i_idx];
  vec2   tex_uv = mix(p.uv.xy, p.uv.zw, i_uv);
  o_color       = bilerp(p.tint, i_uv) *
            texture(sampler2D(textures[nonuniformEXT(p.albedo)],
                              samplers[nonuniformEXT(p.isampler)]),
                    tex_uv * p.tiling);
}

#endif
