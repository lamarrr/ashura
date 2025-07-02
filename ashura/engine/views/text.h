/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

struct Text : View
{
  struct State
  {
    bool copyable = false;
  } state_;

  struct Style
  {
    TextHighlightStyle highlight{.color        = theme.highlight,
                                 .corner_radii = Vec4::splat(0)};
  } style_;

  RenderText text_;

  TextCompositor compositor_;

  Text(Str32             text      = U""_str,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Text(Str8              text,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Text(Text const &)             = delete;
  Text(Text &&)                  = default;
  Text & operator=(Text const &) = delete;
  Text & operator=(Text &&)      = default;
  virtual ~Text() override       = default;

  Text & copyable(bool allow);

  Text & highlight_style(TextHighlightStyle highlight);

  Text & clear_highlights();

  Text & run(TextStyle const & style, FontStyle const & font, usize first = 0,
             usize count = USIZE_MAX);

  Text & text(Str32 text);

  Text & text(Str8 text);

  Str32 text() const;

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};
}    // namespace ui

}    // namespace ash
