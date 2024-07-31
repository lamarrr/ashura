/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/font.h"
#include "ashura/engine/render_context.h"
#include "ashura/gfx/gfx.h"
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
    return ImageLayerSpan<u8, 1>{.channels = ::ash::span(channels),
                                 .width    = extent.x,
                                 .height   = extent.y,
                                 .layers   = num_layers};
  }

  void reset()
  {
    glyphs.reset();
    channels.reset();
  }
};

struct GpuFontAtlas
{
  gfx::Image          image       = nullptr;
  Vec<gfx::ImageView> views       = {};
  Vec<u32>            textures    = {};
  i32                 font_height = 0;
  u32                 num_layers  = 0;
  Vec2U               extent      = {};
  Vec<AtlasGlyph>     glyphs      = {};
  gfx::Format         format      = gfx::Format::Undefined;

  void reset()
  {
    image = nullptr;
    views.reset();
    textures.reset();
    glyphs.reset();
    format = gfx::Format::Undefined;
  }
};

}        // namespace ash
