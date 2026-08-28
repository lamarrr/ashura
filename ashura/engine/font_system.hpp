/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.hpp"
#include "ashura/engine/font.hpp"
#include "ashura/engine/text.hpp"
#include "ashura/std/allocator.hpp"
#include "ashura/std/dyn.hpp"
#include "ashura/std/result.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

typedef struct IFontSys * FontSys;

struct IFontSys
{
    static Dyn<FontSys> create(Allocator allocator, FileSys file_sys,
                               ImageSys image_sys);

    virtual ~IFontSys() = default;

    virtual AwaitFuturesVec init() = 0;

    virtual void shutdown() = 0;

    virtual TextLayout layout_text(TextBlock const & block, f32 max_width,
                                   f32 align_width) = 0;

    virtual Future<Result<FontId, SysErr>>
      load_from_memory(Str label, RcBlob8 encoded, u32 font_height, u32 face, Option<FontId> target_id) = 0;

    virtual Future<Result<FontId, SysErr>>
      load_from_path(Str label, Str path, u32 font_height, u32 face, Option<FontId> target_id) = 0;

    virtual FontInfo get(FontId id) = 0;

    virtual Option<FontInfo> get(Str label) = 0;

    virtual void unload(FontId id) = 0;
};

Span<u8 const> get_default_font_data();

}    // namespace ash
