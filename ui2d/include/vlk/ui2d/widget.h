#pragma once

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <map>
#include <memory>
#include <numeric>
#include <string_view>
#include <vector>

#include "stx/option.h"

#include "vlk/assets/image.h"
#include "vlk/ui2d/canvas.h"
#include "vlk/ui2d/primitives.h"
#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

#include "include/core/SkFont.h"

namespace vlk {

namespace ui2d {

// Widgets do not contain any spatio-temporal models, they should not be
// wrapped.
// The widget itself **must** not touch the children as widgets are modeled for
// rendering independent of one another.
// touch processing: find widgets on touched surface area. if returns multiple,
// they should be children???? what about stack? we might need R-tree after all
// events should be processed in the R-tree/map not by the renderer
// tap_event(Offset position)
struct Widget {
  // used by the widget render compositor to determine if a widget needs drawing
  // or not.
  //
  // layout widgets will occupy a region of the surface but would not have
  // draw data nor receive interaction events i.e. Align, Margin, Column, Row.
  // they can be stateful or stateless. stateful layout widgets when
  // dirty will disrupt the widget tree and trigger a rebuild of all the
  // widget's dimensions.
  //
  // Optimizations: Interactive events can only be received by widgets that are
  // actually rendered. They don't need to be a part of the residual/active
  // widgets group.
  //
  //
  // `draw` will not be called if widget is a layout type.
  //
  //
  // TODO(lamarrr): what can we do to prevent rebuilding the whole tree?
  virtual bool is_layout_type() const noexcept = 0;

  // will any property of this widget that can affect its or its children
  // rendering change?
  //
  // statelss widgets are ***always*** stateless and their rendering data will
  // not
  // change throughout their lifetime and their `is_dirty` method is never
  // called.
  //
  //
  // stateful widgets are stateful by default. and their rendering data is
  // assumed to change on every frame. their statefulness can be overriden using
  // the `is_stateful` method to provide more complex statefulness behaviours.
  // i.e. only stateless when all its children are stateless. if `is_stateful`
  // returns true, their `is_dirty` method is called on every frame to check if
  // their rendering data has changed.
  //
  //
  virtual bool is_stateful() const noexcept = 0;

  // called to check if the rendering data of the widget has changed.
  // called on every frame if and only if the widget is a stateful type.
  virtual bool is_dirty() const noexcept = 0;

  // marks that the renderer has responded and rebuilt this widget.
  virtual void mark_clean() noexcept = 0;

  // gets the list of children for this widget
  virtual stx::Span<Widget *const> get_children() const noexcept = 0;

  bool has_children() const noexcept { return !get_children().empty(); }

  // used for positioning this widget within it's allotted area by its parent.
  // it should return an area it wants to occupy out of the allotted_extent, and
  // an extent it actually needs for canvas drawing.
  //
  //  For widgets that have children, it'd be more efficient to use a cache to
  //  get the dimensions of the child widgets than manually re-computing them in
  //  this widget everytime. As this widget could also have parents that depend
  //  on it and already calculated and used this widget's and its children
  //  dimensions. this could be very costly depending on the number of widget
  //  hierarchy depth being calculated for.
  //
  //  if this widget has children it should position its children within itself
  //  by modifying the `children_area` span, `children_area.size()` is always
  //  equal to `get_children().size()`.
  //
  // Optimizations:
  //    -   the widget might not use all of the extent allotted to it by the
  //    parent widget.
  //    -   the extent returned will be used for allocating a canvas for
  //    drawing.
  //
  virtual Rect compute_area(Extent const &allotted_extent,
                            stx::Span<Rect> const &children_area) const
      noexcept = 0;

  // draw itself. if it has children, the children will be drawn separately.
  // for overlaying, use a `Stack` widget instead.
  virtual void draw(Canvas &canvas, Extent const &requested_extent) const = 0;

  // useful for debugging widgets
  virtual std::string_view get_name() const noexcept { return "<unnamed>"; }

  // get a type identifier for this widget type. uses RTTI.
  virtual std::string_view get_type_name() const noexcept final {
    auto &info = typeid(*this);
    return info.name();
  }

  // enables the widget's appropriate destructor to be called.
  // widget graphs will be held in memory till rendering is finished.
  virtual ~Widget() noexcept {}
};

using DefaultWidgetDeleter = std::default_delete<Widget>;
using DefaultWidgetChildDeleter = DefaultWidgetDeleter;

// Ownership of the provided widgets is taken
// NOTE: this should only be used for widgets that have raster data that
// underlays its child widgets.
// TODO(lamarrr): Positioning elements don't need this, create a
// ForwardingWidgetBase. and make it store the vector of ui elements. on events,
// it forwards the event to the appropriate child. It thus would not need any
// canvas for rendering.
//

template <bool IsStateful, typename ChildDeleter = DefaultWidgetChildDeleter,
          typename PointerAllocator = std::allocator<Widget *>>
struct LayoutWidgetBase : public Widget {
  explicit LayoutWidgetBase(stx::Span<Widget *const> const &children) {
    add_children_(children);
  }

  explicit LayoutWidgetBase(std::initializer_list<Widget *> const &children) {
    add_children_(children);
  }

  virtual bool is_layout_type() const noexcept final override { return true; }

  virtual bool is_stateful() const noexcept final override {
    return IsStateful;
  }

  virtual stx::Span<Widget *const> get_children() const
      noexcept final override {
    return children_;
  }

  virtual Rect compute_area(Extent const &allotted_extent,
                            stx::Span<Rect> const &children_area) const
      noexcept override = 0;

  virtual void draw(
      [[maybe_unused]] Canvas &canvas,
      [[maybe_unused]] Extent const &requested_extent) const final override {
    // no-op, won't be called since `is_positioning_type` is true.
  }

  virtual ~LayoutWidgetBase() noexcept override { delete_children_(); }

 private:
  std::vector<Widget *, PointerAllocator> children_;

  void add_children_(stx::Span<Widget *const> const &children) {
    // exception needs to be handled because we've taken ownership of the widget
    VLK_ENSURE(children.size() <= static_cast<uint64_t>(u32_max),
               "size of widget children exceeded u32_max", children.size());

    try {
      children_.insert(children_.end(), children.begin(), children.end());
    } catch (std::exception &oom) {
      for (Widget *child : children) {
        ChildDeleter{}(child);
      }
      throw oom;
    }
  }

  void delete_children_() {
    for (auto *child : children_) {
      ChildDeleter{}(child);
    }
    children_.clear();
  }
};

template <bool IsStateful, typename ChildDeleter = DefaultWidgetChildDeleter,
          typename PointerAllocator = std::allocator<Widget *>>
struct BasicColumn
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
                            stx::Span<Rect> const &children_area) const
      noexcept override {
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

using Column = BasicColumn<false>;

}  // namespace ui2d
}  // namespace vlk

// Stacking widgets will override the draw method and draw thier child widgets
// in the order defined by their provided z-indices.

// TODO(lamarrr): have a root widget to act as a surface. i.e. Scaffold. we can
// do special checking of bounds in it instead of manual checking.
