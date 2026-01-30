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
#    define ITEM_TYPE CorePbrItem
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

struct PbrShaderParams
{
    DescriptorHandle<StructuredBuffer<PbrVertex>>     vertices;
    DescriptorHandle<StructuredBuffer<u32>>           indices;
    DescriptorHandle<StructuredBuffer<ITEM_TYPE>>     items;
    DescriptorHandle<StructuredBuffer<PunctualLight>> lights;
};

[[vk::push_constant]]
ConstantBuffer<PbrShaderParams> params;

[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D textures[];

[[shader("vertex")]]
VertexOutput vert(u32 vertex : SV_VulkanVertexID, u32 instance : SV_VulkanInstanceID)
{
    var   idx        = params.indices[vertex];
    var   vtx        = params.vertices[idx];
    var   p          = params.items[instance];
    f32x4 rel_pos    = vtx.pos();
    f32x4 world_pos  = mul(p.world_transform, rel_pos);
    f32x4 screen_pos = mul(p.world_to_ndc, world_pos);
    f32x2 uv         = mul(p.uv_transform, f32x4(vtx.uv(), 0.0, 1.0)).xy;
    return VertexOutput(screen_pos, world_pos, uv, instance);
}

[[shader("fragment")]]
FragmentOutput frag(VertexOutput in)
{
    var frag     = PbrFragmentInfo(in.world_pos, in.screen_pos, in.uv, in.instance);
    var p        = params.items[in.instance];
    var material = p.material;
    var m        = material.shade(frag, samplers, textures);

    f32x3 luminance = m.emission;

    f32x3 V   = normalize(p.eye_position.xyz - in.world_pos.xyz);
    f32   NoV = dot(m.normal, V.xyz);

    for (u32 i = p.first_light; i < (p.first_light + p.num_lights); i++)
    {
        // irradiance - light from source
        // radiance - reaction of the object to the light source
        var   light     = params.lights[i];
        f32x3 light_pos = light.position().xyz;
        f32x3 irradiance =
          m.lit * light.irradiance(f32x4(light_pos, 1), f32x4(m.normal, 1)).xyz;
        luminance += m.radiance(light_pos, in.world_pos.xyz, V, NoV, irradiance);
    }

    return FragmentOutput::make(f32x4(luminance, 1), f32x4(m.emission, 1));
}
