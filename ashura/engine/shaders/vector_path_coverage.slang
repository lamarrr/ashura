/// SPDX-License-Identifier: MIT
#include "items.slang"
#include "modules/core.slang"
#include "modules/types.slang"

// https://github.com/rive-app/rive-runtime/blob/4d6675c0ff635c8b0038846397945da56cff2fe6/renderer/src/shaders/draw_clockwise_path.glsl#L1
// https://www.youtube.com/watch?v=64DDqMQRF9o&t=100s
[[vk::push_constant]]
ConstantBuffer<VectorPathCfg> cfg;

[[vk::binding(0, 0)]]
ConstantBuffer<f32x4x4> world_to_ndc;

[[vk::binding(0, 1)]]
StructuredBuffer<VectorPathCoverageItem> items;

[[vk::binding(0, 2)]]
StructuredBuffer<VectorPathVertex> vertices;

[[vk::binding(0, 3)]]
StructuredBuffer<u32> indices;

[[vk::binding(0, 4)]]
// default: 0
RWBuffer<f32> alpha_masks[];

[[vk::binding(0, 5)]]
// default: 0
RWBuffer<u32> fill_ids[];

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos : SV_Position;
  [[vk::location(1)]] f32                 alpha_mask : ALPHA_MASK;
  [[vk::location(2)]] nointerpolation u32 fill_id    : FILL_ID;
};

// paint id 0: No painting
// paint id 1: Feather Joint
// paint id 2: Feather
// paint id 3: Material
// alpha mask; default = 0
// stencil buffer: winding_buffer; default = 0
//
// inner triangles will have alpha mask 1
// feather triangles will have alpha mask 0-1 interpolated
//
//
// To draw overlapping paths: clear the stencil buffer and then continue with previous attachments
//
//
[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  u32   idx        = indices[vertex];
  var   vtx        = vertices[idx];
  var   item       = items[instance];
  f32x4 world_pos  = mul(item.world_transform, f32x4(vtx.position, 0.0, 1.0));
  f32x4 screen_pos = mul(world_to_ndc, world_pos);
  f32   object     = vtx.fill_id >> 2;
  f32   type       = (vtx.fill_id & 3);
  f32   depth      = 0.001 * object + 0.0001 * type;
  f32x4 output     = f32x4(screen_pos.xy, depth, 1.0);
  return VertexOutput(output, vtx.alpha_mask, vtx.fill_id);
}

[[shader("fragment")]]
void frag(VertexOutput in)
{
  var pixel       = u32x2(in.screen_pos.xy);
  u32 texel_index = tiled_flat_offset(pixel, cfg.tile_extent_log2,
                                      cfg.tile_texel_count, cfg.tile_count);
  InterlockedExchange(alpha_masks[0][texel_index], in.alpha_mask);
  InterlockedExchange(fill_ids[0][texel_index], in.fill_id);
}
