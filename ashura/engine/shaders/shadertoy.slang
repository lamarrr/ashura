/// SPDX-License-Identifier: MIT

#pragma once
#include "modules/types.slang"

struct Inputs
{
  f32x3 Resolution;              // viewport resolution (in pixels)
  f32   Time;                    // shader playback time (in seconds)
  f32   TimeDelta;               // render time (in seconds)
  f32   FrameRate;               // shader frame rate
  i32   Frame;                   // shader playback frame
  f32   SampleRate;              // sound sample rate (i.e., 44100)
  f32x4 Date;                    // (year, month, day, time in seconds)
  f32   ChannelTime[4];          // channel playback time (in seconds)
  f32x3 ChannelResolution[4];    // channel resolution (in pixels)
  f32x4 Mouse;    // mouse pixel coords. xy: current (if MLB down), zw: click
};

struct VertexOutput
{
  f32x4                     pos : SV_Position;
  [[vk::location(0)]] f32x2 uv  : REL_POS;
};

[[vk::binding(0, 0)]]
ConstantBuffer<Inputs> in;

[[vk::binding(0, 1)]]
SamplerState Samplers[];

[[vk::binding(0, 2)]]
Texture2D Channels[];    // input channels. XX = 2D/Cube

static constexpr f32x2 VERTEX_BUFFER[] = {f32x2(-0.5, -0.5), f32x2(0.5, -0.5),
                                          f32x2(0.5, 0.5), f32x2(-0.5, 0.5)};

f32x4 main_image(f32x2 fragCoord);

[[shader("vertex")]]
VertexOutput vert(u32 vertex_idx : SV_VertexID)
{
  f32x2 local_pos = VERTEX_BUFFER[vertex_idx];
  return VertexOutput(f32x4(local_pos * 2, 0.0, 1.0), local_pos + 0.5);
}

[[shader("fragment")]]
f32x4 frag(VertexOutput in) :
  COLOR
{
  return main_image(in.uv);
}
