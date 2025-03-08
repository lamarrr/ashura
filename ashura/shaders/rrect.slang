/// SPDX-License-Identifier: MIT

import "modules/core.slang";
import "modules/sdf.slang";

static constexpr int TOP_LEFT     = 0;
static constexpr int TOP_RIGHT    = 1;
static constexpr int BOTTOM_LEFT  = 2;
static constexpr int BOTTOM_RIGHT = 3;

struct Params
{
  row_major float4x4 transform;
  float4             tint[4];
  float4             radii;
  float4             uv;
  float              tiling;
  float              aspect_ratio;
  float              stroke;
  float              thickness;
  float              edge_smoothness;
  uint32_t           sampler;
  uint32_t           albedo;
};

[[vk::binding(0, 0)]]
StructuredBuffer<Params> params;

[[vk::binding(1, 0)]]
SamplerState samplers[];

[[vk::binding(2, 0)]]
Texture2D textures[];

[[vk::push_constant]]
row_major float4x4 world_to_view;

static constexpr float2 VERTEX_BUFFER[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

struct VertexOutput
{
  uint32_t instance : INSTANCE;
  float4   pos : SV_Position;
  float2   rel_pos : REL_POSITION;
};

[[shader("vertex")]] VertexOutput vs_main(uint32_t instance_idx : SV_InstanceID,
                                          uint32_t vertex_idx : SV_VertexID)
{
  Params p       = params[instance_idx];
  float2 rel_pos = VERTEX_BUFFER[vertex_idx];
  float4 pos = mul(world_to_view, mul(p.transform, float4(rel_pos, 0.0, 1.0)));
  return VertexOutput(instance_idx, pos, rel_pos);
}

[[shader("fragment")]] float4 fs_main(uint32_t instance : INSTANCE,
                                      float2   i_pos : REL_POSITION) :
  COLOR
{
  Params   p    = params[instance];
  bool     left = i_pos.x < 0;
  bool     top  = i_pos.y < 0;
  uint32_t corner =
    left ? (top ? TOP_LEFT : BOTTOM_LEFT) : (top ? TOP_RIGHT : BOTTOM_RIGHT);
  float        radius      = p.radii[corner];
  float2       pos         = i_pos * float2(p.aspect_ratio, 1);
  float2       half_extent = float2(p.aspect_ratio * 0.5, 0.5);
  float        dist        = rrect_sdf(pos, half_extent, radius);
  float2       uv          = i_pos + 0.5;
  float2       tex_uv      = lerp(p.uv.xy, p.uv.zw, uv);
  SamplerState sampler     = samplers[NonUniformResourceIndex(p.sampler)];
  float4       color       = textures[NonUniformResourceIndex(p.albedo)].Sample(
                   sampler, tex_uv * p.tiling) *
                 bilerp(p.tint, uv);
  float fill_alpha = 1 - smoothstep(0, p.edge_smoothness, dist);
  float stroke_alpha =
    1 - smoothstep(p.thickness, p.thickness + p.edge_smoothness,
                   abs(dist + p.thickness));
  float alpha = lerp(fill_alpha, stroke_alpha, p.stroke);
  return float4(color.rgb, color.a * alpha);
}
