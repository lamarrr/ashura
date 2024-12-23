/// SPDX-License-Identifier: MIT
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "core.glsl"

/// @param radius relative to texture dimensions
struct Params
{
  vec2 uv[2];
  vec2 radius;
  uint isampler;
  uint tex;
};

layout(push_constant) uniform ParamBuffer
{
  Params p;
};

layout(set = 0, binding = 0) uniform sampler samplers[];

layout(set = 1, binding = 0) uniform texture2D textures[];

vec2 const VERTEX_BUFFER[] = {vec2(-0.5, -0.5), vec2(0.5, -0.5), vec2(0.5, 0.5),
                              vec2(-0.5, 0.5)};

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 o_uv;

void main()
{
  vec2 i_pos  = VERTEX_BUFFER[gl_VertexIndex];
  gl_Position = vec4(i_pos * 2, 0, 1);
  o_uv        = i_pos + 0.5;
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 i_uv;

layout(location = 0) out vec4 o_color;

void main()
{
  vec2 src_uv = mix(p.uv[0], p.uv[1], i_uv);
#  if UPSAMPLE
  o_color = upsample(samplers[p.isampler], textures[p.tex], src_uv, p.radius);
#  else
  o_color = downsample(samplers[p.isampler], textures[p.tex], src_uv, p.radius);
#  endif
}
#endif
