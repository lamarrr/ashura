#include <algorithm>
#include <memory>
#include <vector>

#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/widgets/layout_widget_base.h"

#include "stx/span.h"

namespace vlk {
namespace ui {

/*
template <bool IsStateful, typename ChildDeleter = DefaultWidgetChildDeleter,
          typename PointerAllocator = std::allocator<Widget *>>
struct BasicRow
    : public LayoutWidgetBase<IsStateful, ChildDeleter, PointerAllocator> {
  using base = LayoutWidgetBase<IsStateful, ChildDeleter, PointerAllocator>;

  using base::base;

  // TODO(lamarrr): parent widget's dimensions should depend on the dimensions
  // of the children. therefore we compute for the children first, then
  // accumulate and give the parent?
  // we thus would not need `allotted_extent` since this would return the final
  // extent.
  // Don't forget to make Rect t-param const in the Span below
  // Also pass in the surface constraints for it to use in sizing itself if
  // necessary. i.e. max_width (default = u32_max), max_height (default =
  // u32_max)
  //
  //
  // This is the area the visible widget will belong to on the widget tree. It
  // will receive interaction events that are within its boundaries.
  //
  // The child can decide to use the allotted_extent or not.
  virtual Rect compute_area(Extent const &allotted_extent,
                            stx::Span<Rect> const &children_area) override {
    auto children = this->get_children();
    uint32_t num_children = children.size();

    // height property: if the child will use all of the parent's allotted
    // height, allow it. the child widget is however constrained by this
    // widget's width.
    auto max_children_height = std::accumulate(
        children.begin(), children.end(), static_cast<uint32_t>(0),
        [num_children, allotted_extent](uint32_t max_height, Widget *child) {
          auto child_extent = Extent{allotted_extent.width / num_children,
                                     allotted_extent.height};

          std::vector<Rect> vec;
          vec.resize(child->get_children().size());
          return std::max(max_height,
                          child->compute_area(child_extent, vec).extent.height);
        });

    for (uint32_t i = 0; i < num_children; i++) {
      children_area[i].extent.width = allotted_extent.width / num_children;
      children_area[i].extent.height = max_children_height;

      children_area[i].offset.x = children_area[i].extent.width * i;
      children_area[i].offset.y = 0;
    }

    Offset offset{0, 0};
    Extent extent{};
    extent.width = allotted_extent.width;
    extent.height = max_children_height;

    Rect area{offset, extent};

    return area;
  }

  virtual bool is_dirty() const noexcept override { return false; }
  virtual void mark_clean() noexcept override {
    // no-op
  }
};

using Row = BasicRow<false>;

}  // namespace ui
}  // namespace vlk
*/
