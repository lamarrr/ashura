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
#include "simdjson.h"
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
  constraint width;
  constraint height;

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

struct Layout
{
  FlexProps flex;
  rect      area;
  Position  position = Position::Relative;
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
  constexpr virtual stx::Span<Widget *const> get_children()
  {
    return {};
  }

  //
  constexpr virtual WidgetInfo get_info()
  {
    return WidgetInfo{.type = "Widget"};
  }

  //
  constexpr virtual Visibility get_visibility()
  {
    return Visibility::Visible;
  }

  //
  constexpr virtual i64 get_z_index(i64 z_index)
  {
    return z_index;
  }

  //
  constexpr virtual mat4 get_transform()
  {
    return mat4::identity();
  }

  //
  constexpr virtual Layout layout(rect area)
  {
    return Layout{};
  }

  //
  constexpr virtual void draw(gfx::Canvas &canvas, rect area)
  {}

  // called before children are drawn
  constexpr virtual void pre_draw(gfx::Canvas &canvas, Widget &child, rect area)
  {}

  // called once children are drawn
  constexpr virtual void post_draw(gfx::Canvas &canvas, Widget &child, rect area)
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

  constexpr virtual void on_click(Context &context, MouseButton button, vec2 screen_position, u32 nclicks, quad quad)
  {}

  constexpr virtual void on_mouse_move(Context &context, vec2 screen_position, vec2 translation, quad quad)
  {}

  constexpr virtual void on_mouse_enter(Context &context, vec2 screen_position, quad quad)
  {}

  virtual void on_mouse_leave(Context &context, stx::Option<vec2> screen_position)
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

  // TODO(lamarrr): we need a widget build tree???
  //
  virtual simdjson::dom::element save(Context &context, simdjson::dom::parser &parser)
  {
    return parser.parse("{}", 2);
  }

  //
  virtual void restore(Context &context, simdjson::dom::element const &element)
  {}

  // position of the widget on the viewport. typically calculated on every
  // frame.
  rect area;
  u64  id = 0;
};

struct WidgetImpl
{
  template <typename DerivedWidget>
  constexpr WidgetImpl(DerivedWidget widget) :
      impl{new DerivedWidget{std::move(widget)}}
  {
    static_assert(std::is_base_of_v<Widget, DerivedWidget>);
  }

  constexpr WidgetImpl()
  {}

  static constexpr WidgetImpl make(Widget *impl)
  {
    WidgetImpl w;
    w.impl = impl;
    return w;
  }

  constexpr WidgetImpl(WidgetImpl const &) = default;

  constexpr WidgetImpl(WidgetImpl &&) = default;

  constexpr WidgetImpl &operator=(WidgetImpl const &) = default;

  constexpr WidgetImpl &operator=(WidgetImpl &&) = default;

  constexpr ~WidgetImpl() = default;

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

}        // namespace ash
