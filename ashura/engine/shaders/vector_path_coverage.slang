/// SPDX-License-Identifier: MIT
#include "items.slang"
#include "modules/core.slang"
#include "modules/types.slang"

// https://github.com/rive-app/rive-runtime/blob/4d6675c0ff635c8b0038846397945da56cff2fe6/renderer/src/shaders/draw_clockwise_path.glsl#L1
// https://www.youtube.com/watch?v=64DDqMQRF9o&t=100s

struct VectorPathCoverageShaderParams
{
    VectorPathCfg            cfg;
    f32x4x4                  world_to_ndc;
    VectorPathCoverageItem * items;
    VectorPathVertex *       vertices;
    u32 *                    indices;
};

[[vk::push_constant]]
ConstantBuffer<VectorPathCoverageShaderParams> params;

[[vk::binding(0, 0)]]
// default: 0
RWBuffer<f32> alpha_masks[];

[[vk::binding(0, 1)]]
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
// To draw overlapping paths: clear the stencil buffer and then continue with
// previous attachments
//
//
[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_VulkanInstanceID, u32 vertex : SV_VulkanVertexID)
{
    u32   idx        = params.indices[vertex];
    var   vtx        = params.vertices[idx];
    var   item       = params.items[instance];
    f32x4 world_pos  = mul(item.world_transform, f32x4(vtx.position, 0.0, 1.0));
    f32x4 screen_pos = mul(params.world_to_ndc, world_pos);
    f32x4 output     = f32x4(screen_pos.xy, 0, 1.0);
    return VertexOutput(output, vtx.alpha_mask, vtx.fill_id);
}

[[shader("fragment")]]
void frag(VertexOutput in)
{
    // TODO: incorrect coord
    var pixel = u32x2(in.screen_pos.xy);
    u32 texel_index =
      tiled_flat_offset(pixel, params.cfg.tile_extent_log2, params.cfg.tile_texel_count,
                        params.cfg.tile_count);
    InterlockedExchange(alpha_masks[0][texel_index], in.alpha_mask);
    InterlockedExchange(fill_ids[0][texel_index], in.fill_id);
}
