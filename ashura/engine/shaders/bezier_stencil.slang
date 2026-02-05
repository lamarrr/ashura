/// SPDX-License-Identifier: MIT

#pragma once
#include "items.slang"
#include "modules/core.slang"
#include "modules/sdf.slang"

struct VertexOutput
{
    f32x4                                   pos    : SV_Position;
    [[vk::location(0)]] f32x2               uv     : UV_COORD;
    [[vk::location(1)]] nointerpolation u32 region : FILL_REGIONS;
};

struct BezierStencilShaderParams
{
    f32x4x4             world_to_ndc;
    BezierStencilItem * items;
    f32x2 *             vertices;
    u32 *               indices;
};

[[vk::push_constant]]
ConstantBuffer<BezierStencilShaderParams> params;

static const f32x2 BARYCENTRIC_COORDS[3] = {
  {0.0, 0.0},
  {0.5, 0.0},
  {1.0, 1.0}
};

[[shader("vertex")]] VertexOutput vert(u32 instance : SV_VulkanInstanceID,
                                       u32 vertex : SV_VulkanVertexID)
{
    u32   idx        = params.indices[vertex];
    f32x2 rel_pos    = params.vertices[idx];
    f32x2 uv         = BARYCENTRIC_COORDS[vertex % 3];
    var   item       = params.items[instance];
    f32x4 world_pos  = mul(item.world_transform, f32x4(rel_pos, 0.0, 1.0));
    f32x4 screen_pos = mul(params.world_to_ndc, world_pos);
    var   region =
      item.first_bezier_index >= vertex ? BezierRegions::Inside : BezierRegions::All;
    return VertexOutput(screen_pos, uv, (u32) region);
}

[[shader("fragment")]] void frag(VertexOutput in)
{
    BezierRegions fill_type = (BezierRegions) in.region;
    BezierRegions regions   = bary::bezier_region(in.uv);
    var           fill      = regions & fill_type;

    if (fill == BezierRegions::None)
    {
        discard;
    }
}
