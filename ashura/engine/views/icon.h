/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_text.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

struct Icon : View
{
  struct State
  {
    bool hidden : 1 = false;
  } state_;

  RenderText text_;

  Icon(Str32             text      = U""_str,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.icon_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Icon(Str8              text,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.icon_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Icon(Icon const &)             = delete;
  Icon(Icon &&)                  = default;
  Icon & operator=(Icon const &) = delete;
  Icon & operator=(Icon &&)      = default;
  virtual ~Icon() override       = default;

  Icon & hide(bool hide);

  Icon & icon(Str8 text, TextStyle const & style, FontStyle const & font);

  Icon & icon(Str32 text, TextStyle const & style, FontStyle const & font);

  ui::State tick(Ctx const & ctx, Events const & events,
                 Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;
};

}    // namespace ui

}    // namespace ash
