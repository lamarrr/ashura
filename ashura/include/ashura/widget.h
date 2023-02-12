#pragma once
#include <chrono>
#include <type_traits>
#include <utility>

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

enum class Visibility : u8 { Shown, Hidden };

enum class Direction : u8 { Row, Column };

enum class Wrap : u8 { None, Wrap };

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

struct layout {
  constraint x, y, width, height;
};

struct WidgetInfo {
  std::string_view id;
  std::string_view type;
};

struct WidgetContext {};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct WidgetImpl;

/// SEE: (https://www.w3.org/TR/uievents)
//
// TODO(lamarrr): should we go recursive?
//
struct Widget {
  constexpr Widget() {}

  STX_MAKE_PINNED(Widget)

  constexpr virtual ~Widget() {}

  //
  constexpr virtual stx::Span<WidgetImpl const> get_children() { return {}; }

  //
  constexpr virtual WidgetInfo get_debug_info() { return {}; }

  //
  constexpr virtual Visibility get_visibility() { return Visibility::Shown; }

  //
  constexpr virtual stx::Option<i64> get_z_index() { return stx::None; }

  //
  constexpr virtual ash::layout layout(rect area) { return {}; }

  // children layout??? or do we entirely allow self layout
  // + other properties that will make its rendering work well with other
  // widgets
  constexpr virtual void draw(gfx::Canvas &canvas, rect area) {
    (void)canvas;
    (void)area;
  }

  // called before children are drawn
  constexpr virtual void pre_draw(gfx::Canvas &canvas, rect area, usize child) {
    (void)canvas;
    (void)area;
    (void)child;
  }

  // called once children are drawn
  constexpr virtual void post_draw(gfx::Canvas &canvas, rect area,
                                   usize child) {
    (void)canvas;
    (void)area;
    (void)child;
  }

  //
  constexpr virtual void tick(WidgetContext &context,
                              std::chrono::nanoseconds interval) {
    (void)context;
    (void)interval;
  }

  //
  constexpr virtual void on_launch() {}

  //
  constexpr virtual void on_enter_viewport() {}

  //
  constexpr virtual void on_leave_viewport() {}

  //
  constexpr virtual void on_click(MouseButton button, KeyModifiers modifiers,
                                  vec2 position) {}

  //
  constexpr virtual void on_double_click(MouseButton button,
                                         KeyModifiers modifiers,
                                         vec2 position) {}

  //
  constexpr virtual void on_mouse_scroll(KeyModifiers modifiers,
                                         vec2 translation,
                                         vec2 previous_position,
                                         vec2 current_position) {}

  //
  constexpr virtual void on_mouse_move(KeyModifiers modifiers,
                                       vec2 previous_position,
                                       vec2 current_position) {}

  //
  constexpr virtual void on_hover(KeyModifiers modifiers, vec2 position) {}

  //
 constexpr virtual void on_mouse_down(KeyModifiers modifiers, vec2 position) {}

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

  //
  constexpr virtual void on_key_down(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_key_up(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_keypressed(KeyModifiers modifiers) {}

  //
  constexpr virtual void on_input(/*data*/) {}  // <- input widget

  // TODO(lamarrr): can this be simpler?
  // virtual void raise_tooltip(){}
  // virtual void lower_tooltip(){}

  // virtual void accessibility_navigate(){}
  // virtual void accessibility_info(){}

  //
  virtual simdjson::dom::element save(simdjson::dom::parser &parser) {
    return parser.parse("{}", 2);
  }

  //
  virtual void restore(simdjson::dom::element const &element) { (void)element; }
};

struct WidgetImpl {
  constexpr WidgetImpl(Widget *widget) : impl{widget} {}

  template <typename DerivedWidget,
            STX_ENABLE_IF(!std::is_pointer_v<DerivedWidget>)>
  constexpr WidgetImpl(DerivedWidget widget)
      : WidgetImpl{new DerivedWidget{std::move(widget)}} {}

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
