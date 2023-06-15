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

enum class Position : u8
{
  Relative,
  Static
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
  constraint width       = constraint{.scale = 1};
  constraint height      = constraint{.scale = 1};

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
  FlexProps flex;                                 // flex properties used for layout
  rect      area;                                 // initial area to use, final area will be determined by flex flayout
  Position  position = Position::Relative;        // determines if widget should be positioned relative to parent and flow along with its other children or pop out into its own area
};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct WidgetInfo
{
  std::string_view type;
};

// TODO(lamarrr): we might need request detach so child widgets can request to
// be removed and remove all callbacks they may have attached or cancel tasks
// they have pending.
// consider: having tokens that de-register themselves once deleted
struct Widget
{
  constexpr Widget()
  {}

  constexpr virtual ~Widget()
  {}

  //
  constexpr virtual stx::Span<Widget *const> get_children(Context &context)
  {
    return {};
  }

  // TODO(lamarrr): get_flex_children and get_static_children()

  //
  constexpr virtual WidgetInfo get_info(Context &context)
  {
    return WidgetInfo{.type = "Widget"};
  }

  //
  constexpr virtual Visibility get_visibility(Context &context)
  {
    return Visibility::Visible;
  }

  //
  constexpr virtual i64 get_z_index(Context &context, i64 z_index)
  {
    return z_index;
  }

  //
  constexpr virtual mat4 get_transform(Context &context)
  {
    return mat4::identity();
  }

  //
  constexpr virtual Layout layout(Context &context, rect area)
  {
    return Layout{};
  }

  //
  constexpr virtual void draw(Context &context, gfx::Canvas &canvas, rect area)
  {}

  //
  constexpr virtual void tick(Context &context, std::chrono::nanoseconds interval)
  {}

  //
  constexpr virtual void on_startup(Context &context)
  {}

  //
  constexpr virtual void on_exit(Context &context)
  {}

  //
  constexpr virtual void on_enter_viewport(Context &context)
  {}

  //
  constexpr virtual void on_leave_viewport(Context &context)
  {}

  constexpr virtual bool hit_test(Context &context, vec2 mouse_position);

  // TODO(lamarrr): on mouse up and on mouse down aren't same as dragging
  constexpr virtual void on_mouse_down(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {}

  constexpr virtual void on_mouse_up(Context &context, MouseButton button, vec2 mouse_position, u32 nclicks, quad quad)
  {}

  constexpr virtual void on_mouse_move(Context &context, vec2 mouse_position, vec2 translation, quad quad)
  {}

  constexpr virtual void on_mouse_enter(Context &context, vec2 mouse_position, quad quad)
  {}

  virtual void on_mouse_leave(Context &context, stx::Option<vec2> mouse_position)
  {}

  //
  constexpr virtual void on_tap(Context &context)
  {}

  //
  constexpr virtual void on_drag(Context &context)
  {}

  //
  constexpr virtual void on_drag_start(Context &context)
  {}

  //
  constexpr virtual void on_drag_end(Context &context)
  {}

  //
  constexpr virtual void on_drag_enter(Context &context)
  {}

  //
  constexpr virtual void on_drag_leave(Context &context)
  {}

  //
  constexpr virtual void on_drag_over(Context &context)
  {}

  //
  constexpr virtual void on_drop(Context &context)
  {}

  //
  constexpr virtual void on_touch_cancel(Context &context)
  {}

  //
  constexpr virtual void on_touch_end(Context &context)
  {}

  //
  constexpr virtual void on_touch_move(Context &context)
  {}

  //
  constexpr virtual void on_touch_start(Context &context)
  {}

  //
  constexpr virtual void on_touch_enter(Context &context)
  {}

  //
  constexpr virtual void on_touch_leave(Context &context)
  {}

  // TODO(lamarrr): can this be simpler?
  // virtual void raise_tooltip(){}
  // virtual void lower_tooltip(){}
  rect area;          // position of the widget on the viewport. calculated on every frame
  u64  id = 0;        // id used to recognise the widget. changes from frame to frame??? // TODO(lamarrr)
};

struct WidgetPtr
{
  template <typename DerivedWidget>
  constexpr WidgetPtr(DerivedWidget widget) :
      impl{new DerivedWidget{std::move(widget)}}
  {
    static_assert(std::is_base_of_v<Widget, DerivedWidget>);
  }

  constexpr WidgetPtr()
  {}

  static constexpr WidgetPtr make(Widget *impl)
  {
    WidgetPtr w;
    w.impl = impl;
    return w;
  }

  constexpr WidgetPtr(WidgetPtr const &) = default;

  constexpr WidgetPtr(WidgetPtr &&) = default;

  constexpr WidgetPtr &operator=(WidgetPtr const &) = default;

  constexpr WidgetPtr &operator=(WidgetPtr &&) = default;

  constexpr ~WidgetPtr() = default;

  constexpr Widget *operator->() const
  {
    return impl;
  }

  constexpr operator Widget *() const
  {
    return impl;
  }

  Widget *impl = nullptr;
};

template <typename T>
concept WidgetImpl = std::is_base_of_v<Widget, T>;

}        // namespace ash
