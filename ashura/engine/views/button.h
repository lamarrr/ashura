
/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/engine/views/text_view.h"
#include "ashura/std/types.h"

namespace ash
{

struct Button : public View
{
  struct State
  {
    bool disabled : 1      = false;
    bool pointer_down : 1  = false;
    bool pointer_up : 1    = false;
    bool pointer_enter : 1 = false;
    bool pointer_leave : 1 = false;
    bool focus_in : 1      = false;
    bool focus_out : 1     = false;
    bool focused : 1       = false;
    bool hovered : 1       = false;
    bool pressed : 1       = false;
  } state;

  struct Style
  {
    ColorGradient color = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
    ColorGradient disabled_color = ColorGradient::all(DEFAULT_THEME.inactive);
    CornerRadii   corner_radii   = CornerRadii::all({.scale = 0.125F});
    f32           stroke         = 0.0F;
    f32           thickness      = 1.0F;
    Frame         frame          = {};
    Frame         padding        = {};
  } style;

  Fn<void()> on_pressed = fn([] {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View *)>) override
  {
    state.pointer_down =
        events.mouse_down && ctx.mouse_down(MouseButtons::Primary);
    state.pointer_up = events.mouse_up && ctx.mouse_up(MouseButtons::Primary);
    state.pointer_enter = events.mouse_enter;
    state.pointer_leave = events.mouse_leave;
    state.focus_in      = events.focus_in;
    state.focus_out     = events.focus_out;

    if (events.focus_in)
    {
      state.focused = true;
    }

    if (events.focus_out)
    {
      state.focused = false;
    }

    if (state.pointer_enter)
    {
      state.hovered = true;
    }

    if (state.pointer_leave)
    {
      state.hovered = false;
    }

    if (state.pointer_down ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      on_pressed();
      state.pressed = true;
    }

    if (state.pointer_up || (events.key_up && ctx.key_up(KeyCode::Return)))
    {
      state.pressed = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 size = allocated - 2 * style.padding(allocated);
    size.x    = max(size.x, 0.0F);
    size.y    = max(size.y, 0.0F);
    fill(sizes, size);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    fill(offsets, Vec2{0, 0});
    return (sizes.is_empty() ? Vec2{0, 0} : sizes[0]) +
           2 * style.padding(allocated);
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    ColorGradient tint =
        (state.hovered && !state.pressed) ? style.hovered_color : style.color;
    tint = state.disabled ? style.disabled_color : tint;
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .stroke       = style.stroke,
                           .thickness    = style.thickness,
                           .tint         = tint});
    if (state.focused)
    {
      // [ ] draw focus interaction for all widgets
    }
  }
};

struct TextButton : public Button
{
  TextView text{};

  TextButton()
  {
    text.copyable = false;
  }

  virtual ~TextButton() override = default;
};

}        // namespace ash