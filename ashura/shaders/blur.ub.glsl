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

const vec2 VERTEX_BUFFER[] = {vec2(-1, -1), vec2(1, -1), vec2(1, 1),
                              vec2(-1, 1)};

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 o_pos;

void main()
{
  vec2 i_pos  = VERTEX_BUFFER[gl_VertexIndex];
  gl_Position = vec4(i_pos, 0, 1);
  o_pos       = (i_pos + 1) * 0.5;
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 i_pos;

layout(location = 0) out vec4 o_color;

void main()
{
  vec2 src_pos = mix(p.uv[0], p.uv[1], i_pos);
#  if UPSAMPLE
  o_color = upsample(samplers[p.isampler], textures[p.tex], src_pos, p.radius);
#  else
  o_color =
      downsample(samplers[p.isampler], textures[p.tex], src_pos, p.radius);
#  endif
}
#endif
