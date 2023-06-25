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

enum class Direction : u8
{
  Row,
  Column
};

enum class Wrap : u8
{
  None,
  Wrap
};

/// main-axis alignment
/// affects how free space is used on the main axis
/// main-axis for row flex is x
/// main-axis for column flex is y
enum class MainAlign : u8
{
  Start,
  End,
  SpaceBetween,
  SpaceAround,
  SpaceEvenly
};

/// cross-axis alignment
/// affects how free space is used on the cross axis
/// cross-axis for row flex is y
/// cross-axis for column flex is x
enum class CrossAlign : u8
{
  Start,
  End,
  Center,
  Stretch
};

enum class Fit : u8
{
  Shrink,
  Expand
};

struct FlexProps
{
  Direction  direction   = Direction::Row;
  Wrap       wrap        = Wrap::Wrap;
  MainAlign  main_align  = MainAlign::Start;
  CrossAlign cross_align = CrossAlign::Start;
  Fit        main_fit    = Fit::Shrink;
  Fit        cross_fit   = Fit::Shrink;

  constexpr vec2 fit(vec2 span, vec2 initial_extent) const
  {
    vec2 extent;

    if (main_fit == Fit::Shrink)
    {
      if (direction == Direction::Row)
      {
        extent.x = std::min(span.x, initial_extent.x);
      }
      else
      {
        extent.y = std::min(span.y, initial_extent.y);
      }
    }
    else
    {
      // expand
      if (direction == Direction::Row)
      {
        extent.x = initial_extent.x;
      }
      else
      {
        extent.y = initial_extent.y;
      }
    }

    if (cross_fit == Fit::Shrink)
    {
      if (direction == Direction::Row)
      {
        extent.y = std::min(span.y, initial_extent.y);
      }
      else
      {
        extent.x = std::min(span.x, initial_extent.x);
      }
    }
    else
    {
      // expand
      if (direction == Direction::Row)
      {
        extent.y = initial_extent.y;
      }
      else
      {
        extent.x = initial_extent.x;
      }
    }

    return extent;
  }
};

// TODO(lamarrr): i.e. a number floating on a picture
struct Layout
{
  FlexProps  flex;        /// flex properties used for layout
  rect       area;        /// initial area to use, final area will be determined by flex flayout
  EdgeInsets padding;
};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct WidgetInfo
{
  std::string_view type;
};

struct DragData
{
  virtual ~DragData()
  {}
};

// TODO(lamarrr): we might need request detach so child widgets can request to
// be removed and remove all callbacks they may have attached or cancel tasks
// they have pending.
// consider: having tokens that de-register themselves once deleted
struct Widget
{
  Widget()
  {}

  virtual ~Widget()
  {}

  // TODO(lamarrr): what if we want to position child relative to the parent? positioning to 0,0 isn't a good idea
  virtual stx::Span<Widget *const> get_flex_children(Context &ctx)
  {
    return {};
  }

  /// all floating children will be laid out to the left and stack on each other,
  /// no flex layout rules will be applied to them
  virtual stx::Span<Widget *const> get_floating_children(Context &ctx)
  {
    return {};
  }

  //
  virtual WidgetInfo get_info(Context &ctx)
  {
    return WidgetInfo{.type = "Widget"};
  }

  /// called at the beginning of the application
  virtual void on_startup(Context &ctx)
  {}

  /// called at application exit
  virtual void on_exit(Context &ctx)
  {}

  /// returns the visibility of this widget. an invisible widget will not be drawn nor receive mouse/touch events
  virtual Visibility get_visibility(Context &ctx)
  {
    return Visibility::Visible;
  }

  /// returns the desired z_index of this widget given the hierarchy assigned z_index.
  virtual i64 get_z_index(Context &ctx, i64 z_index)
  {
    return z_index;
  }

  /// returns the rect placement transform of this widget. this will be used in positioning the widget
  virtual mat4 get_transform(Context &ctx)
  {
    return mat4::identity();
  }

  /// called when layout of the widget is required
  /// area is the widget's assigned area
  virtual Layout layout(Context &ctx, rect area)
  {
    return Layout{};
  }

  //
  virtual void draw(Context &ctx, gfx::Canvas &canvas)
  {}

  /// called on every frame. interval is the time passed since the last call to this tick method
  /// state changes, animations, task dispatch and lightweight processing related to the GUI should happen here
  virtual void tick(Context &ctx, std::chrono::nanoseconds interval)
  {}

  //
  virtual void on_enter_viewport(Context &ctx)
  {}

  //
  virtual void on_leave_viewport(Context &ctx)
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

  // signifies that this widget is about to be dragged
  // return true if this widget allows dragging
  virtual bool on_drag_start(Context &ctx)
  {
    return false;
  }

  // gives the drag position of the widget,
  /**
   * @brief
   *
   * @param context
   * @param global_position: current global drag position
   * @param local_position: current position relative to its initial position
   * @param delta: difference between this drah update and the last drag update
   */
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

  stx::Option<uuid> id;                      // id used to recognise the widget. checked every frame. if one is not present or removed. a value is assigned.
  rect              area;                    // area of the widget on the viewport. calculated on every frame
  quad              transformed_area;        // transformed area of the widget on the viewport. calculated on every frame
};

template <typename T>
concept WidgetImpl = std::is_base_of_v<Widget, T>;

// TODO(lamarrr): what if we want a widget to be at the edge of its parent?

// TODO(lamarrr): in layout we were performing layout twice. some widgets will
// TODO(lamarrr): we need re-calculable offsets so we can shift the parents around without shifting the children
// this is important for layout.
// this might mean we need to totally remove the concept of area. storing transformed area might not be needed?

// has the advantage that children wouldn't need extra attributes for specific kind of placements
//
struct SWidget
{
  /**
   * @brief
   *
   * @param ctx
   * @param children_sizes: sizes of the child widgets
   * @param children_positions: layout positions of the children widget on the view
   * @param allocated_size: the size allocated to this widget
   * @return this widget's extent
   */
  virtual vec2 layout(Context &ctx, vec2 allocated_size, stx::Span<vec2 const> children_sizes, stx::Span<vec2> children_positions) = 0;

  /**
   * @brief allocate sizes to the child widgets
   *
   * @param ctx
   * @param children_allocation
   * @param allocated_size
   */
  virtual void allocate_size(Context &ctx, vec2 allocated_size, stx::Span<vec2> children_allocation) = 0;
};

}        // namespace ash
