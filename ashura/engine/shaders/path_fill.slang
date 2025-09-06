/// SPDX-License-Identifier: MIT
#include "items.slang"
#include "types.slang"

static const f32x2 VERTEX_BUFFER[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 3)]]
StructuredBuffer<PathPaintItem> items;

[[vk::binding(0, 4)]]
Texture2D<f32> alpha_masks;

[[vk::binding(0, 5)]]
Texture2D<u32> paint_ids;

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos : SV_Position;
  [[vk::location(1)]] f32x4               world_pos  : WORLD_POSITION;
  [[vk::location(2)]] f32x2               uv         : UV_POSITION;
  [[vk::location(3)]] nointerpolation u32 instance   : INSTANCE_IDX;
};

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  var   item       = items[instance];
  f32x2 pos        = VERTEX_BUFFER[vertex];
  f32x4 world_pos  = mul(item.world_transform, f32x4(pos, 0.0, 1.0));
  f32x4 screen_pos = mul(world_to_ndc, world_pos);
  return VertexOutput(screen_pos, world_pos, pos, instance);
}

[[shader("fragment")]]
f32x4 frag(VertexOutput in, f32x4 coord : SV_FragCoord)
{
  var   item       = items[in.instance];
  u32x2 pixel      = u32x2(coord.xy);
  f32   alpha_mask = f32(alpha_masks[pixel]);
  u32   paint_id   = paint_ids[pixel];

  if (item.paint_id != paint_id)
  {
    discard;
  }

  f32 alpha = smoothstep(item.feather_min, item.feather_max, alpha_mask);

  var info =
    QuadFragmentInfo(in.world_pos, in.screen_pos, in.uv, in.uv, in.instance);

  f32x4 color = item.material.shade(info, samplers, textures);

  color.w *= alpha;

  return color;
}
