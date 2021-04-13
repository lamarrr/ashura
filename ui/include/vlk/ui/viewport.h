
#pragma once

#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

// TODO(lamarrr): scroll to widget?
// we might need a pointer binding
// TODO(lamarrr): friend struct tilecache or whatever?
namespace impl {
struct Viewport {
  Viewport(Extent const& extent = Extent{1920, 1080},
           ViewOffset const& offset = ViewOffset{Constrain{.0f},
                                                 Constrain{.0f}},
           ViewExtent const& widgets_allocation =
               ViewExtent{Constrain{1.0f}, Constrain{.0f, u32_max}})
      : extent_{extent},
        offset_{offset},
        widgets_allocation_{widgets_allocation},
        on_resize_{[] {}},
        on_scroll_{[] {}} {}

  Extent get_extent() const { return extent_; }

  ViewOffset get_offset() const { return offset_; }

  ViewExtent get_widgets_allocation() const { return widgets_allocation_; }

  void resize(Extent const& extent = Extent{1920, 1080},
              ViewExtent const& widgets_allocation = ViewExtent{
                  Constrain{1.0f}, Constrain{.0f, u32_max}}) {
    extent_ = extent;
    widgets_allocation_ = widgets_allocation;
    on_resize_();
  }

  void scroll(ViewOffset const& offset = ViewOffset{Constrain{0.0f},
                                                    Constrain{0.0f}}) {
    offset_ = offset;
    on_scroll_();
  }

  std::function<void()>& get_on_resize() { return on_resize_; }

  std::function<void()>& get_on_scroll() { return on_scroll_; }

 private:
  // usually updated due to a resize event
  Extent extent_;

  // usually updated due to a scrolling event
  ViewOffset offset_;

  ViewExtent widgets_allocation_;

  std::function<void()> on_resize_;
  std::function<void()> on_scroll_;
};

}  // namespace impl

}  // namespace ui
}  // namespace vlk
