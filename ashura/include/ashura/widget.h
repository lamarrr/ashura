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

struct WidgetInfo {
  std::string_view id;
  std::string_view type;
};

struct WidgetContext {};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct WidgetImpl;

/// SEE: (https://www.w3.org/TR/uievents)
struct Widget {
  virtual ~Widget() {}
  //
  virtual stx::Span<WidgetImpl const> get_children() { return {}; }

  //
  virtual WidgetInfo get_debug_info() { return {}; }

  //
  virtual Visibility get_visibility() { return Visibility::Shown; }

  //
  virtual stx::Option<i64> get_z_index() { return stx::None; }

  // + handling floating, relative, sticky and fixed positioned elements
  // min, max, available
  virtual rect layout(rect target, stx::Span<rect> children_allocation) {
    (void)target;
    (void)children_allocation;
    return {};
  }

  // children layout??? or do we entirely allow self layout
  // + other properties that will make its rendering work well with other
  // widgets
  virtual void draw(gfx::Canvas &canvas, rect area) {
    (void)canvas;
    (void)area;
  }

  // called before children are drawn
  virtual void pre_effect(gfx::Canvas &canvas, rect area, usize child) {
    (void)canvas;
    (void)area;
    (void)child;
  }

  // called once children are drawn
  virtual void post_effect(gfx::Canvas &canvas, rect area, usize child) {
    (void)canvas;
    (void)area;
    (void)child;
  }

  //
  virtual void tick(WidgetContext &context, std::chrono::nanoseconds interval) {
    (void)context;
    (void)interval;
  }

  //
  virtual void on_launch() {}

  //
  virtual void on_enter_viewport() {}

  //
  virtual void on_leave_viewport() {}

  //
  virtual void on_click(MouseButton button, KeyModifiers modifiers,
                        vec2 position) {}

  //
  virtual void on_double_click(MouseButton button, KeyModifiers modifiers,
                               vec2 position) {}

  //
  virtual void on_mouse_scroll(KeyModifiers modifiers, vec2 translation,
                               vec2 previous_position, vec2 current_position) {}

  //
  virtual void on_mouse_move(KeyModifiers modifiers, vec2 previous_position,
                             vec2 current_position) {}

  //
  virtual void on_hover(KeyModifiers modifiers, vec2 position) {}

  //
  virtual void on_mouse_down(KeyModifiers modifiers, vec2 position) {}

  //
  virtual void on_mouse_up(KeyModifiers modifiers) {}

  //
  virtual void on_mouse_enter(KeyModifiers modifiers) {}

  //
  virtual void on_mouse_leave(KeyModifiers modifiers) {}

  //
  virtual void on_mouse_out(KeyModifiers modifiers) {}

  //
  virtual void on_mouse_over(KeyModifiers modifiers) {}

  //
  virtual void on_enter(KeyModifiers modifiers) {}

  //
  virtual void on_tap() {}

  //
  virtual void on_drag() {}

  //
  virtual void on_drag_start() {}

  //
  virtual void on_drag_end() {}

  //
  virtual void on_drag_enter() {}

  //
  virtual void on_drag_leave() {}

  //
  virtual void on_drag_over() {}

  //
  virtual void on_drop() {}

  //
  virtual void on_touch_cancel() {}

  //
  virtual void on_touch_end() {}

  //
  virtual void on_touch_move() {}

  //
  virtual void on_touch_start() {}

  //
  virtual void on_touch_enter() {}

  //
  virtual void on_touch_leave() {}

  //
  virtual void on_focus() {}

  //
  virtual void on_focus_in() {}

  //
  virtual void on_focus_out() {}

  //
  virtual void on_select() {}

  //
  virtual void on_key_down(KeyModifiers modifiers) {}

  //
  virtual void on_key_up(KeyModifiers modifiers) {}

  //
  virtual void on_keypressed(KeyModifiers modifiers) {}

  //
  virtual void on_input(/*data*/) {}  // <- input widget

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
