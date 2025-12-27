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
  static Dyn<FontSys> create(Allocator allocator, FileSys file_sys,
                             ImageSys image_sys, Scheduler scheduler);

  virtual ~IFontSys() = default;

  virtual AwaitFuturesVec init() = 0;

  virtual void shutdown() = 0;

  virtual Dyn<TextLayoutBuffer> create_layout_buffer(Allocator allocator) = 0;

  virtual void layout_text(TextBlock const & block, f32 max_width,
                           TextLayout & layout, TextLayoutBuffer buffer,
                           Allocator scratch) = 0;

  virtual Future<Result<FontId, SysErr>>
    load_from_memory(Str label, RcBlob8 encoded, u32 font_height, u32 face) = 0;

  virtual Future<Result<FontId, SysErr>>
    load_from_path(Str label, Str path, u32 font_height, u32 face) = 0;

  virtual FontInfo get(FontId id) = 0;

  virtual Option<FontInfo> get(Str label) = 0;

  virtual void unload(FontId id) = 0;
};

Span<u8 const> get_default_font_data();

}    // namespace ash
