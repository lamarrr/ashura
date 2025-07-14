/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/engine/views/button.h"
#include "ashura/engine/views/flex.h"
#include "ashura/engine/views/input.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

using Scalar = Enum<f32, i32>;

/// @param start starting value, this is the value to be reset to when cancel is
/// requested
/// @param min minimum value of the scalar
/// @param max maximum value of the scalar
/// @param step step in either direction that should be taken. i.e. when `+` or
/// `-` is pressed.
/// @param current current value of the scalar, mutated by the GUI system
struct F32Info
{
  f32 base = 0;

  f32 min = 0;

  f32 max = 1;

  f32 step = 0.05F;

  constexpr f32 step_value(f32 current, f32 direction) const
  {
    return clamp(current + direction * step, min, max);
  }

  constexpr f32 uninterp(f32 current) const
  {
    return clamp(unlerp(min, max, current), 0.0F, 1.0F);
  }

  constexpr f32 interp(f32 t) const
  {
    return clamp(lerp(min, max, t), min, max);
  }
};

/// @param start starting value, this is the value to be reset to when cancel is
/// requested
/// @param min minimum value of the scalar
/// @param max maximum value of the scalar
/// @param step step in either direction that should be taken. i.e. when `+` or
/// `-` is pressed.
/// @param current current value of the scalar, mutated by the GUI system
struct I32Info
{
  i32 base = 0;

  i32 min = 0;

  i32 max = 1'000;

  i32 step = 100;

  constexpr i32 step_value(i32 current, f32 direction) const
  {
    return clamp((i32) (current + direction * step), min, max);
  }

  constexpr f32 uninterp(i32 current) const
  {
    return clamp(unlerp((f32) min, (f32) max, (f32) current), 0.0F, 1.0F);
  }

  constexpr i32 interp(f32 t) const
  {
    return clamp((i32) lerp((f32) min, (f32) max, t), min, max);
  }
};

using ScalarInfo = Enum<F32Info, I32Info>;

}    // namespace ui

inline void format(fmt::Sink sink, fmt::Spec spec, ui::Scalar const & value)
{
  return value.match([&](f32 f) { return format(sink, spec, f); },
                     [&](i32 i) { return format(sink, spec, i); });
}

namespace ui
{

// [ ] on-focused go to input mode
// [ ] fix alt+click
struct ScalarDragBox : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool input_mode : 1 = false;

    bool dragging : 1 = false;

    ScalarInfo spec = F32Info{};

    Scalar scalar = 0.0F;
  } state_;

  struct Style
  {
    Frame frame = Frame{}.min(200, theme.body_font_height);

    Padding padding = Padding::all(2.5F);

    CornerRadii corner_radii = CornerRadii::all(2);

    u8x4 color = theme.inactive;

    u8x4 thumb_color = theme.inactive;

    f32 stroke = 1.0F;

    f32 thickness = 0.5F;

    Str format = "{}"_str;
  } style_;

  Input input_;

  struct Callbacks
  {
    Fn<void(Scalar)> update = noop;
  } cb;

  ScalarDragBox(
    TextStyle const & style     = TextStyle{.color = theme.on_surface},
    FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                            .height      = theme.body_font_height,
                                            .line_height = theme.line_height},
    AllocatorRef      allocator = default_allocator);

  ScalarDragBox(ScalarDragBox const &)             = delete;
  ScalarDragBox(ScalarDragBox &&)                  = default;
  ScalarDragBox & operator=(ScalarDragBox const &) = delete;
  ScalarDragBox & operator=(ScalarDragBox &&)      = default;
  virtual ~ScalarDragBox() override                = default;

  static void scalar_parse(Str32 text, ScalarInfo const & spec, Scalar &);

  void format_();

  ScalarDragBox & on_update(Fn<void(Scalar)> cb);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(f32x2 extent, f32x2 offset) override;
};

// [ ] spacing / margin
struct ScalarBox : Flex
{
  struct Callbacks
  {
    Fn<void(Scalar)> update = noop;
  } cb;

  TextButton dec_;

  TextButton inc_;

  ScalarDragBox drag_;

  ScalarBox(
    Str32 decrease_text = U"minus"_str, Str32 increase_text = U"plus"_str,
    TextStyle const & button_text_style = TextStyle{.color = theme.on_primary},
    TextStyle const & drag_text_style   = TextStyle{.color = theme.on_primary},
    FontStyle const & icon_font         = FontStyle{.font   = theme.icon_font,
                                                    .height = theme.body_font_height,
                                                    .line_height = theme.line_height},
    FontStyle const & text_font         = FontStyle{.font   = theme.body_font,
                                                    .height = theme.body_font_height,
                                                    .line_height = theme.line_height},
    AllocatorRef      allocator         = default_allocator);

  ScalarBox(ScalarBox const &)             = delete;
  ScalarBox(ScalarBox &&)                  = default;
  ScalarBox & operator=(ScalarBox const &) = delete;
  ScalarBox & operator=(ScalarBox &&)      = default;
  virtual ~ScalarBox() override            = default;

  ScalarBox & step(i32 direction);

  ScalarBox & stub(Str32 text);

  ScalarBox & stub(Str8 text);

  ScalarBox & format(Str format);

  ScalarBox & spec(f32 scalar, F32Info info);

  ScalarBox & spec(i32 scalar, I32Info info);

  ScalarBox & stroke(f32 stroke);

  ScalarBox & thickness(f32 thickness);

  ScalarBox & padding(Padding padding);

  ScalarBox & frame(Frame frame);

  ScalarBox & corner_radii(CornerRadii const & radii);

  ScalarBox & on_update(Fn<void(Scalar)> fn);

  ScalarBox & button_text_style(TextStyle const & style, FontStyle const & font,
                                usize first = 0, usize count = USIZE_MAX);

  ScalarBox & drag_text_style(TextStyle const & style, FontStyle const & font,
                              usize first = 0, usize count = USIZE_MAX);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;
};

}    // namespace ui

}    // namespace ash
