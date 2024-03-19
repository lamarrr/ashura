#version 450

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

vec4 downsample(sampler2D src, vec2 uv, vec2 radius)
{
  vec4 sum = texture(src, uv) * vec4(4.0);
  sum += texture(src, uv + radius);
  sum += texture(src, uv - radius);
  sum += texture(src, uv + vec2(radius.x, -radius.y));
  sum += texture(src, uv + vec2(-radius.x, radius.y));
  return sum / 8.0;
}

vec4 upsample(sampler2D src, vec2 uv, vec2 radius)
{
  vec4 sum = texture(src, uv + vec2(-radius.x * 2, 0));
  sum += texture(src, uv + vec2(-radius.x, radius.y)) * 2.0;
  sum += texture(src, uv + vec2(0, radius.y * 2));
  sum += texture(src, uv + vec2(radius.x, radius.y)) * 2.0;
  sum += texture(src, uv + vec2(radius.x * 2, 0));
  sum += texture(src, uv + vec2(radius.x, -radius.y)) * 2.0;
  sum += texture(src, uv + vec2(0, -radius.y * 2));
  sum += texture(src, uv + vec2(-radius.x, -radius.y)) * 2.0;
  return sum / 12.0;
}

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
