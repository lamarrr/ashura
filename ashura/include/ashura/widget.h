#pragma once
#include <chrono>

#include "ashura/event.h"
#include "ashura/primitives.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/string.h"

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
  virtual RectF layout(vec2 allotted_extent);  // min, max, available

  // children layout??? or do we entirely allow self layout
  // + other properties that will make its rendering work well with other
  // widgets
  virtual void draw(vec2 offset, vec2 extent);

  // called before children are drawn
  virtual void pre_effect(vec2 offset, vec2 extent);

  // called once children are drawn
  virtual void post_effect(vec2 offset, vec2 extent);

  virtual void tick(std::chrono::nanoseconds interval, Context &ctx);

  /*=== EVENTS ===*/

  virtual void on_enter_view();
  virtual void on_leave_view();

  virtual void on_click(MouseButton btn, vec2 pos);
  virtual void on_double_click(MouseButton btn, vec2 pos);
  virtual void on_mouse_scroll(vec2 translation, vec2 x, vec2 y);
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
  virtual void on_drag_enter();
  virtual void on_drag_leave();
  virtual void on_drag_over();
  virtual void on_drop();
  virtual void on_wheel();

  virtual void on_touch_cancel();
  virtual void on_touch_end();
  virtual void on_touch_move();
  virtual void on_touch_start();
  virtual void on_touch_enter();
  virtual void on_touch_leave();

  virtual void on_key_down();
  virtual void on_key_up();
  virtual void on_keypressed();

  virtual void on_focus();
  virtual void on_focus_in();
  virtual void on_focus_out();

  virtual void on_select();

  virtual void on_copy();
  virtual void on_cut();
  virtual void on_paste();

  // scroll of this widget's content
  virtual void on_scroll(vec2 translation, vec2 y);

  virtual void on_full_screen_change();

  virtual void on_input();  // <- input widget

  virtual void on_resize();

  // TODO(lamarrr): can this be simpler?
  virtual stx::String tooltip();

  virtual void accessibility_navigate();
  virtual void accessibility_info();

  virtual JsonObject save();
  virtual void restore(JsonObject const &);
};

struct GlobalEvent {
  void on_mouse_click();
  void on_key();
  void on_close_requested();
};

}  // namespace asr