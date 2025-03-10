// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float        iTime;
float2       iResolution;
SamplerState sampler;
Texture2D    iChannel0;

float4 mainImage(float2 fragCoord)
{
  float2 q  = fragCoord.xy / iResolution.xy;
  float2 uv = 0.5 + (q - 0.5) * (0.9 + 0.1 * sin(0.2 * iTime));

  float3 oricol = iChannel0.Sample(sampler, float2(q.x, 1.0 - q.y)).xyz;
  float3 col;

  col.r = iChannel0.Sample(sampler, float2(uv.x + 0.003, -uv.y)).x;
  col.g = iChannel0.Sample(sampler, float2(uv.x + 0.000, -uv.y)).y;
  col.b = iChannel0.Sample(sampler, float2(uv.x - 0.003, -uv.y)).z;

  col = clamp(col * 0.5 + 0.5 * col * col * 1.2, 0.0, 1.0);

  col *= 0.5 + 0.5 * 16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);

  col *= float3(0.95, 1.05, 0.95);

  col *= 0.9 + 0.1 * sin(10.0 * iTime + uv.y * 1000.0);

  col *= 0.99 + 0.01 * sin(110.0 * iTime);

  float comp = smoothstep(0.2, 0.7, sin(iTime));
  col = lerp(col, oricol, clamp(-2.0 + 2.0 * q.x + 3.0 * comp, 0.0, 1.0));

  return float4(col, 1.0);
}
