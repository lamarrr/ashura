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

// [ ] scroll and clip text if region isn't large enough
// -  wrapping to the next line if not large enough
// -  no wrap
// -  max-len
// -  filter/transform function
// -  secret text input
struct InputCfg
{
  bool wrappable     : 1 = false;
  bool submittable   : 1 = false;
  bool multiline     : 1 = false;
  bool enter_submits : 1 = false;
  bool tab_input     : 1 = false;

  Fn<void(Vec<c32> &, Str32)> insert;
};

// [ ] renderer hooks for regions
struct Input : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool editing : 1 = false;

    bool submit : 1 = false;

    bool multiline : 1 = false;

    bool enter_submits : 1 = false;

    bool tab_input : 1 = false;
  } state_;

  struct Style
  {
    TextHighlightStyle highlight = {.color        = theme.highlight,
                                    .corner_radii = Vec4::splat(0)};
    CaretStyle         caret{.color = theme.caret, .thickness = 1.0F};
    usize              lines_per_page = 40;
    usize              tab_width      = 1;
  } style_;

  struct Callbacks
  {
    Fn<void()> edit = noop;

    Fn<void()> submit = noop;

    Fn<void()> focus_in = noop;

    Fn<void()> focus_out = noop;
  } cb;

  AllocatorRef allocator_;

  RenderText content_;

  RenderText stub_;

  TextCompositor compositor_;

  Input(Str32             stub      = U""_str,
        TextStyle const & style     = TextStyle{.color = theme.on_surface},
        FontStyle const & font      = FontStyle{.font   = theme.body_font,
                                                .height = theme.body_font_height,
                                                .line_height = theme.line_height},
        AllocatorRef      allocator = default_allocator);

  Input(Str8              stub,
        TextStyle const & style     = TextStyle{.color = theme.on_surface},
        FontStyle const & font      = FontStyle{.font   = theme.body_font,
                                                .height = theme.body_font_height,
                                                .line_height = theme.line_height},
        AllocatorRef      allocator = default_allocator);

  Input(Input const &)             = delete;
  Input(Input &&)                  = default;
  Input & operator=(Input const &) = delete;
  Input & operator=(Input &&)      = default;
  virtual ~Input() override        = default;

  Input & disable(bool disable);

  Input & multiline(bool enable);

  Input & enter_submits(bool enable);

  Input & tab_input(bool enable);

  Input & on_edit(Fn<void()> fn);

  Input & on_submit(Fn<void()> fn);

  Input & on_focus_in(Fn<void()> fn);

  Input & on_focus_out(Fn<void()> fn);

  Input & content(Str8 text);

  Input & content(Str32 text);

  Input & content_run(TextStyle const & style, FontStyle const & font,
                      usize first = 0, usize count = USIZE_MAX);

  Input & stub(Str8 text);

  Input & stub(Str32 text);

  Input & stub_run(TextStyle const & style, FontStyle const & font,
                   usize first = 0, usize count = USIZE_MAX);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

}    // namespace ui

}    // namespace ash
