#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 i_vert;
layout(location = 0) out vec4 o_color;

layout(set = 0, binding = 0) uniform Params
{
  vec2 src_offset;
  vec2 src_extent;
  vec2 src_tex_extent;
  vec2 radius;
}
u_params;

layout(set = 1, binding = 0) uniform sampler2D src;

vec4 kawase_downsample(sampler2D src, vec2 uv, vec2 radius)
{
  vec4 sum = texture(src, uv) * vec4(4.0);
  sum += texture(src, uv + radius);
  sum += texture(src, uv - radius);
  sum += texture(src, uv + vec2(radius.x, -radius.y));
  sum += texture(src, uv + vec2(-radius.x, radius.y));
  return sum / 8.0;
}

vec4 kawase_upsample(sampler2D src, vec2 uv, vec2 radius)
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
  // todo(lamarrr): use mix/lerp instead
#if UPSAMPLE
  o_color =
      kawase_upsample(src,
                      (u_params.src_offset + i_vert * u_params.src_extent) /
                          u_params.src_tex_extent,
                      u_params.radius / u_params.src_tex_extent);
#else
  o_color =
      kawase_downsample(src,
                        (u_params.src_offset + i_vert * u_params.src_extent) /
                            u_params.src_tex_extent,
                        u_params.radius / u_params.src_tex_extent);
#endif
}
