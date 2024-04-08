#ifndef KAWASE_GLSL
#define KAWASE_GLSL

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

#endif