
#pragma once

#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

struct ViewportStateProxy {
  std::function<void()> on_resize = [] {};
  std::function<void()> on_scroll = [] {};
};

struct Viewport {
  friend struct ViewportSystemProxy;

  Viewport(Extent const extent, ViewOffset const offset,
           ViewExtent const widgets_allocation = ViewExtent{
               Constrain::relative(1.0f), Constrain::absolute(u32_max)}) {
    resize(extent, widgets_allocation);
    scroll_to(offset);
  }

  Extent get_extent() const { return extent_; }

  IOffset get_offset() const { return offset_; }

  Extent get_widgets_allocation() const { return widgets_allocation_; }

  void resize(Extent const extent,
              // give the widgets Extent{width of viewport, infinite height}
              ViewExtent const widgets_allocation = ViewExtent{
                  Constrain::relative(1.0f), Constrain::absolute(u32_max)}) {
    if (extent_ != extent) {
      extent_ = extent;
      is_resized_ = true;
    }

    Extent const new_widgets_allocation = widgets_allocation.resolve(extent);
    if (widgets_allocation_ != new_widgets_allocation) {
      widgets_allocation_ = new_widgets_allocation;
      is_resized_ = true;
    }
  }

  void scroll_to(ViewOffset const unresolved_offset) {
    // we need to resolve immediately as scroll offset depends on the extent
    // at that point in time
    IOffset const new_resolved_offset = unresolved_offset.resolve(extent_);
    if (offset_ != new_resolved_offset) {
      offset_ = new_resolved_offset;
      is_scrolled_ = true;
    }
  }

 private:
  void system_tick() {
    if (is_resized_) {
      state_proxy_.on_resize();
      is_resized_ = false;
    }

    if (is_scrolled_) {
      state_proxy_.on_scroll();
      is_scrolled_ = false;
    }
  }

  // updated due to a resize event
  Extent extent_;
  Extent widgets_allocation_;
  bool is_resized_ = true;

  // updated due to a scrolling event
  IOffset offset_;
  bool is_scrolled_ = true;

  ViewportStateProxy state_proxy_{};
};

struct ViewportSystemProxy {
  static void tick(Viewport& viewport) { viewport.system_tick(); }

  static ViewportStateProxy& get_state_proxy(Viewport& viewport) {
    return viewport.state_proxy_;
  }
};

}  // namespace ui
}  // namespace vlk
