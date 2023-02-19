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
#include "ashura/primitives.h"
#include "simdjson.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash {

enum class Visibility : u8 { Visible, Hidden };

enum class Direction : u8 { Row, Column };

enum class Wrap : u8 { None, Wrap };

enum class Position : u8 { Relative, Static };

/// main-axis alignment
/// affects how free space is used on the main axis
/// main-axis for row flex is x
/// main-axis for column flex is y
enum class MainAlign : u8 {
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
enum class CrossAlign : u8 { Start, End, Center, Stretch };

enum class Fit : u8 { Shrink, Expand };

struct FlexProps {
  Direction direction = Direction::Row;
  Wrap wrap = Wrap::Wrap;
  MainAlign main_align = MainAlign::Start;
  CrossAlign cross_align = CrossAlign::Start;
  Fit main_fit = Fit::Shrink;
  Fit cross_fit = Fit::Shrink;
  constraint width;
  constraint height;

  constexpr vec2 fit(vec2 span, vec2 initial_extent) const {
    vec2 extent;

    if (main_fit == Fit::Shrink) {
      if (direction == Direction::Row) {
        extent.x = std::min(span.x, initial_extent.x);
      } else {
        extent.y = std::min(span.y, initial_extent.y);
      }
    } else {
      // expand
      if (direction == Direction::Row) {
        extent.x = initial_extent.x;
      } else {
        extent.y = initial_extent.y;
      }
    }

    if (cross_fit == Fit::Shrink) {
      if (direction == Direction::Row) {
        extent.y = std::min(span.y, initial_extent.y);
      } else {
        extent.x = std::min(span.x, initial_extent.x);
      }
    } else {
      // expand
      if (direction == Direction::Row) {
        extent.y = initial_extent.y;
      } else {
        extent.x = initial_extent.x;
      }
    }

    return extent;
  }
};

struct Layout {
  FlexProps flex;
  rect area;
  Position position = Position::Relative;
};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct WidgetInfo {
  std::string_view type;
  u64 id = 0;
};

struct WidgetContext {
  // renderer context
  // asset bundles
  std::map<std::string, Plugin *, std::less<>> plugins;

  template <typename T>
  stx::Option<T *> get_plugin(std::string_view id) const {
    auto pos = plugins.find(id);
    if (pos != plugins.end()) {
      return stx::Some(pos->second->as<T>());
    } else {
      return stx::None;
    }
  }
};

/// SEE: (https://www.w3.org/TR/uievents)
//
// TODO(lamarrr): should we go recursive?
//
struct Widget {
  constexpr Widget() {}

  constexpr virtual ~Widget() {}

  //
  constexpr virtual stx::Span<Widget *const> get_children() { return {}; }

  //
  constexpr virtual WidgetInfo get_info() {
    return WidgetInfo{.type = "Widget", .id = id};
  }

  //
  constexpr virtual Visibility get_visibility() { return Visibility::Visible; }

  //
  constexpr virtual i64 get_z_index(i64 z_index) { return z_index; }

  //
  constexpr virtual Layout layout(rect area) { return Layout{}; }

  //
  constexpr virtual void draw(gfx::Canvas &canvas, rect area) {}

  // called before children are drawn
  // constexpr virtual void pre_draw(gfx::Canvas &canvas, usize child) {}

  // called once children are drawn
  // constexpr virtual void post_draw(gfx::Canvas &canvas, usize child) {}

  //
  constexpr virtual void tick(WidgetContext &context,
                              std::chrono::nanoseconds interval) {}

  //
  constexpr virtual void on_launch(WidgetContext &context) {}

  //
  constexpr virtual void on_exit(WidgetContext &context) {}

  //
  constexpr virtual void on_enter_viewport() {}

  //
  constexpr virtual void on_leave_viewport() {}

  //
  constexpr virtual void on_click(MouseButton button, vec2 position,
                                  u32 nclicks, KeyModifiers modifiers) {}

  //
  constexpr virtual void on_double_click(MouseButton button, vec2 position,
                                         KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_scroll(vec2 previous_position,
                                         vec2 current_position,
                                         vec2 translation,
                                         KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_move(vec2 previous_position,
                                       vec2 current_position,
                                       KeyModifiers modifiers) {}

  //
  constexpr virtual void on_hover(vec2 position, KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_down(vec2 position, KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_up(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_enter(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_leave(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_out(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_mouse_over(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_enter(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_tap() {}

  //
  constexpr virtual void on_drag() {}

  //
  constexpr virtual void on_drag_start() {}

  //
  constexpr virtual void on_drag_end() {}

  //
  constexpr virtual void on_drag_enter() {}

  //
  constexpr virtual void on_drag_leave() {}

  //
  constexpr virtual void on_drag_over() {}

  //
  constexpr virtual void on_drop() {}

  //
  constexpr virtual void on_touch_cancel() {}

  //
  constexpr virtual void on_touch_end() {}

  //
  constexpr virtual void on_touch_move() {}

  //
  constexpr virtual void on_touch_start() {}

  //
  constexpr virtual void on_touch_enter() {}

  //
  constexpr virtual void on_touch_leave() {}

  //
  constexpr virtual void on_focus() {}

  //
  constexpr virtual void on_focus_in() {}

  //
  constexpr virtual void on_focus_out() {}

  //
  constexpr virtual void on_select() {}

  // TODO(lamarrr): can this be simpler?
  // virtual void raise_tooltip(){}
  // virtual void lower_tooltip(){}

  // virtual void accessibility_navigate(){}
  // virtual void accessibility_info(){}

  // TODO(lamarrr): we need a widget build tree
  //
  virtual simdjson::dom::element save(simdjson::dom::parser &parser) {
    return parser.parse("{}", 2);
  }

  //
  virtual void restore(simdjson::dom::element const &element) {}

  // position of the widget on the viewport. typically calculated on every
  // frame.
  rect area;
  u64 id = 0;
};

struct WidgetImpl {
  template <typename DerivedWidget>
  constexpr WidgetImpl(DerivedWidget widget)
      : impl{new DerivedWidget{std::move(widget)}} {
    static_assert(std::is_base_of_v<Widget, DerivedWidget>);
  }

  constexpr WidgetImpl() {}

  static constexpr WidgetImpl make(Widget *impl) {
    WidgetImpl w;
    w.impl = impl;
    return w;
  }

  constexpr WidgetImpl(WidgetImpl const &) = default;

  constexpr WidgetImpl(WidgetImpl &&) = default;

  constexpr WidgetImpl &operator=(WidgetImpl const &) = default;

  constexpr WidgetImpl &operator=(WidgetImpl &&) = default;

  constexpr ~WidgetImpl() = default;

  constexpr Widget *operator->() const { return impl; }

  constexpr operator Widget *() const { return impl; }

  Widget *impl = nullptr;
};

}  // namespace ash
