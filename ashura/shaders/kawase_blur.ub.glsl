#extension GL_EXT_nonuniform_qualifier : require
struct Params
{
  vec2 offset;        // relative to the source texture
  vec2 extent;        // relative to the source texture
  vec2 radius;        // relative to the source texture
  uint src;
};

layout(push_constant) uniform ParamsPushConstant
{
  Params p;
};

layout(set = 0, binding = 0) uniform sampler2D textures[];

const uint INDEX_BUFFER[]  = {0, 1, 2, 2, 3, 0};
const vec2 VERTEX_BUFFER[] = {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)};

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

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 o_pos;

void main()
{
  uint vertex = INDEX_BUFFER[gl_VertexIndex];
  vec2 i_pos  = VERTEX_BUFFER[vertex];
  gl_Position = vec4(i_pos, 0, 1);
  o_pos       = i_pos;
}
#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 i_pos;

layout(location = 0) out vec4 o_color;

void main()
{
  vec2 src_pos = (p.offset + i_pos * p.extent);
#  if UPSAMPLE
  o_color = kawase_upsample(textures[p.src], src_pos, p.radius);
#  else
  o_color = kawase_downsample(textures[p.src], src_pos, p.radius);
#  endif
}
#endif
