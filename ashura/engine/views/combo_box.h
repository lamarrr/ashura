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
struct ComboBoxItem : public View
{
  bool          disabled : 1 = false;
  bool          hovered : 1  = false;
  bool          selected : 1 = false;
  RenderText    text         = {};
  ColorGradient text_color   = ColorGradient::all(DEFAULT_THEME.on_surface);
  ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);
  ColorGradient hovered_color =
      ColorGradient::all(DEFAULT_THEME.surface_variant);
  Vec4  border_radii = Vec4::splat(0.25F);
  Frame frame        = {};

  ComboBoxItem()                                = default;
  ComboBoxItem(ComboBoxItem const &)            = delete;
  ComboBoxItem &operator=(ComboBoxItem const &) = delete;
  ComboBoxItem(ComboBoxItem &&)
  {
  }
  ComboBoxItem &operator=(ComboBoxItem &&)
  {
  }
  ~ComboBoxItem()
  {
    text.reset();
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center = region.center,
                           .extent = region.extent,
                           .tint   = hovered ? hovered_color : color});
    text.render(region, clip, canvas);
  }
};

/// [ ] z-index on expanded?
/// [ ] z-index effects on viewport
struct ComboBoxScrollView : public View
{
  bool                           disabled : 1 = false;
  Fn<void(u32, Span<u32 const>)> on_selected  = fn([](u32, Span<u32 const>) {});
  struct Inner
  {
    Vec<ComboBoxItem> items = {};
  } inner = {};

  Frame         frame        = {};
  Vec4          border_radii = Vec4::splat(0.25F);
  f32           alignment    = 0;
  Vec4          corner_radii = {};
  ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);

  virtual View *iter(u32 i) override
  {
    if (i >= items.size32())
    {
      return nullptr;
    }
    return &items[i];
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    // [ ] scrolling
    // on scroll increase offset of views
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes)
  {
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    return Vec2{0, 0};
  }

  virtual CRect clip(CRect const &region, CRect const &allocated)
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
};

struct ComboBox : public View
{
  bool               disabled : 1 = false;
  bool               hovered : 1  = false;
  ComboBoxScrollView scroll_view;
  Frame              frame =
      Frame{.width = {.max = 200, .scale = 1}, .height = {.offset = 25}};
  Vec4          corner_radii = Vec4::splat(0.25F);
  ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);
  ColorGradient hovered_color =
      ColorGradient::all(DEFAULT_THEME.surface_variant);
  ColorGradient text_color = ColorGradient::all(DEFAULT_THEME.on_surface);
  ColorGradient inactive_text_color =
      ColorGradient::all(DEFAULT_THEME.inactive);

  virtual View *iter(u32 i) override
  {
    return subview({&scroll_view}, i);
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    if (ctx.mouse_down(MouseButtons::Primary))
    {
      if (scroll_view.is_open())
      {
        scroll_view.close();
      }
      else if (!disabled)
      {
        scroll_view.open();
      }
    }

    if (events.mouse_enter)
    {
      hovered = true;
    }

    if (events.mouse_leave)
    {
      hovered = false;
    }

    if (events.focus_in)
    {
    }

    return ViewState{.focusable = !disabled, .clickable = !disabled};
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii,
                           .tint         = hovered ? hovered_color : color});
    //[ ] draw selection text
    //[ ]  draw button: scroll_view.opened?
    // [ ] focus
  }
};

/*
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
enum class ComboBoxMode : u8
{
  Inline,
  Dialog
};

/// @param alignment alignment of the item, based on the text direction
struct ComboBoxItem : public View
{
  bool       disabled : 1        = false;
  bool       hovered : 1         = false;
  bool       selected : 1        = false;
  bool       focused : 1         = false;
  Fn<void()> on_selected         = fn([] {});
  RenderText text                = {};
  Vec4       hovered_color       = DEFAULT_THEME.surface_variant;
  Vec4       selected_color      = DEFAULT_THEME.surface_variant;
  Vec4       text_color          = DEFAULT_THEME.on_surface;
  Vec4       disabled_text_color = DEFAULT_THEME.inactive;
  Vec4       hovered_text_color  = DEFAULT_THEME.active;
  Frame      frame               = {};
  f32        alignment           = -1;

  ~ComboBoxItem()
  {
    text.reset();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    if (events.mouse_enter)
    {
      hovered = true;
    }

    if (events.mouse_leave)
    {
      hovered = false;
    }

    if (events.mouse_down && ctx.mouse_down(MouseButtons::Primary))
    {
      if (!selected)
      {
        selected = true;
        on_selected();
      }
    }

    if (events.key_down && ctx.key_down(KeyCode::Return))
    {
      if (!selected)
      {
        selected = true;
        on_selected();
      }
    }

    if (events.focus_in)
    {
      focused = true;
    }

    if (events.focus_out)
    {
      focused = false;
    }

    return ViewState{
        .pointable = !disabled, .clickable = !disabled, .focusable = !disabled};
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    text.render(region, clip, canvas);
  }
};

/// [ ] z-index effects on viewport
struct ComboBoxScrollView : public View
{
  typedef Fn<void(u32, Span<u32 const>)> Selected;

  bool     disabled : 1 = false;
  bool     hovered : 1  = false;
  bool     pressed : 1  = false;
  bool     expanded : 1 = false;
  Selected on_selected  = fn([](u32, Span<u32 const>) {});
  Frame    frame        = {};
  Vec4     corner_radii = Vec4::splat(0.125F);
  Vec4     color        = DEFAULT_THEME.surface;

  Vec<ComboBoxItem> items = {};

  ~ComboBoxScrollView()
  {
    items.reset();
  }

  virtual View *iter(u32 i) override
  {
    if (i >= items.size32())
    {
      return nullptr;
    }
    return &items[i];
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    // [ ] scrolling
    // on scroll increase offset of views
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes)
  {
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    return Vec2{0, 0};
  }

  virtual CRect clip(CRect const &region, CRect const &allocated)
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
};

// [ ] use mode to select state of the comboboxscroll view
struct ComboBox : public View
{
  typedef Fn<void(u32, Span<u32 const>)> Selected;

  bool     disabled : 1  = false;
  bool     hovered : 1   = false;
  bool     pressed : 1   = false;
  Selected on_selected   = fn([](u32, Span<u32 const>) {});
  Frame    frame         = {};
  Vec4     color         = DEFAULT_THEME.surface;
  Vec4     hovered_color = DEFAULT_THEME.surface_variant;
  Vec4     arrow_color   = DEFAULT_THEME.on_surface;
  Vec4     corner_radii  = Vec4::splat(0.125F);

  ComboBoxScrollView scroll_view;
};

}        // namespace ash

*/

}        // namespace ash