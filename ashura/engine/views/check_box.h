/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/engine/views/icon.h"
#include "ashura/std/types.h"

namespace ash
{
namespace ui
{

// [ ] fix sizing
struct CheckBox : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool hovered : 1 = false;

    bool held : 1 = false;

    bool value : 1 = false;
  } state_;

  struct Style
  {
    u8x4 box_color = theme.inactive;

    u8x4 box_hovered_color = theme.active;

    f32 stroke = 1;

    f32 thickness = 0.5F;

    CornerRadii corner_radii = CornerRadii::all(5);

    // [ ] we need to find a way to resolve relative to something else, instead of the child
    Frame frame = Frame{}.rel(1, 1).abs(10, 10).rel_max(F32_INF, F32_INF);
  } style_;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  Icon icon_;

  CheckBox(Str32             text      = U"checkmark"_str,
           TextStyle const & style     = TextStyle{.color = theme.on_surface},
           FontStyle const & font      = FontStyle{.font   = theme.icon_font,
                                                   .height = theme.body_font_height,
                                                   .line_height = 1.0F},
           Allocator         allocator = default_allocator);

  CheckBox(Str8              text,
           TextStyle const & style     = TextStyle{.color = theme.on_surface},
           FontStyle const & font      = FontStyle{.font   = theme.icon_font,
                                                   .height = theme.body_font_height,
                                                   .line_height = 1.0F},
           Allocator         allocator = default_allocator);

  CheckBox(CheckBox const &)             = delete;
  CheckBox(CheckBox &&)                  = default;
  CheckBox & operator=(CheckBox const &) = delete;
  CheckBox & operator=(CheckBox &&)      = default;
  virtual ~CheckBox() override           = default;

  Icon & icon();

  CheckBox & disable(bool disable);

  CheckBox & box_color(u8x4 color);

  CheckBox & box_hovered_color(u8x4 color);

  CheckBox & stroke(f32 stroke);

  CheckBox & thickness(f32 thickness);

  CheckBox & corner_radii(CornerRadii const & radii);

  CheckBox & on_changed(Fn<void(bool)> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

}    // namespace ui

}    // namespace ash
