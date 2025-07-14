/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{
namespace ui
{

// [ ] replicate Checkbox and use font
struct Radio : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool hovered : 1 = false;

    bool value : 1 = false;
  } state_;

  struct Style
  {
    Frame frame = Frame{}.abs(20, 20);

    CornerRadii corner_radii = CornerRadii::all(0.5);

    f32 thickness = 0.5F;

    u8x4 color = theme.inactive;

    u8x4 inner_color = theme.primary;

    u8x4 inner_hovered_color = theme.primary_variant;
  } style_;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  Radio()                          = default;
  Radio(Radio const &)             = delete;
  Radio(Radio &&)                  = default;
  Radio & operator=(Radio const &) = delete;
  Radio & operator=(Radio &&)      = default;
  virtual ~Radio() override        = default;

  Radio & disable(bool disable);

  Radio & corner_radii(CornerRadii const & radii);

  Radio & thickness(f32 thickness);

  Radio & color(u8x4 color);

  Radio & inner_color(u8x4 color);

  Radio & inner_hovered_color(u8x4 color);

  Radio & frame(Frame frame);

  Radio & on_changed(Fn<void(bool)> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

}    // namespace ui

}    // namespace ash
