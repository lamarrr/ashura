/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_text.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

// [ ] items
// [ ] placeholder
// [ ] item alignment
// [ ] scrolling
// [ ] selection
// [ ] clipping
// [ ] disabled color
struct ComboBoxItem : public View
{
  bool       disabled : 1  = false;
  bool       hovered : 1   = false;
  bool       selected : 1  = false;
  RenderText text          = {};
  Vec4       hovered_color = DEFAULT_THEME.surface_variant;
  Frame      frame         = {};
  f32        alignment     = -1;        // based on text direction
  // Vec4       text_color    = DEFAULT_THEME.on_surface;
};

/// [ ] z-index on expanded?
/// [ ] z-index effects on viewport
/// [ ] preview
struct ComboBoxScrollView : public View
{
  typedef Fn<void(u32, Span<u32 const>)> Selected;

  bool     disabled : 1 = false;
  bool     hovered : 1  = false;
  bool     pressed : 1  = false;
  Selected on_selected  = fn([](u32, Span<u32 const>) {});
  Frame    frame        = {};
  Vec4     corner_radii = Vec4::splat(0.125F);
  Vec4     color        = DEFAULT_THEME.surface;

  Vec<ComboBoxItem> items = {};

  virtual View *iter(u32 i) override
  {
    if (i >= items.size32())
    {
      return nullptr;
    }
    return &items[i];
  }

  void clear_items();
  void add_item();
};

struct ComboBox : public View
{
  typedef Fn<void(u32, Span<u32 const>)> Selected;

  bool     disabled : 1  = false;
  bool     hovered : 1   = false;
  bool     pressed : 1   = false;
  Selected on_selected   = fn([](u32, Span<u32 const>) {});
  Vec4     color         = DEFAULT_THEME.surface;
  Vec4     hovered_color = DEFAULT_THEME.surface_variant;
  Vec4     arrow_color   = DEFAULT_THEME.on_surface;
  Vec4     corner_radii  = Vec4::splat(0.125F);

  ComboBoxScrollView scroll_view;
};

}        // namespace ash