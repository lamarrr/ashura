/// SPDX-License-Identifier: MIT

#pragma once
#include "custom.inl.slang"
#include "items.slang"
#include "materials/pbr.slang"
#include "modules/core.slang"
#include "modules/types.slang"

struct VertexOutput
{
  [[vk::location(0)]] f32x4               screen_pos : SV_Position;
  [[vk::location(1)]] f32x4               world_pos  : WORLD_POS;
  [[vk::location(2)]] f32x2               uv         : UV_COORD;
  [[vk::location(3)]] nointerpolation u32 instance   : INSTANCE;
};

#ifndef ITEM_TYPE
#  define ITEM_TYPE CorePbrItem
#endif

struct FragmentOutput
{
  [[vk::location(0)]] f32x4 color;

#ifndef EXCLUDE_EMISSION
  [[vk::location(1)]] f32x4 emission;
#endif

  static FragmentOutput make(f32x4 color, f32x4 emission)
  {
    FragmentOutput f;

    f.color = color;

#ifndef EXCLUDE_EMISSION
    f.emission = emission;
#endif

    return f;
  }
};

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[vk::binding(0, 2)]]
StructuredBuffer<PbrVertex> vertices;

[[vk::binding(0, 3)]]
StructuredBuffer<u32> indices;

[[vk::binding(0, 4)]]
ConstantBuffer<ITEM_TYPE> item;

[[vk::binding(0, 5)]]
StructuredBuffer<pbr::PunctualLight> lights;

[[shader("vertex")]]
VertexOutput vert(u32 vertex : SV_VertexID, u32 instance : SV_InstanceID)
{
  var   idx        = indices[vertex];
  var   vtx        = vertices[idx];
  f32x4 rel_pos    = vtx.pos();
  f32x4 world_pos  = mul(item.world_transform, rel_pos);
  f32x4 screen_pos = mul(item.world_to_ndc, world_pos);
  f32x2 uv         = mul(item.uv_transform, f32x4(vtx.uv(), 0, 1)).xy;
  return VertexOutput(screen_pos, world_pos, uv, instance);
}

[[shader("fragment")]]
FragmentOutput frag(VertexOutput in)
{
  var frag = pbr::FragmentInfo(in.world_pos, in.screen_pos, in.uv, in.instance);
  var material = item.material;
  var m        = material.shade(frag, samplers, textures);

  f32x3 luminance = m.emission;

  f32x3 V   = normalize(item.eye_position.xyz - in.world_pos.xyz);
  f32   NoV = dot(m.normal, V.xyz);

  for (u32 i = item.first_light; i < (item.first_light + item.num_lights); i++)
  {
    // irradiance - light from source
    // radiance - reaction of the object to the light source
    var   light     = lights[i];
    f32x3 light_pos = light.position().xyz;
    f32x3 irradiance =
      m.lit * light.irradiance(f32x4(light_pos, 1), f32x4(m.normal, 1)).xyz;
    luminance += m.radiance(light_pos, in.world_pos.xyz, V, NoV, irradiance);
  }

  return FragmentOutput::make(f32x4(luminance, 1), f32x4(m.emission, 1));
}
