/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

/// @brief Multi-directional Slider
struct Slider : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool dragging : 1 = false;

    bool hovered : 1 = false;

    f32 t = 0;

    f32 low = 0;

    f32 high = 1;
  } state_;

  struct Style
  {
    Axis axis = Axis::X;

    Frame frame = Frame{}.abs(360, theme.body_font_height);

    f32 thumb_size = theme.body_font_height * 0.75F;

    f32 track_size = 4;

    u8x4 thumb_color = theme.primary;

    u8x4 thumb_hovered_color = theme.primary;

    u8x4 thumb_dragging_color = theme.primary_variant;

    CornerRadii thumb_corner_radii = CornerRadii::all(1'000);

    u8x4 track_color = theme.inactive;

    CornerRadii track_corner_radii = CornerRadii::all(2.5);

    f32 delta = 0.1F;
  } style_;

  struct Callbacks
  {
    Fn<void(f32)> changed = noop;
  } cb;

  Slider()                           = default;
  Slider(Slider const &)             = delete;
  Slider(Slider &&)                  = default;
  Slider & operator=(Slider const &) = delete;
  Slider & operator=(Slider &&)      = default;
  virtual ~Slider() override         = default;

  Slider & disable(bool disable);

  Slider & range(f32 low, f32 high);

  Slider & interp(f32 t);

  Slider & axis(Axis axis);

  Slider & frame(Frame frame);

  Slider & thumb_size(f32 size);

  Slider & track_size(f32 size);

  Slider & thumb_color(u8x4 color);

  Slider & thumb_hovered_color(u8x4 color);

  Slider & thumb_dragging_color(u8x4 color);

  Slider & thumb_corner_radii(CornerRadii const & color);

  Slider & track_color(u8x4 color);

  Slider & track_corner_radii(CornerRadii const & radii);

  Slider & on_changed(Fn<void(f32)> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

}    // namespace ui

}    // namespace ash
