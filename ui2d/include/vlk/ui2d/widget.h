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
struct [[nodiscard]] Widget {
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
  [[nodiscard]] virtual bool is_layout_type() const noexcept = 0;

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
  [[nodiscard]] virtual bool is_stateful() const noexcept = 0;

  // called to check if the rendering data of the widget has changed.
  // called on every frame if and only if the widget is a stateful type.
  [[nodiscard]] virtual bool is_dirty() const noexcept = 0;

  // marks that the renderer has responded and rebuilt this widget.
  virtual void mark_clean() noexcept = 0;

  // gets the list of children for this widget
  [[nodiscard]] virtual stx::Span<Widget *const> get_children()
      const noexcept = 0;

  [[nodiscard]] bool has_children() const noexcept {
    return !get_children().empty();
  }

  // static or part of widget state?
  // if this changes from the one in our cache entry then we need to update and
  // put it in the appropriate position by default, z_index increases by
  // increasing depth
  [[nodiscard]] virtual stx::Option<uint32_t> z_index() const noexcept {
    return stx::None;
  }

  // used for optimizing the case where the widget is a simple one and don't
  // take much of render time. i.e. a simple color fill block.
  // TODO(lamarrr) virtual bool is_simple() const noexcept { return false; }

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
  [[nodiscard]] virtual Rect compute_area(
      Extent const &allotted_extent, stx::Span<Rect> const &children_area) = 0;

  // draw itself. if it has children, the children will be drawn separately.
  // for overlaying, use a `Stack` widget instead.
  virtual void draw(Canvas & canvas, Extent const &requested_extent) = 0;

  // useful for debugging widgets
  [[nodiscard]] virtual std::string_view get_name() const noexcept {
    return "<unnamed>";
  }

  // get a type identifier for this widget type.
  [[nodiscard]] virtual std::string_view get_type_hint() const noexcept = 0;

  [[nodiscard]] virtual bool should_cache() const noexcept { return true; }

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

}  // namespace ui2d
}  // namespace vlk

// Stacking widgets will override the draw method and draw thier child widgets
// in the order defined by their provided z-indices.

// TODO(lamarrr): have a root widget to act as a surface. i.e. Scaffold. we can
// do special checking of bounds in it instead of manual checking.
