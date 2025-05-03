// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
#include "shadertoy.slang"

f32x4 mainImage(f32x2 fragCoord)
{
  f32x2 q  = fragCoord.xy / i.Resolution.xy;
  f32x2 uv = 0.5 + (q - 0.5) * (0.9 + 0.1 * sin(0.2 * i.Time));

  f32x3 oricol = i.Channel0.Sample(i.sampler, f32x2(q.x, 1.0 - q.y)).xyz;
  f32x3 col;

  col.r = i.Channel0.Sample(i.sampler, f32x2(uv.x + 0.003, -uv.y)).x;
  col.g = i.Channel0.Sample(i.sampler, f32x2(uv.x + 0.000, -uv.y)).y;
  col.b = i.Channel0.Sample(i.sampler, f32x2(uv.x - 0.003, -uv.y)).z;

  col = clamp(col * 0.5 + 0.5 * col * col * 1.2, 0.0, 1.0);

  col *= 0.5 + 0.5 * 16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y);

  col *= f32x3(0.95, 1.05, 0.95);

  col *= 0.9 + 0.1 * sin(10.0 * i.Time + uv.y * 1000.0);

  col *= 0.99 + 0.01 * sin(110.0 * i.Time);

  f32 comp = smoothstep(0.2, 0.7, sin(i.Time));
  col      = lerp(col, oricol, clamp(-2.0 + 2.0 * q.x + 3.0 * comp, 0.0, 1.0));

  return f32x4(col, 1.0);
}
