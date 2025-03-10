/// SPDX-License-Identifier: MIT

#pragma once
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

static constexpr int TOP_LEFT     = 0;
static constexpr int TOP_RIGHT    = 1;
static constexpr int BOTTOM_LEFT  = 2;
static constexpr int BOTTOM_RIGHT = 3;

struct RRectParam
{
  row_major f32x4x4 transform;
  f32x4             tint[4];
  f32x4             radii;
  f32x4             uv;
  f32               tiling;
  f32               aspect_ratio;
  f32               stroke;
  f32               thickness;
  f32               edge_smoothness;
  u16               sampler;
  u16               albedo;
};

[[vk::push_constant]]
row_major f32x4x4 world_to_view;

[[vk::binding(0, 0)]]
StructuredBuffer<RRectParam> params;

[[vk::binding(1, 0)]]
SamplerState samplers[];

[[vk::binding(2, 0)]]
Texture2D textures[];

static constexpr f32x2 VERTEX_BUFFER[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

struct VertexOutput
{
  u32   instance : INSTANCE;
  f32x2 local_pos : REL_POSITION;
  f32x4 pos : SV_Position;
};

[[shader("vertex")]] VertexOutput vs_main(u32 instance : SV_InstanceID,
                                          u32 vertex : SV_VertexID)
{
  var   p         = params[instance];
  f32x2 local_pos = VERTEX_BUFFER[vertex];
  f32x4 pos = mul(world_to_view, mul(p.transform, f32x4(local_pos, 0.0, 1.0)));
  return VertexOutput(instance, local_pos, pos);
}

[[shader("fragment")]] f32x4 fs_main(u32   instance : INSTANCE,
                                     f32x2 local_pos : REL_POSITION) :
  COLOR
{
  var  p    = params[instance];
  bool left = local_pos.x < 0;
  bool top  = local_pos.y < 0;
  var  corner =
    left ? (top ? TOP_LEFT : BOTTOM_LEFT) : (top ? TOP_RIGHT : BOTTOM_RIGHT);
  f32          radius      = p.radii[corner];
  f32x2        pos         = local_pos * f32x2(p.aspect_ratio, 1);
  f32x2        half_extent = f32x2(p.aspect_ratio * 0.5, 0.5);
  f32          dist        = rrect_sdf(pos, half_extent, radius);
  f32x2        uv          = local_pos + 0.5;
  f32x2        tex_uv      = lerp(p.uv.xy, p.uv.zw, uv);
  SamplerState s = samplers[NonUniformResourceIndex((uint) p.sampler)];
  f32x4 color    = textures[NonUniformResourceIndex((uint) p.albedo)].Sample(
                  s, tex_uv * p.tiling) *
                bilerp(p.tint, uv);
  f32 fill_alpha = 1 - smoothstep(0, p.edge_smoothness, dist);
  f32 stroke_alpha =
    1 - smoothstep(p.thickness, p.thickness + p.edge_smoothness,
                   abs(dist + p.thickness));
  f32 alpha = lerp(fill_alpha, stroke_alpha, p.stroke);
  return f32x4(color.rgb, color.a * alpha);
}
