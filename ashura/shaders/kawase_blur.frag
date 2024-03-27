#version 450
#extension GL_GOOGLE_include_directive : require

#include "kawase.glsl"

layout(location = 0) in vec2 in_vert;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler2D src;

layout(set = 1, binding = 0) uniform Params
{
  vec2 src_offset;
  vec2 src_extent;
  vec2 src_tex_extent;
  vec2 radius;
}
params;

void main()
{
#if UPSAMPLE
  out_color = upsample(src,
                       (params.src_offset + in_vert * params.src_extent) /
                           params.src_tex_extent,
                       params.radius / params.src_tex_extent);
#else
  out_color = downsample(src,
                         (params.src_offset + in_vert * params.src_extent) /
                             params.src_tex_extent,
                         params.radius / params.src_tex_extent);
#endif
}
