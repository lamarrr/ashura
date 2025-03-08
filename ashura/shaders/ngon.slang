/// SPDX-License-Identifier: MIT

import "modules/core.slang";

struct Params
{
  row_major float4x4 transform;
  float4             tint[4];
  float4             uv;
  float              tiling;
  uint32_t           sampler;
  uint32_t           albedo;
  uint32_t           first_index;
  uint32_t           first_vertex;
};

[[vk::binding(0, 0)]]
StructuredBuffer<float2> vtx_buffer;

[[vk::binding(1, 0)]]
StructuredBuffer<uint32_t> idx_buffer;

[[vk::binding(2, 0)]]
StructuredBuffer<Params> params;

[[vk::binding(3, 0)]]
SamplerState samplers[];

[[vk::binding(4, 0)]]
Texture2D textures[];

[[vk::push_constant]]
float4x4 world_to_view;

struct VertexOutput
{
  nointerpolation uint32_t instance : INSTANCE_ID;
  float2                   uv : UV_COORD;
  float4                   position : SV_Position;
};

[[shader("vertex")]]
VertexOutput vs_main(uint32_t instance_idx : SV_InstanceID,
                     uint32_t vertex_idx : SV_VertexID)
{
  Params   p   = params[instance_idx];
  uint32_t idx = idx_buffer[p.first_index + vertex_idx];
  float2   pos = vtx_buffer[p.first_vertex + idx];
  return VertexOutput(
    instance_idx, pos + 0.5,
    mul(world_to_view, mul(p.transform, float4(pos, 0.0, 1.0))));
}

[[shader("fragment")]]
float4 main(nointerpolation uint32_t i_idx : INSTANCE_ID,
            float2                   i_uv : UV_COORD) :
  COLOR
{
  Params p      = params[i_idx];
  float2 tex_uv = lerp(p.uv.xy, p.uv.zw, i_uv);
  return bilerp(p.tint, i_uv) *
         textures[NonUniformResourceIndex(p.albedo)].Sample(
           samplers[NonUniformResourceIndex(p.sampler)], tex_uv * p.tiling);
}
