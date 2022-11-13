#pragma once
#include <chrono>

#include "ashura/event.h"
#include "ashura/primitives.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"

namespace asr {

enum class Visibility : u8 { Shown, Hidden };

struct JsonObject {};

struct WidgetInfo {};

struct Context {};

// TODO(lamarrr): we need to pass in a zoom level to the rendering widget? so
// that widgets like text can shape their glyphs properly

struct Widget {
  virtual stx::Span<Widget *const> get_children();
  virtual WidgetInfo get_debug_info();
  virtual Visibility get_visibility();
  virtual stx::Option<i64> get_z_index();
  // + handling floating, relative, sticky and fixed positioned elements
  virtual Rect layout(Extent allotted_extent);  // min, max, available
  // children layout??? or do we entirely allow self layout
  // + other properties that will make its rendering work well with other
  // widgets
  virtual void draw();
  // called before children are drawn
  virtual void pre_effect();
  // called once children are drawn
  virtual void post_effect();

  virtual void tick(std::chrono::nanoseconds interval, Context &ctx);
  // events
  virtual void on_click(MouseButton btn, Offset pos);
  virtual void on_double_click(MouseButton btn, Offset pos);
  virtual void on_mouse_scroll(OffsetI translation, f32 x, f32 y);
  virtual void on_mouse_move();
  virtual void on_hover(Offset pos);
  virtual void on_mouse_down();
  virtual void on_mouse_up();
  virtual void on_mouse_enter();
  virtual void on_mouse_leave();
  virtual void on_mouse_out();
  virtual void on_mouse_over();
  virtual void on_enter();
  virtual void on_tap();
  virtual void on_drag();
  virtual void on_drag_start();
  virtual void on_drag_end();
  virtual void on_focus();
  virtual void on_focus_in();
  virtual void on_focus_out();
  // scroll of this widget's content
  virtual void on_scroll(OffsetI translation, f32 x, f32 y);
  virtual void on_enter_view();
  virtual void on_leave_view();
  // virtual void on_full_screen_change();
  // virtual void on_keydown();
  // virtual void on_keyup();
  // virtual void on_input(); - input widget
  virtual void tooltip();
  virtual void accessibility_navigate();
  virtual void accessibility_info();
  // bind to keyboard
  // virtual void on_keyboard();
  // state saving - just bytes, left to the widget to decide how to save and
  // restore state
  virtual JsonObject save();
  virtual void restore(JsonObject const &);

  stx::UniqueFn<void()> mark_needs_redraw =
      stx::fn::rc::make_unique_static([]() {});
  stx::UniqueFn<void()> mark_needs_relayout =
      stx::fn::rc::make_unique_static([]() {});
  stx::UniqueFn<void()> mark_children_changed =
      stx::fn::rc::make_unique_static([]() {});
};

struct GlobalEvent {
  void on_mouse_click();
  void on_key();
  void on_shutdown_requested();
};

}  // namespace asr