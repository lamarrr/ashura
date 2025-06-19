/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{
namespace ui
{

struct Switch : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool hovered : 1 = false;

    bool value : 1 = false;
  } state_;

  struct Style
  {
    Vec4U8 on_color = theme.primary;

    Vec4U8 on_hovered_color = theme.primary_variant;

    Vec4U8 off_color = theme.active;

    Vec4U8 off_hovered_color = theme.inactive;

    Vec4U8 track_color = theme.surface_variant;

    f32 track_thickness = 1;

    f32 track_stroke = 0;

    CornerRadii corner_radii = CornerRadii::all(4);

    Frame frame = Frame{}.abs(40, 20);
  } style_;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  Switch()                           = default;
  Switch(Switch const &)             = delete;
  Switch(Switch &&)                  = default;
  Switch & operator=(Switch const &) = delete;
  Switch & operator=(Switch &&)      = default;
  virtual ~Switch() override         = default;

  Switch & disable(bool disable);

  Switch & on();

  Switch & off();

  Switch & toggle();

  Switch & on_color(Vec4U8 color);

  Switch & on_hovered_color(Vec4U8 color);

  Switch & off_color(Vec4U8 color);

  Switch & off_hovered_color(Vec4U8 color);

  Switch & track_color(Vec4U8 color);

  Switch & corner_radii(CornerRadii const & radii);

  Switch & frame(Frame frame);

  Switch & thumb_frame(Frame frame);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};
}    // namespace ui

}    // namespace ash
