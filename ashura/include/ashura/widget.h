#pragma once
#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>

#include "ashura/asset_bundle.h"
#include "ashura/canvas.h"
#include "ashura/event.h"
#include "ashura/plugin.h"
#include "ashura/primitives.h"
#include "simdjson.h"
#include "spdlog/logger.h"
#include "stx/async.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/scheduler.h"
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
  u64              id = 0;
};

// add window controller class
struct WidgetContext
{
  std::map<std::string, Plugin *, std::less<>> plugins;
  stx::TaskScheduler                          *task_scheduler = nullptr;
  spdlog::logger                              *logger         = nullptr;

  ~WidgetContext()
  {
    for (auto &entry : plugins)
    {
      delete entry.second;
    }
  }

  void register_plugin(Plugin *plugin)
  {
    std::string_view id = plugin->get_id();
    auto [it, inserted] = plugins.emplace(id, plugin);
    if (inserted)
      return;
    usize index = 1;
    while (!inserted)
    {
      auto [it, was_inserted] = plugins.emplace(fmt::format("{} ({})", id, index), plugin);
      index++;
      inserted = was_inserted;
    }
  }

  template <typename T>
  stx::Option<T *> get_plugin(std::string_view id) const
  {
    auto pos = plugins.find(id);
    if (pos != plugins.end())
    {
      return stx::Some(pos->second->as<T>());
    }
    else
    {
      return stx::None;
    }
  }
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
    return WidgetInfo{.type = "Widget", .id = id};
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
  constexpr virtual void tick(WidgetContext &context, std::chrono::nanoseconds interval)
  {}

  //
  constexpr virtual void on_launch(WidgetContext &context)
  {}

  //
  constexpr virtual void on_exit(WidgetContext &context)
  {}

  //
  constexpr virtual void on_enter_viewport(WidgetContext &context)
  {}

  //
  constexpr virtual void on_leave_viewport(WidgetContext &context)
  {}

  constexpr virtual void on_click(WidgetContext &context, MouseButton button, vec2 screen_position, u32 nclicks, quad quad)
  {}

  constexpr virtual void on_mouse_move(WidgetContext &context, vec2 screen_position, vec2 translation, quad quad)
  {}

  constexpr virtual void on_mouse_enter(WidgetContext &context, vec2 screen_position, quad quad)
  {}

  virtual void on_mouse_leave(WidgetContext &context, stx::Option<vec2> screen_position)
  {}

  //
  constexpr virtual void on_tap(WidgetContext &context)
  {}

  //
  constexpr virtual void on_drag(WidgetContext &context)
  {}

  //
  constexpr virtual void on_drag_start(WidgetContext &context)
  {}

  //
  constexpr virtual void on_drag_end(WidgetContext &context)
  {}

  //
  constexpr virtual void on_drag_enter(WidgetContext &context)
  {}

  //
  constexpr virtual void on_drag_leave(WidgetContext &context)
  {}

  //
  constexpr virtual void on_drag_over(WidgetContext &context)
  {}

  //
  constexpr virtual void on_drop(WidgetContext &context)
  {}

  //
  constexpr virtual void on_touch_cancel(WidgetContext &context)
  {}

  //
  constexpr virtual void on_touch_end(WidgetContext &context)
  {}

  //
  constexpr virtual void on_touch_move(WidgetContext &context)
  {}

  //
  constexpr virtual void on_touch_start(WidgetContext &context)
  {}

  //
  constexpr virtual void on_touch_enter(WidgetContext &context)
  {}

  //
  constexpr virtual void on_touch_leave(WidgetContext &context)
  {}

  // TODO(lamarrr): can this be simpler?
  // virtual void raise_tooltip(){}
  // virtual void lower_tooltip(){}

  // TODO(lamarrr): we need a widget build tree???
  //
  virtual simdjson::dom::element save(WidgetContext &context, simdjson::dom::parser &parser)
  {
    return parser.parse("{}", 2);
  }

  //
  virtual void restore(WidgetContext &context, simdjson::dom::element const &element)
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
