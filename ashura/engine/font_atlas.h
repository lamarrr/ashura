/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/font.h"
#include "ashura/engine/gpu_context.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/image.h"
#include "ashura/std/types.h"

namespace ash
{

struct CpuFontAtlas
{
  i32             font_height = 0;
  Vec2U           extent      = {};
  u32             num_layers  = 0;
  Vec<AtlasGlyph> glyphs      = {};
  Vec<u8>         channels    = {};

  ImageLayerSpan<u8, 1> span() const
  {
    return ImageLayerSpan<u8, 1>{.channels = channels,
                                 .width    = extent.x,
                                 .height   = extent.y,
                                 .layers   = num_layers};
  }
};

struct GpuFontAtlas
{
  gpu::Image          image       = nullptr;
  Vec<gpu::ImageView> views       = {};
  Vec<u32>            textures    = {};
  i32                 font_height = 0;
  u32                 num_layers  = 0;
  Vec2U               extent      = {};
  Vec<AtlasGlyph>     glyphs      = {};
  gpu::Format         format      = gpu::Format::Undefined;
};

}        // namespace ash
