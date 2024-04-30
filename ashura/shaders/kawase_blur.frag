#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 i_pos;

layout(set = 0, binding = 0) uniform Params
{
  vec2 offset;        // relative to the source texture
  vec2 extent;        // relative to the source texture
  vec2 radius;        // relative to the source texture
}
u_params;

layout(set = 1, binding = 0) uniform sampler2D src;

layout(location = 0) out vec4 o_color;

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
  vec2 src_pos = (u_params.offset + i_pos * u_params.extent);
#if UPSAMPLE
  o_color = kawase_upsample(src, src_pos, u_params.radius);
#else
  o_color = kawase_downsample(src, src_pos, u_params.radius);
#endif
}
