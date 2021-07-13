
#pragma once

#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

struct Viewport {
  friend struct ViewportSystemProxy;

  Viewport() = default;

  Viewport(Extent extent) { resize(extent, unresolved_widgets_allocation_); }

  Viewport(Extent extent, ViewExtent widgets_allocation) {
    resize(extent, widgets_allocation);
  }

  Viewport(Extent extent, ViewExtent widgets_allocation, ViewOffset offset) {
    resize(extent, widgets_allocation);
    scroll_to(offset);
  }

  Extent get_extent() const { return extent_; }

  IOffset get_offset() const { return offset_; }

  Extent get_widgets_allocation() const { return widgets_allocation_; }

  ViewExtent get_unresolved_widgets_allocation() const {
    return unresolved_widgets_allocation_;
  }

  bool is_resized() const { return is_resized_; }

  bool is_widgets_allocation_changed() const {
    return widgets_allocation_changed_;
  }

  bool is_scrolled() const { return is_scrolled_; }

  void resize(Extent extent, ViewExtent widgets_allocation) {
    VLK_ENSURE(extent.visible());

    if (extent_ != extent) {
      extent_ = extent;
      is_resized_ = true;
    }

    // we need to update the resolved scroll offset, as the new extent can be
    // smaller than the previous one, at which point the previous resolved
    // scroll offset would be invalidated
    scroll_to(unresolved_offset_);

    // we update this irregardless, as different values of this could resolve to
    // same extent

    Extent new_widgets_allocation = widgets_allocation.resolve(extent);
    if (widgets_allocation_ != new_widgets_allocation) {
      widgets_allocation_ = new_widgets_allocation;
      widgets_allocation_changed_ = true;
    }
  }

  void scroll_to(ViewOffset unresolved_offset) {
    // we need to resolve immediately as scroll offset depends on the extent
    // at that point in time
    unresolved_offset_ = unresolved_offset;
    IOffset new_resolved_offset = unresolved_offset.resolve(extent_);
    if (offset_ != new_resolved_offset) {
      offset_ = new_resolved_offset;
      is_scrolled_ = true;
    }
  }

 private:
  // updated due to a resize event
  Extent extent_ {1920, 1080};
  bool is_resized_ = true;

  Extent widgets_allocation_;
  bool widgets_allocation_changed_ = true;
  // give the widgets Extent{width of viewport, infinite height}
  ViewExtent unresolved_widgets_allocation_ =
      ViewExtent{Constrain::relative(1.0f), Constrain::absolute(u32_max)};

  // updated due to a scrolling event
  IOffset offset_;
  ViewOffset unresolved_offset_;
  bool is_scrolled_ = true;
};

struct ViewportSystemProxy {
  static void mark_clean(Viewport& viewport) {
    viewport.is_resized_ = false;
    viewport.is_scrolled_ = false;
    viewport.widgets_allocation_changed_ = false;
  }
};

}  // namespace ui
}  // namespace vlk
