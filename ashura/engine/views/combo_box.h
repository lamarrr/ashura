/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_text.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

// [ ] scrolling
// [ ] selection
// [ ] clipping
// [ ] ios-style combo box
/// @param alignment alignment of the item, based on the text direction
struct ComboBoxItem : public View
{
  struct State
  {
    bool disabled : 1 = false;
    bool hovered : 1  = false;
    bool selected : 1 = false;
    bool focused : 1  = false;
  } state;

  struct Style
  {
    ColorGradient hovered_text_color = ColorGradient::all(DEFAULT_THEME.active);
    ColorGradient hovered_background_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient disabled_text_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient disabled_background_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient selected_background_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient selected_text_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient text_color = ColorGradient::all(DEFAULT_THEME.on_surface);
    ColorGradient background_color = ColorGradient::all(DEFAULT_THEME.surface);
    CornerRadii   corner_radii     = CornerRadii::all({.scale = 0.125F});
    Frame         frame            = {};
    f32           alignment        = -1;
  } style;

  Fn<void()> on_selected = fn([] {});

  RenderText text = {};

  ComboBoxItem()                                = default;
  ComboBoxItem(ComboBoxItem const &)            = delete;
  ComboBoxItem &operator=(ComboBoxItem const &) = delete;
  ComboBoxItem(ComboBoxItem &&);
  ComboBoxItem &operator=(ComboBoxItem &&);
  ~ComboBoxItem()
  {
    text.reset();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View *)>) override
  {
    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if (events.mouse_down && ctx.mouse_down(MouseButtons::Primary))
    {
      if (!state.selected)
      {
        state.selected = true;
        on_selected();
      }
    }

    if (events.key_down && ctx.key_down(KeyCode::Return))
    {
      if (!state.selected)
      {
        state.selected = true;
        on_selected();
      }
    }

    if (events.focus_in)
    {
      state.focused = true;
    }

    if (events.focus_out)
    {
      state.focused = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center = region.center,
                           .extent = region.extent,
                           .tint   = state.hovered ?
                                         style.hovered_background_color :
                                         style.background_color});
    text.render(region, clip, canvas);
  }
};

/// [ ] z-index on expanded?
/// [ ] z-index effects on viewport
struct ComboBoxScrollView : public View
{
  typedef Fn<void(u32, Span<u32 const>)> Selected;

  struct State
  {
    bool disabled : 1 = false;
    bool expanded : 1 = false;
  } state;

  struct Style
  {
    Frame         frame        = {};
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    f32           alignment    = 0;
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);
  } style;

  Vec<ComboBoxItem> items       = {};
  Selected          on_selected = fn([](u32, Span<u32 const>) {});

  ~ComboBoxScrollView()
  {
    items.reset();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View *)> build) override
  {
    // [ ] scrolling
    // on scroll increase offset of views
    for (ComboBoxItem &item : items)
    {
      build(&item);
    }
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    return Vec2{0, 0};
  }

  virtual CRect clip(CRect const &region, CRect const &allocated) override
  {
    return intersect(region, allocated);
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    // render rrect covering region
  }

  void clear_items();
  void add_item();
  bool is_opened();
  void close();
  void open();
  bool is_closed();
};

enum class ComboBoxMode : u8
{
  Inline = 0,
  Dialog = 1
};

struct ComboBox : public View
{
  typedef ComboBoxScrollView::Selected Selected;

  struct State
  {
    bool disabled : 1 = false;
    bool hovered : 1  = false;
    bool pressed : 1  = false;
  } state;

  ComboBoxScrollView scroll_view;

  struct Style
  {
    ComboBoxMode  mode         = ComboBoxMode::Inline;
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);
    ColorGradient hovered_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient text_color = ColorGradient::all(DEFAULT_THEME.on_surface);
    ColorGradient inactive_text_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    Frame frame =
        Frame{.width = {.scale = 1, .max = 200}, .height = {.offset = 25}};
  } style;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View *)> build) override
  {
    if (ctx.mouse_down(MouseButtons::Primary))
    {
      if (scroll_view.is_opened())
      {
        scroll_view.close();
      }
      else if (!state.disabled)
      {
        scroll_view.open();
      }
    }

    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if (events.focus_in)
    {
    }

    build(&scroll_view);

    return ViewState{.clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    canvas.rrect(
        ShapeDesc{.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent.y),
                  .tint = state.hovered ? style.hovered_color : style.color});
    //[ ] draw selection text
    //[ ]  draw button: scroll_view.opened?
    // [ ] focus
  }
};

}        // namespace ash
