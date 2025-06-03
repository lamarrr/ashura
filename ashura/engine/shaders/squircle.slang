/// SPDX-License-Identifier: MIT

#pragma once
#include "modules/core.slang"
#include "modules/sdf.slang"
#include "modules/types.slang"

static const int TOP_LEFT     = 0;
static const int TOP_RIGHT    = 1;
static const int BOTTOM_LEFT  = 2;
static const int BOTTOM_RIGHT = 3;

struct SquircleShaderParam
{
  row_major f32x4x4 transform;
  f32x4             tint[4];
  f32x4             uv;
  f32               degree;
  f32               tiling;
  f32               stroke;
  f32               thickness;
  f32               edge_smoothness;
  u32               sampler;
  u32               albedo;
};

struct ShaderConstants
{
  row_major f32x4x4 world_to_ndc;
  row_major f32x4x4 uv_transform;
};

static const f32x2 VERTEX_BUFFER[] = {
  {-0.5, -0.5},
  {0.5,  -0.5},
  {0.5,  0.5 },
  {-0.5, 0.5 }
};

struct VertexOutput
{
  f32x4                                   pos : SV_Position;
  [[vk::location(0)]] nointerpolation u32 instance : INSTANCE;
  [[vk::location(1)]] f32x2               local_pos : LOCAL_POSITION;
};

[[vk::push_constant]]
row_major ConstantBuffer<ShaderConstants, Std430DataLayout> constants;

[[vk::binding(0, 0)]]
row_major StructuredBuffer<SquircleShaderParam> params;

[[vk::binding(0, 1)]]
SamplerState samplers[];

[[vk::binding(0, 2)]]
Texture2D textures[];

[[shader("vertex")]] VertexOutput vs_main(u32 instance : SV_InstanceID,
                                          u32 vertex : SV_VertexID)
{
  var   p         = params[instance];
  f32x2 local_pos = VERTEX_BUFFER[vertex];
  f32x4 pos =
    mul(constants.world_to_ndc, mul(p.transform, f32x4(local_pos, 0.0, 1.0)));
  return VertexOutput(pos, instance, local_pos);
}

[[shader("fragment")]] f32x4
  fs_main([[vk::location(0)]] u32   instance : INSTANCE,
          [[vk::location(1)]] f32x2 local_pos : LOCAL_POSITION) :
  COLOR
{
  var          p      = params[instance];
  f32x2        pos    = local_pos;
  f32x3        sdf_g  = sdf::squircle(pos, 0.5F, p.degree);
  f32          dist   = sdf_g.x;
  f32x2        uv     = local_pos + 0.5;
  f32x2        tex_uv = lerp(p.uv.xy, p.uv.zw, uv);
  SamplerState s      = samplers[NonUniformResourceIndex(p.sampler)];
  f32x4        color =
    textures[NonUniformResourceIndex(p.albedo)].Sample(s, tex_uv * p.tiling) *
    bilerp(p.tint, uv);
  f32 fill_alpha = 1 - smoothstep(0, p.edge_smoothness, dist);
  f32 stroke_alpha =
    1 - smoothstep(p.thickness, p.thickness + p.edge_smoothness,
                   abs(dist + p.thickness));
  f32 alpha = lerp(fill_alpha, stroke_alpha, p.stroke);
  return f32x4(color.rgb, color.a * alpha);
}
