/// SPDX-License-Identifier: MIT
#pragma once

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
struct ComboBoxItem : public View
{
  bool            disabled : 1  = false;
  bool            hovered : 1   = false;
  bool            selected : 1  = false;
  Span<u32 const> text          = {};
  Vec4            text_color    = DEFAULT_THEME.on_surface;
  Vec4            hovered_color = DEFAULT_THEME.surface_variant;
  Frame           frame         = {};
  f32             alignment     = 0;
};

struct ComboBoxButton : public View
{
  // [ ] swiftui-style
};

/// [ ] z-index on expanded?
/// [ ] z-index effects on viewport
struct ComboBoxScrollView : public View
{
  bool                           disabled : 1 = false;
  Fn<void(u32, Span<u32 const>)> on_selected  = fn([](u32, Span<u32 const>) {});
  struct Inner
  {
    Vec<u32>          text  = {};
    Vec<u32>          runs  = {};
    Vec<ComboBoxItem> items = {};
  } inner = {};

  Frame frame        = {};
  f32   alignment    = 0;
  Vec4  corner_radii = {};
  Vec4  color        = DEFAULT_THEME.surface;

  virtual View *iter(u32 i) override
  {
    if (i >= inner.items.size32())
    {
      return nullptr;
    }
    return &inner.items[i];
  }

  void clear_items();
  void add_item();
};

struct ComboBox : public View
{
  ComboBoxScrollView scroll_view;
};

}        // namespace ash