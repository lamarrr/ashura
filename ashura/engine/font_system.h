/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.h"
#include "ashura/engine/font.h"
#include "ashura/engine/text.h"
#include "ashura/std/allocator.h"
#include "ashura/std/dyn.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct IFontSys * FontSys;

struct IFontSys
{
  static Dyn<FontSys> create(Allocator allocator);

  virtual ~IFontSys() = default;

  virtual void shutdown() = 0;

  /// @brief rasterize the font at the specified font height. Note: raster is
  /// stored as alpha values.
  /// @note rasterizing mutates the font's internal data, not thread-safe
  /// @param font_height the font height at which the texture should be
  /// rasterized at (px)
  /// @param allocator scratch allocator to use for storing intermediates
  virtual Result<> rasterize(Font font, u32 font_height) = 0;

  virtual void layout_text(TextBlock const & block, f32 max_width,
                           TextLayout & layout) = 0;

  virtual Future<Result<FontId, FontLoadErr>>
    load_from_memory(Vec<char> label, Vec<u8> encoded, u32 font_height,
                     u32 face = 0) = 0;

  virtual Future<Result<FontId, FontLoadErr>> load_from_path(Vec<char> label,
                                                             Str       path,
                                                             u32 font_height,
                                                             u32 face = 0) = 0;

  virtual FontInfo get(FontId id) = 0;

  virtual Option<FontInfo> get(Str label) = 0;

  virtual void unload(FontId id) = 0;
};

}    // namespace ash
