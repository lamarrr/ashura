#pragma once

#include <memory>
#include <string_view>

#include "stx/span.h"
#include "vlk/ui/canvas.h"
#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

// Widgets do not contain any spatio-temporal models, they should not be
// wrapped.
// The widget itself **must** not touch the children as widgets are modeled for
// rendering independent of one another.
// touch processing: find widgets on touched surface area. if returns multiple,
// they should be children???? what about stack? we might need R-tree after all
// events should be processed in the R-tree/map not by the renderer
// tap_event(Offset position)
struct [[nodiscard]] Widget {
  enum class [[nodiscard]] Type : uint8_t{Render, Layout, View};

  [[nodiscard]] virtual Type get_type() const noexcept = 0;

  [[nodiscard]] bool is_render_type() const noexcept {
    return get_type() == Type::Render;
  }

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
  [[nodiscard]] bool is_layout_type() const noexcept {
    return get_type() == Type::Layout;
  }

  // TODO(lamarrr): remove default
  // view types like layout types do not contain render data.
  // they are widget that present a view over their children.
  [[nodiscard]] bool is_view_type() const noexcept {
    return get_type() == Type::View;
  }

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

  [[nodiscard]] bool is_stateless() const noexcept { return !is_stateful(); }

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
  // The extent is allowed to exceed `allotted_extent`  if the widget is a view
  // type.
  [[nodiscard]] virtual Rect compute_area(
      Extent const &allotted_extent, stx::Span<Rect> const &children_area) = 0;

  // called everytime is_stateful returns true.
  // .extent of the returned rect should rarely change. we keep track of it and
  // if it changed on calling this function, we have to perform a rebuild of its
  // children. the returned rect's .offset is the offset from the area returned
  // from compute_area and the widgets would be laid out on the canvas based on
  // this offset. The rect returned here must be contained in the rect returned
  // in `compute_area`
  // the rect can be used a sliding window accross the widget.
  [[nodiscard]] virtual Rect compute_view_area(Extent const &allotted_extent,
                                               Extent const &requested_extent) {
    return Rect{Offset{0, 0}, requested_extent};
  }

  // draw itself. if it has children, the children will be drawn separately.
  // for overlaying, use a `Stack` widget instead.
  virtual void draw(Canvas & canvas, Extent const &requested_extent) = 0;

  // useful for debugging widgets
  [[nodiscard]] virtual std::string_view get_name() const noexcept {
    return "<unnamed>";
  }

  // get a type identifier hint for this widget type.
  [[nodiscard]] virtual std::string_view get_type_hint() const noexcept = 0;

  [[nodiscard]] virtual bool should_cache() const noexcept { return true; }

  // enables the widget's appropriate destructor to be called.
  // widget graphs will be held in memory till rendering is finished.
  virtual ~Widget() noexcept {}
};

// Ownership of the provided widgets is taken
// NOTE: this should only be used for widgets that have raster data that
// underlays its child widgets.
// TODO(lamarrr): Positioning elements don't need this, create a
// ForwardingWidgetBase. and make it store the vector of ui elements. on events,
// it forwards the event to the appropriate child. It thus would not need any
// canvas for rendering.
//
using DefaultWidgetDeleter = std::default_delete<Widget>;
using DefaultWidgetChildDeleter = DefaultWidgetDeleter;

struct AbsRel {
 private:
  union {
    uint32_t absolute_value_;
    float relative_value_;
  };

  bool is_absolute_;
};

enum class SelfSizingAttribute : uint16_t {
  IndependentX =
      0b0000'0000,  // it is a fixed size and doesn't depend on neither parent
                    // nor child widget,  makes no function call but retrieves
  DependChildX,     // its sizing depends on the child's sizing. equation = {
                    // max(low, min(scaling_factor * child_sizing +
                    // sizing_increment, high)) }
  ExpandX,          // occupies the whole space of the parent

  IndependentY,
  DependChildY,
  ExpandY,
};

struct ChildAreaAllocation {
  // max(low, min(scaling_factor * allotted_size + bias, high))
  struct Parameters {
    float scale = 1.0f;
    uint32_t bias = u32_min;
    uint32_t low = u32_min;
    uint32_t high = u32_max;
  };

  struct Area {
    Parameters x;
    Parameters y;
    Parameters width;
    Parameters height;
  };

  stx::Span<Parameters> children_size_allocation;

  size_t child_index;
};

struct SelfSizing {
  // max(low, min(scaling_factor * allotted_size + bias, high))
  struct IndependentParameters {
    float scale = 1.0f;
    uint32_t bias = u32_min;
    uint32_t low = u32_min;
    uint32_t high = u32_max;
  };

  // we'd have to query the child's sizing first by giving it the max allottable
  // and min allottable area of the parent?
  // max(low, min(scaling_factor * allotted_size + bias, high))
  struct DependentParameters {
    float scale = 1.0f;
    uint32_t bias = u32_min;
    uint32_t low = u32_min;
    uint32_t high = u32_max;
  };
};

}  // namespace ui
}  // namespace vlk

// Stacking widgets will override the draw method and draw thier child widgets
// in the order defined by their provided z-indices.

// TODO(lamarrr): have a root widget to act as a surface. i.e. Scaffold. we can
// do special checking of bounds in it instead of manual checking.
