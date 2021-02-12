
#include <algorithm>

#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

STX_FORCE_INLINE constexpr Rect clamp_rect(Rect const &rect,
                                           Extent const &extent) noexcept {
  Rect out{};

  // clamp to the extent
  out.offset.x = std::min(rect.offset.x, extent.width);
  out.offset.y = std::min(rect.offset.y, extent.height);

  // clamp to the remaining space
  out.extent.width =
      std::min(out.offset.x + rect.extent.width, extent.width) - out.offset.x;
  out.extent.height =
      std::min(out.offset.y + rect.extent.height, extent.height) - out.offset.y;

  return out;
}

// TODO(lamarrr): fix message add offset info

STX_FORCE_INLINE void overflow_warn(Rect const &widget_desired_parent_area,
                                    Extent const &parent_allotted_extent,
                                    Widget *widget) {
  auto const widget_x_max = widget_desired_parent_area.extent.width +
                            widget_desired_parent_area.offset.x;

  auto const widget_y_max = widget_desired_parent_area.extent.height +
                            widget_desired_parent_area.offset.y;

  VLK_WARN_IF(widget_desired_parent_area.extent.width == u32_max,
              "widget {}'s (type: {}, address: {}) width is u32_max",
              widget->get_name(), widget->get_type_hint(),
              static_cast<void *>(widget));
  VLK_WARN_IF(widget_desired_parent_area.extent.height == u32_max,
              "widget {}'s (type: {}, address: {}) height is u32_max",
              widget->get_name(), widget->get_type_hint(),
              static_cast<void *>(widget));

  VLK_WARN_IF(widget_x_max > parent_allotted_extent.width,
              "overflow on x-axis by {}px detected in widget: {} (type: {}, "
              "address: {}) >>> "
              "parent allotted width: {}px, widget requested: {}px offset and "
              "{}px extent",
              widget_x_max - parent_allotted_extent.width, widget->get_name(),
              widget->get_type_hint(), static_cast<void *>(widget),
              parent_allotted_extent.width, widget_desired_parent_area.offset.x,
              widget_desired_parent_area.extent.width);
  VLK_WARN_IF(widget_y_max > parent_allotted_extent.height,
              "overflow on y-axis by {}px detected in widget: {} (type: {}, "
              "address: {}) >>> "
              "parent allotted height: {}px, widget requested: {}px offset and "
              "{}px extent",
              widget_y_max - parent_allotted_extent.height, widget->get_name(),
              widget->get_type_hint(), static_cast<void *>(widget),
              parent_allotted_extent.height,
              widget_desired_parent_area.offset.y,
              widget_desired_parent_area.extent.height);
}

}  // namespace ui
}  // namespace vlk
