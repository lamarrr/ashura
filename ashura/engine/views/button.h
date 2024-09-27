
/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/engine/views/text_view.h"
#include "ashura/std/types.h"

namespace ash
{

struct Button : public View
{
  bool          disabled : 1      = false;
  bool          pointer_down : 1  = false;
  bool          pointer_up : 1    = false;
  bool          pointer_enter : 1 = false;
  bool          pointer_leave : 1 = false;
  bool          focus_in : 1      = false;
  bool          focus_out : 1     = false;
  bool          focused : 1       = false;
  bool          hovered : 1       = false;
  bool          pressed : 1       = false;
  Fn<void()>    on_pressed        = fn([] {});
  Frame         frame             = {};
  Frame         padding           = {};
  ColorGradient color             = ColorGradient::all(DEFAULT_THEME.primary);
  ColorGradient hovered_color =
      ColorGradient::all(DEFAULT_THEME.primary_variant);
  ColorGradient disabled_color = ColorGradient::all(DEFAULT_THEME.inactive);
  Vec4          corner_radii   = Vec4::splat(0.125F);
  f32           stroke         = 0.0F;
  f32           thickness      = 1.0F;

  virtual View *iter(u32 i) override final
  {
    return subview({item()}, i);
  }

  virtual View *item()
  {
    return nullptr;
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events) override
  {
    pointer_down  = events.mouse_down && ctx.mouse_down(MouseButtons::Primary);
    pointer_up    = events.mouse_up && ctx.mouse_up(MouseButtons::Primary);
    pointer_enter = events.mouse_enter;
    pointer_leave = events.mouse_leave;
    focus_in      = events.focus_in;
    focus_out     = events.focus_out;

    if (events.focus_in)
    {
      focused = true;
    }

    if (events.focus_out)
    {
      focused = false;
    }

    if (pointer_enter)
    {
      hovered = true;
    }

    if (pointer_leave)
    {
      hovered = false;
    }

    if (pointer_down || (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      on_pressed();
      pressed = true;
    }

    if (pointer_up || (events.key_up && ctx.key_up(KeyCode::Return)))
    {
      pressed = false;
    }

    return ViewState{
        .pointable = !disabled, .clickable = !disabled, .focusable = !disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 size = allocated - 2 * padding(allocated);
    size.x    = max(size.x, 0.0F);
    size.y    = max(size.y, 0.0F);
    fill(sizes, size);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    fill(offsets, Vec2{0, 0});
    return (sizes.is_empty() ? Vec2{0, 0} : sizes[0]) + 2 * padding(allocated);
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    ColorGradient tint = (hovered && !pressed) ? hovered_color : color;
    tint               = disabled ? disabled_color : tint;
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii * region.extent.y,
                           .stroke       = stroke,
                           .thickness    = thickness,
                           .tint         = tint});
    if (focused)
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

  virtual View *item() override
  {
    return &text;
  }
};

}        // namespace ash