/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/engine.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/engine/views/button.h"
#include "ashura/engine/views/flex_view.h"
#include "ashura/engine/views/scroll_view.h"
#include "ashura/std/text.h"
#include <charconv>

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

  constexpr void step_value(i32 direction)
  {
    switch (base.type)
    {
      case ScalarInputType::i32:
        current.i32 =
            clamp(sat_add(current.i32, (direction > 0) ? step.i32 : -step.i32),
                  min.i32, max.i32);
        return;
      case ScalarInputType::f32:
        current.f32 =
            clamp(current.f32 + ((direction > 0) ? step.f32 : -step.f32),
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

  struct State
  {
    bool        disabled : 1   = false;
    bool        input_mode : 1 = false;
    bool        dragging : 1   = false;
    ScalarState value          = {};
  } state;

  struct Style
  {
    Frame         frame        = {.width = {.min = 100},
                                  .height = {.min = DEFAULT_THEME.body_font_height + 10}};
    Frame         padding      = {.width = {5}, .height = {5}};
    Size          thumb_width  = {2.75F};
    CornerRadii   corner_radii = CornerRadii({.scale = 0.125F});
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient thumb_color  = ColorGradient::all(DEFAULT_THEME.inactive);
    f32           stroke       = 1.0F;
    f32           thickness    = 1.0F;
  } style;

  TextInput input = {};
  Fmt       fmt   = fn(scalar_fmt);
  Parse     parse = fn(scalar_parse);

  Fn<void(ScalarInput)> on_update = fn([](ScalarInput) {});

  ScalarDragBox()
  {
    input.state.is_multiline  = false;
    input.state.tab_input     = false;
    input.state.enter_submits = false;
  }

  virtual ~ScalarDragBox() override
  {
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
    defer utf8_reset{[&] { utf8.reset(); }};

    char const *const first = (char const *) utf8.begin();
    char const *const last  = (char const *) utf8.end();

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
                         ViewEvents events, Fn<void(View *)>) override
  {
    state.dragging = events.dragging;

    if (events.drag_start &&
        (ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)))
    {
      state.input_mode = !state.input_mode;
    }

    if (state.dragging && !state.input_mode)
    {
      f32 const t =
          clamp(unlerp(region.begin().x, region.end().x, ctx.mouse_position.x),
                0.0F, 1.0F);
      state.value.interp(t);
    }

    if (input.state.editing)
    {
      parse(input.content.get_text(), state.value);
    }
    else
    {
      char         scratch[128];
      char         text[128];
      fmt::Buffer  buffer{.buffer = span(text), .pos = 0};
      fmt::Context ctx = fmt::buffer(&buffer, span(scratch));
      fmt(ctx, state.value.current);
      input.content.set_text(span(text).slice(0, buffer.pos).as_u8());
    }

    input.state.disabled = !state.input_mode;

    if (input.state.editing || state.dragging)
    {
      on_update(state.value.current);
    }

    return ViewState{.pointable = !state.disabled,
                     .draggable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 child = style.frame(allocated) - 2 * style.padding(allocated);
    child.x    = max(child.x, 0.0F);
    child.y    = max(child.y, 0.0F);
    fill(sizes, child);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    fill(offsets, Vec2{0, 0});
    return sizes[0] + 2 * style.padding(allocated);
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .stroke       = style.stroke,
                           .thickness    = style.thickness,
                           .tint         = style.color});

    if (!state.input_mode)
    {
      f32 const  t = state.value.uninterp();
      Vec2 const thumb_extent{style.thumb_width(region.extent.x),
                              region.extent.y};
      Vec2       thumb_center{0, region.center.y};

      thumb_center.x =
          space_align(region.extent.x, thumb_extent.x, norm_to_axis(t));

      canvas.rrect(
          ShapeDesc{.center       = thumb_center,
                    .extent       = thumb_extent,
                    .corner_radii = Vec4::splat(region.extent.y * 0.125F),
                    .tint         = style.thumb_color});
    }
  }

  virtual Cursor cursor(CRect const &region, Vec2 offset) override
  {
    (void) region;
    (void) offset;
    return state.disabled ? Cursor::Default : Cursor::EWResize;
  }
};

struct ScalarBox : FlexView, Pin<>
{
  struct State
  {
    bool disabled : 1  = false;
    bool steppable : 1 = true;
    bool draggable : 1 = true;
  } state;

  Fn<void(ScalarInput)> on_update = fn([](ScalarInput) {});

  TextButton    dec  = {};
  TextButton    inc  = {};
  ScalarDragBox drag = {};

  ScalarBox()
  {
    FlexView::style.axis        = Axis::X;
    FlexView::style.wrap        = false;
    FlexView::style.reverse     = false;
    FlexView::style.main_align  = MainAlign::Start;
    FlexView::style.cross_align = 0;
    FlexView::style.frame =
        Frame{.width = {.scale = 1}, .height = {.scale = 1}};

    dec.text.text.set_text(
        U"-"_utf,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_primary)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});
    inc.text.text.set_text(
        U"+"_utf,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_primary)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});

    inc.style.stroke = dec.style.stroke = drag.style.stroke;
    inc.style.thickness = dec.style.thickness = drag.style.thickness;
    inc.style.padding = dec.style.padding = drag.style.padding;
    inc.style.frame.width = dec.style.frame.width = drag.style.frame.height;
    inc.style.frame.height = dec.style.frame.height = drag.style.frame.height;
    inc.style.corner_radii = dec.style.corner_radii = drag.style.corner_radii;

    dec.on_pressed = fn(this, [](ScalarBox *b) {
      b->drag.state.value.step_value(-1);
      b->on_update(b->drag.state.value.current);
    });

    inc.on_pressed = fn(this, [](ScalarBox *b) {
      b->drag.state.value.step_value(1);
      b->on_update(b->drag.state.value.current);
    });

    drag.on_update =
        fn(this, [](ScalarBox *b, ScalarInput in) { b->on_update(in); });
  }

  virtual ViewState tick(ViewContext const &, CRect const &, ViewEvents,
                         Fn<void(View *)> build) override
  {
    dec.state.disabled  = state.disabled || !state.steppable;
    inc.state.disabled  = state.disabled || !state.steppable;
    drag.state.disabled = state.disabled || !state.draggable;
    build(&dec);
    build(&drag);
    build(&inc);
    return ViewState{};
  }
};

}        // namespace ash
