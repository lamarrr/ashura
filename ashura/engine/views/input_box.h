/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/engine.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/engine/views/button.h"
#include "ashura/engine/views/scroll_box.h"
#include "ashura/std/text.h"

namespace ash
{

enum class ScalarInputType : u8
{
  i32 = 0,
  f32 = 1
};

/// @brief Numeric Scalar UI Input.
struct ScalarInput
{
  union
  {
    i32 i32;
    f32 f32;
  };
  ScalarInputType type = ScalarInputType::i32;
};

namespace fmt
{

inline bool push(Context const &ctx, Spec const &spec, ScalarInput const &value)
{
  switch (value.type)
  {
    case ScalarInputType::i32:
      return push(ctx, spec, value.i32);
    case ScalarInputType::f32:
      return push(ctx, spec, value.f32);
    default:
      return true;
  }
}

}        // namespace fmt

/// @param start starting value, this is the value to be reset to when cancel is
/// requested
/// @param min minimum value of the scalar
/// @param max maximum value of the scalar
/// @param step step in either direction that should be taken. i.e. when `+` or
/// `-` is pressed.
/// @param current current value of the scalar, mutated by the GUI system
struct ScalarState
{
  ScalarInput base    = {};
  ScalarInput min     = {};
  ScalarInput max     = {};
  ScalarInput step    = {};
  ScalarInput current = {};

  constexpr void step_value(bool direction)
  {
    switch (base.type)
    {
      case ScalarInputType::i32:
        current.i32 =
            clamp(sat_add(current.i32, direction ? step.i32 : -step.i32),
                  min.i32, max.i32);
        return;
      case ScalarInputType::f32:
        current.f32 = clamp(current.f32 + (direction ? step.f32 : -step.f32),
                            min.f32, max.f32);
        return;
      default:
        return;
    }
  }

  constexpr f32 uninterp() const
  {
    switch (base.type)
    {
      case ScalarInputType::i32:
        return clamp(unlerp(min.i32, max.i32, current.i32), 0.0F, 1.0F);
      case ScalarInputType::f32:
        return clamp(unlerp(min.f32, max.f32, current.f32), 0.0F, 1.0F);
      default:
        return 0;
    }
  }

  constexpr void interp(f32 t)
  {
    switch (base.type)
    {
      case ScalarInputType::i32:
        current.i32 = clamp((i32) lerp((f32) min.i32, (f32) max.i32, t),
                            min.i32, max.i32);
        return;
      case ScalarInputType::f32:
        current.f32 = clamp(lerp(min.f32, max.f32, t), min.f32, max.f32);
        return;
      default:
        break;
    }
  }
};

constexpr ScalarState scalar(f32 base, f32 min, f32 max, f32 step)
{
  return ScalarState{.base = {.f32 = base, .type = ScalarInputType::i32},
                     .min  = {.f32 = min, .type = ScalarInputType::i32},
                     .max  = {.f32 = max, .type = ScalarInputType::i32},
                     .step = {.f32 = step, .type = ScalarInputType::i32}};
}

constexpr ScalarState scalar(i32 base, i32 min, i32 max, i32 step)
{
  return ScalarState{.base = {.i32 = base, .type = ScalarInputType::i32},
                     .min  = {.i32 = min, .type = ScalarInputType::i32},
                     .max  = {.i32 = max, .type = ScalarInputType::i32},
                     .step = {.i32 = step, .type = ScalarInputType::i32}};
}

struct ScalarDragBox : View, Pin<>
{
  typedef Fn<void(fmt::Context const &, ScalarInput)> Fmt;
  typedef Fn<void(Span<u32 const>, ScalarState &)>    Parse;

  bool                  disabled : 1   = false;
  bool                  input_mode : 1 = false;
  bool                  dragging : 1   = false;
  ScalarState           value          = {};
  Fmt                   fmt            = fn(scalar_fmt);
  Parse                 parse          = fn(scalar_parse);
  Fn<void(ScalarState)> on_update      = fn([](ScalarState) {});
  Frame                 frame          = {.width = {100}, .height = {20}};
  f32                   padding        = 5;
  SizeConstraint        thumb_width    = {2.75F};
  ColorGradient         border_color   = DEFAULT_THEME.inactive;
  ColorGradient         thumb_color    = DEFAULT_THEME.inactive;
  TextInput             input          = {};

  ScalarDragBox()
  {
    input.is_multiline  = false;
    input.tab_input     = false;
    input.enter_submits = false;
    input.placeholder.set_text(
        U""_utf, TextStyle{.foreground = DEFAULT_THEME.inactive},
        FontStyle{.font = engine.default_font, .font_height = 10});
    input.content.set_text(U""_utf, TextStyle{.foreground = DEFAULT_THEME.text},
                           {.font = engine.default_font, .font_height = 10});
  }

  static void scalar_fmt(fmt::Context const &ctx, ScalarInput v)
  {
    fmt::format(ctx, v);
  }

  static void scalar_parse(Span<u32 const> text, ScalarState &s)
  {
    if (text.is_empty())
    {
      return;
    }

    Vec<u8> utf8;
    utf8_encode(text, utf8).unwrap();

    char *first = (char *) utf8.begin();
    char *last  = (char *) utf8.end();

    switch (s.base.type)
    {
      case ScalarInputType::i32:
      {
        i32 value      = 0;
        auto [ptr, ec] = std::from_chars(first, last, value);
        if (ec != std::errc{} || value < s.min.i32 || value > s.max.i32)
        {
          return;
        }
        s.current = ScalarInput{.i32 = value, .type = ScalarInputType::i32};
      }
      break;

      case ScalarInputType::f32:
      {
        f32 value = 0;
        auto [ptr, ec] =
            std::from_chars(first, last, value, std::chars_format::fixed);
        if (ec != std::errc{} || value < s.min.f32 || value > s.max.f32)
        {
          return;
        }
        s.current = ScalarInput{.f32 = value, .type = ScalarInputType::f32};
      }
      break;
    }
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    dragging = false;

    if (!disabled && events.drag_update)
    {
      f32 const t =
          clamp(unlerp(region.begin().x, region.end().x, ctx.mouse_position.x),
                0.0F, 1.0F);
      value.interp(t);
      dragging = true;
    }

    if (input.editing)
    {
      parse(input.content.get_text(), value);
    }
    else
    {
      char         scratch[128];
      char         text[128];
      fmt::Buffer  buffer{.buffer = span(text), .pos = 0};
      fmt::Context ctx = fmt::buffer(&buffer, span(scratch));
      fmt(ctx, value.current);
      input.content.set_text(span(text).slice(0, buffer.pos).as_u8(), {}, {});
    }

    input_mode =
        !disabled && (!input.focus_out ||
                      (events.mouse_down && (ctx.key_down(KeyCode::LCtrl) ||
                                             ctx.key_down(KeyCode::RCtrl))));

    input.disabled = input_mode;

    if (input.editing || dragging)
    {
      on_update(value);
    }

    return ViewState{.draggable = !input.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) const override
  {
    Vec2 child = frame(allocated) - padding;
    child.x    = max(child.x, 0.0F);
    child.y    = max(child.y, 0.0F);
    fill(sizes, child);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) const override
  {
    fill(offsets, Vec2{0, 0});
    return sizes[0] + padding;
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) const override
  {
    canvas.rrect(
        ShapeDesc{.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = Vec4::splat(region.extent.y * 0.125F),
                  .stroke       = 1,
                  .thickness    = 1.0f,
                  .tint         = border_color});

    f32 const  t = value.uninterp();
    Vec2 const thumb_extent{thumb_width(region.extent.x), region.extent.y};
    Vec2       thumb_center{0, region.center.y};

    thumb_center.x =
        space_align(region.extent.x, thumb_extent.x, norm_to_axis(t));

    canvas.rrect({.center       = thumb_center,
                  .extent       = thumb_extent,
                  .corner_radii = Vec4::splat(region.extent.y * 0.125F),
                  .tint         = thumb_color});
  }

  virtual Cursor cursor(CRect const &region, Vec2 offset) const override
  {
    (void) region;
    (void) offset;
    return disabled ? Cursor::Default : Cursor::EWResize;
  }
};

struct ScalarBox : View, Pin<>
{
  bool                  disabled : 1   = false;
  bool                  steppable : 1  = true;
  bool                  draggable : 1  = true;
  bool                  inputtable : 1 = true;
  ScalarInput           step           = {};
  TextButton            decr           = {};
  TextButton            incr           = {};
  ScalarDragBox         dragger        = {};
  Fn<void(ScalarInput)> on_changed     = fn([](ScalarInput) {});

  ScalarBox()
  {
    decr.text.text.set_text(U"-"_utf, {}, {});
    incr.text.text.set_text(U"+"_utf, {}, {});

    decr.on_clicked =
        fn(this, [](ScalarBox *b) { b->dragger.value.step_value(false); });

    incr.on_clicked =
        fn(this, [](ScalarBox *b) { b->dragger.value.step_value(true); });
  }
};

/// REQUIREMENTS:
/// [ ] Generic Numeric Input: Scalars, Vectors, Matrices, Tensors
template <u32 N>
struct VectorInputBox : View
{
  ScalarBox scalars[N];
};

// struct ScrollableTextInput : ScrollBox, Pin<>
// {
//   TextInput input;
//   ScrollableTextInput()          = default;
//   virtual ~ScrollableTextInput() = default;
//   // [ ] calculate lines per page
//   // [ ] viewport text with scrollable region, scroll direction
//   // [ ] text input while in view, i.e. page down
// };

}        // namespace ash
