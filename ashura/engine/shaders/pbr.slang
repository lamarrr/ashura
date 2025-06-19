/// SPDX-License-Identifier: MIT

#pragma once
#include "materials/custom.inl.slang"
#include "materials/pbr.slang"
#include "modules/core.slang"
#include "modules/types.slang"

struct World
{
  f32x4x4 world_transform;
  f32x4x4 world_to_ndc;
  f32x4   eye_position;
};

static constexpr u32 NUM_OBJECT_LIGHTS = 4;

struct Vertex
{
  f32 x;
  f32 y;
  f32 z;
  f32 u;
  f32 v;

  f32x4 pos()
  {
    return f32x4(x, y, z, 1);
  }

  f32x2 uv()
  {
    return f32x2(u, v);
  }
};

#ifndef VERTEX_TYPE

typedef Vertex VertexType;

#else

typedef VERTEX_TYPE VertexType;

#endif

#ifndef MATERIAL_TYPE

typedef pbr::BaseMaterial MaterialType;

#else

typedef MATERIAL_TYPE MaterialType;

#endif

#ifndef LIGHT_TYPE

typedef pbr::PunctualLight LightType;

#else

typedef LIGHT_TYPE LightType;

#endif

struct VertexOutput
{
  [[vk::location(0)]] f32x4               rel_pos : REL_POS;
  [[vk::location(1)]] f32x4               screen_pos : SV_Position;
  [[vk::location(2)]] f32x4               world_pos : WORLD_POS;
  [[vk::location(3)]] f32x2               uv : UV_COORD;
  [[vk::location(4)]] nointerpolation u32 instance : INSTANCE;
};

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
StructuredBuffer<VertexType> vtx_buffer;

[[vk::binding(0, 3)]]
StructuredBuffer<u32> idx_buffer;

[[vk::binding(0, 4)]]
ConstantBuffer<World> world;

[[vk::binding(0, 5)]]
ConstantBuffer<MaterialType> material;

[[vk::binding(0, 6)]]
StructuredBuffer<LightType> lights;

[[shader("vertex")]]
VertexOutput vert(u32 instance : SV_InstanceID, u32 vertex : SV_VertexID)
{
  var    idx        = idx_buffer[vertex];
  Vertex vtx        = vtx_buffer[idx];
  f32x4  rel_pos    = vtx.pos();
  f32x4  world_pos  = mul(world.world_transform, rel_pos);
  f32x4  screen_pos = mul(world.world_to_ndc, world_pos);
  f32x2  uv         = vtx.uv();
  return VertexOutput(rel_pos, screen_pos, world_pos, uv, instance);
}

[[shader("fragment")]]
FragmentOutput frag(VertexOutput in)
{
  var frag = pbr::FragmentInfo(world.eye_position, in.world_pos, in.screen_pos,
                               in.rel_pos);
  var m    = material.shade(frag, samplers, textures);

  f32x3 luminance = m.emission;

  f32x3 V   = normalize(world.eye_position.xyz - in.world_pos.xyz);
  f32   NoV = dot(m.normal, V.xyz);

  for (u32 i = 0; i < NUM_OBJECT_LIGHTS; i++)
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
