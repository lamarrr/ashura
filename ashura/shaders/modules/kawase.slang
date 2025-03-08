/// SPDX-License-Identifier: MIT

#pragma once

/// SIGGRAPH 2015 - Bandwidth-Efficient Rendering, Marius Bjorge, ARM
/// (https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_notes.pdf)
/// KAWASE multi-tap downsampling
float4 downsample(SamplerState smp, Texture2D src, float2 uv, float2 radius)
{
  float4 sum = src.Sample(smp, uv) * float4(4.0);
  sum += src.Sample(smp, uv + radius);
  sum += src.Sample(smp, uv - radius);
  sum += src.Sample(smp, uv + float2(radius.x, -radius.y));
  sum += src.Sample(smp, uv + float2(-radius.x, radius.y));
  return sum * (1 / 8.0);
}

float4 downsample_clamped(SamplerState smp, Texture2D src, float2 uv,
                          float2 radius, float2 min_uv, float2 max_uv)
{
  float4 sum = src.Sample(smp, clamp(uv, min_uv, max_uv)) * float4(4.0);
  sum += src.Sample(smp, clamp(uv + radius, min_uv, max_uv));
  sum += src.Sample(smp, clamp(uv - radius, min_uv, max_uv));
  sum +=
    src.Sample(smp, clamp(uv + float2(radius.x, -radius.y), min_uv, max_uv));
  sum +=
    src.Sample(smp, clamp(uv + float2(-radius.x, radius.y), min_uv, max_uv));
  return sum * (1 / 8.0);
}

/// SIGGRAPH 2015 - Bandwidth-Efficient Rendering, Marius Bjorge, ARM
/// KAWASE multi-tap upsampling
float4 upsample(SamplerState smp, Texture2D src, float2 uv, float2 radius)
{
  float4 sum = src.Sample(smp, uv + float2(-radius.x * 2, 0));
  sum += src.Sample(smp, uv + float2(-radius.x, radius.y)) * 2.0;
  sum += src.Sample(smp, uv + float2(0, radius.y * 2));
  sum += src.Sample(smp, uv + float2(radius.x, radius.y)) * 2.0;
  sum += src.Sample(smp, uv + float2(radius.x * 2, 0));
  sum += src.Sample(smp, uv + float2(radius.x, -radius.y)) * 2.0;
  sum += src.Sample(smp, uv + float2(0, -radius.y * 2));
  sum += src.Sample(smp, uv + float2(-radius.x, -radius.y)) * 2.0;
  return sum * (1 / 12.0);
}

float4 upsample_clamped(SamplerState smp, Texture2D src, float2 uv,
                        float2 radius, float2 min_uv, float2 max_uv)
{
  float4 sum =
    src.Sample(smp, clamp(uv + float2(-radius.x * 2, 0), min_uv, max_uv));
  sum +=
    src.Sample(smp, clamp(uv + float2(-radius.x, radius.y), min_uv, max_uv)) *
    2.0;
  sum += src.Sample(smp, clamp(uv + float2(0, radius.y * 2), min_uv, max_uv));
  sum +=
    src.Sample(smp, clamp(uv + float2(radius.x, radius.y), min_uv, max_uv)) *
    2.0;
  sum += src.Sample(smp, clamp(uv + float2(radius.x * 2, 0), min_uv, max_uv));
  sum +=
    src.Sample(smp, clamp(uv + float2(radius.x, -radius.y), min_uv, max_uv)) *
    2.0;
  sum += src.Sample(smp, clamp(uv + float2(0, -radius.y * 2), min_uv, max_uv));
  sum +=
    src.Sample(smp, clamp(uv + float2(-radius.x, -radius.y), min_uv, max_uv)) *
    2.0;
  return sum * (1 / 12.0);
}
