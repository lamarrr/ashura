#pragma once
#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>

#include "ashura/canvas.h"
#include "ashura/context.h"
#include "ashura/event.h"
#include "ashura/primitives.h"
#include "ashura/uuid.h"
#include "stx/async.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash
{

enum class Visibility : u8
{
  Visible,
  Hidden
};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct WidgetDebugInfo
{
  std::string_view type;
};

// TODO(lamarrr)
struct DragData
{
  virtual ~DragData()
  {}

  stx::Option<uuid> source;
};

// TODO(lamarrr): we might need request detach so child widgets can request to
// be removed and remove all callbacks they may have attached or cancel tasks
// they have pending.
// consider: having tokens that de-register themselves once deleted

/// @brief Base widget class. All widget types must inherit from this struct.
/// all methods are already implemented with reasonable defaults.
struct Widget
{
  Widget()
  {}

  virtual ~Widget()
  {}

  /// @brief get child widgets
  /// @param ctx
  /// @return
  virtual stx::Span<Widget *const> get_children(Context &ctx)
  {
    return {};
  }

  /// @brief get debug and logging information
  /// @param ctx
  /// @return
  virtual WidgetDebugInfo get_debug_info(Context &ctx)
  {
    return WidgetDebugInfo{.type = "Widget"};
  }

  // TODO(lamarrr): we need re-calculable offsets so we can shift the parents around without shifting the children
  // this is important for cursors, drag and drop?
  // this might mean we need to totally remove the concept of area. storing transformed area might not be needed?

  /// @brief distributes the size allocated to it to its child widgets.
  /// unlike CSS. has the advantage that children wouldn't need extra attributes for specific kind of placements i.e. relative, absolute, etc.
  /// @param ctx
  /// @param allocated_size the size allocated to this widget
  /// @param[out] children_allocation sizes allocated to the children.
  virtual void allocate_size(Context &ctx, vec2 allocated_size, stx::Span<vec2> children_allocation)
  {
    children_allocation.fill(vec2{0, 0});
  }

  /// @brief fits itself around its children and positions child widgets along/relative to itself (i.e. position {0, 0} means the child will be placed on the top left of the parent)
  /// @param ctx
  /// @param allocated_size the size allocated to this widget. the widget can decide to disregard or fit to this as needed.
  /// @param children_sizes sizes of the child widgets
  /// @param[out] children_positions positions of the children widget on the parent
  /// @return this widget's fitted extent
  virtual vec2 fit(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions)
  {
    return vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the widget
  /// @param ctx
  /// @param allocated_position the allocated absolute position of this widget
  /// @return
  virtual vec2 position(Context &ctx, vec2 allocated_position)
  {
    return allocated_position;
  }

  /// @brief returns the visibility of this widget. an invisible widget will not be drawn nor receive mouse/touch events.
  /// parents can decide the visibility of eacg child
  /// @param ctx
  /// @param allocated_visibility
  /// @param[out] children_allocation visibility assigned to children
  /// @return
  virtual Visibility get_visibility(Context &ctx, Visibility allocated_visibility, stx::Span<Visibility> children_allocation)
  {
    children_allocation.fill(allocated_visibility);
    return allocated_visibility;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param ctx
  /// @param allocated_z_index
  /// @param[out] children_allocation z-index assigned to children
  /// @return
  virtual i32 z_stack(Context &ctx, i32 allocated_z_index, stx::Span<i32> children_allocation)
  {
    children_allocation.fill(allocated_z_index + 1);
    return allocated_z_index;
  }

  /// @brief this is used for clipping widget views. the provided clip is relative to the root widget's axis (0, 0).
  /// this can be used for nested viewports where there are multiple intersecting clips.
  /// transforms do not apply to the clip rects. this is used for visibility testing and eventually actual vertex culling.
  /// a nested viewport for example can therefore use the intersection of its allocated clip and it's own viewport clip and assign that to its children,
  /// whilst using the allocated clip on itself.
  /// @param ctx
  /// @param allocated_clip
  /// @param[out] children_allocation
  /// @return
  virtual rect clip(Context &ctx, rect allocated_clip, stx::Span<rect> children_allocation)
  {
    children_allocation.fill(allocated_clip);
    return allocated_clip;
  }

  /// @brief record draw commands needed to render this widget. this method is only called if the widget passes the visibility tests.
  /// this is called on every frame.
  /// @param ctx
  /// @param canvas
  ///
  ///
  virtual void draw(Context &ctx, gfx::Canvas &canvas)
  {
    // TODO(lamarrr): the whole widget tree will be rendered and clipped as necessary
  }

  // TODO(lamarrr): draw_tooltip();

  /// @brief called on every frame. used for state changes, animations, task dispatch and lightweight processing related to the GUI.
  /// heavy-weight and non-sub-millisecond tasks should be dispatched to a Subsystem that would handle that. i.e. using the multi-tasking system.
  /// @param ctx
  /// @param interval time passed since last call to this method
  virtual void tick(Context &ctx, std::chrono::nanoseconds interval)
  {}

  /// @brief called on every frame the widget is viewed on the viewport.
  /// @param ctx
  virtual void on_view_hit(Context &ctx)
  {}

  /// @brief called on every frame that the widget is not seen on the viewport
  /// this can be because it has hidden visibility, is clipped away, or parent positioned out of the visible region
  /// @param ctx
  virtual void on_view_miss(Context &ctx)
  {}

  // TODO(lamarrr): this needs to happen before mouse actions as some widgets .i.e. text don't need to intercept or receive mouse events
  virtual bool hit_test(Context &ctx, vec2 mouse_position)
  {
    return false;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks)
  {}

  virtual void on_mouse_up(Context &ctx, MouseButton button, vec2 mouse_position, u32 nclicks)
  {}

  // TODO(lamarrr): how do we fix translation and zooming? i.e. positioning once transform is applied
  virtual void on_mouse_move(Context &ctx, vec2 mouse_position, vec2 translation)
  {}

  virtual void on_mouse_enter(Context &ctx, vec2 mouse_position)
  {}

  virtual void on_mouse_leave(Context &ctx, stx::Option<vec2> mouse_position)
  {}

  // virtual bool on_mouse_wheel(Context& ctx, vec2 translation, vec2 mouse_position?). propagates up

  // signifies that this widget is about to be dragged
  // return true if this widget allows dragging
  // TODO(lamarrr): see https://github.com/ocornut/imgui/issues/1931
  virtual bool on_drag_start(Context &ctx)
  {
    return false;
  }

  /// @brief called when theres a drag position update
  /// @param ctx
  /// @param global_position current global drag position
  /// @param local_position current position relative to its initial position
  /// @param delta difference between this drag update and the last drag update position
  virtual void on_drag_update(Context &ctx, vec2 global_position, vec2 local_position, vec2 delta)
  {}

  // signifies that the dragging of this widget has been canceled. i.e. released to an area without a widget that accepts the drag event
  virtual void on_drag_canceled(Context &ctx)
  {}

  // the drag operation has been performed
  virtual void on_drag_end(Context &ctx)
  {}

  // this widget has begun receiving drag data, i.e. it has been dragged onto
  //
  // returns true if widget can accept this drag event
  virtual bool on_drag_enter(Context &ctx)
  {
    return false;
  }

  /// this widget has previously begun receiving drag data, but the mouse is still dragging within it
  virtual void on_drag_over(Context &ctx)
  {}

  /// the drag event has left this widget
  virtual void on_drag_leave(Context &ctx)
  {}

  // drop of media file/item outside the context of the application
  virtual void on_drop(Context &ctx)
  {}

  //
  virtual void on_tap(Context &ctx)
  {}

  //
  virtual void on_touch_cancel(Context &ctx)
  {}

  //
  virtual void on_touch_end(Context &ctx)
  {}

  //
  virtual void on_touch_move(Context &ctx)
  {}

  //
  virtual void on_touch_start(Context &ctx)
  {}

  //
  virtual void on_touch_enter(Context &ctx)
  {}

  //
  virtual void on_touch_leave(Context &ctx)
  {}

  stx::Option<uuid> id;          /// id used to recognise the widget. checked every frame. if one is not present or removed. a new uuid is generated and assigned.
  rect              area;        ///
};

template <typename T>
concept WidgetImpl = std::is_base_of_v<Widget, T>;

inline Widget *__find_widget_recursive(Context &ctx, Widget &widget, uuid id)
{
  if (widget.id.contains(id))
  {
    return &widget;
  }

  for (Widget *child : widget.get_children(ctx))
  {
    Widget *found = __find_widget_recursive(ctx, *child, id);
    if (found != nullptr)
    {
      return found;
    }
  }

  return nullptr;
}

}        // namespace ash
