/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/engine/views/button.h"
#include "ashura/engine/views/scroll_box.h"
#include "ashura/std/text.h"

namespace ash
{

enum class ScalarInputType : u8
{
  u8  = 0,
  u16 = 1,
  u32 = 2,
  i8  = 5,
  i16 = 6,
  i32 = 7,
  f32 = 10
};

/// @brief Numeric Scalar Text Input.
/// 64-bit precision is not supported.
struct ScalarInput
{
  union
  {
    u8  u8 = 0;
    u16 u16;
    u32 u32;
    i8  i8;
    i16 i16;
    i32 i32;
    f32 f32;
  };
  ScalarInputType type = ScalarInputType::u8;
};

namespace fmt
{

inline bool push(Context const &ctx, Spec const &spec, ScalarInput const &value)
{
  switch (value.type)
  {
    case ScalarInputType::u8:
      return push(ctx, spec, value.u8);
    case ScalarInputType::u16:
      return push(ctx, spec, value.u16);
    case ScalarInputType::u32:
      return push(ctx, spec, value.u32);
    case ScalarInputType::i8:
      return push(ctx, spec, value.i8);
    case ScalarInputType::i16:
      return push(ctx, spec, value.i16);
    case ScalarInputType::i32:
      return push(ctx, spec, value.i32);
    case ScalarInputType::f32:
      return push(ctx, spec, value.f32);
    default:
      return true;
  }
}

}        // namespace fmt

enum class AxisScale : u8
{
  Linear = 0,
  Log10  = 1,
  Log2   = 2,
};

/// @param start starting value, this is the value to be reset to when cancel is
/// requested
/// @param min minimum value of the scalar
/// @param max maximum value of the scalar
/// @param current current value of the scalar, mutated by the GUI system
struct ScalarState
{
  ScalarInput         base    = {};
  ScalarInput         min     = {};
  ScalarInput         max     = {};
  AxisScale           scale   = AxisScale::Linear;
  mutable ScalarInput current = {};

  template <typename T>
  static constexpr void log10_step()
  {
  }

  constexpr void step(i32 direction, ScalarInput value) const
  {
    /*   switch (v.type)
     {
       case ScalarInputType::u8:
         v.u8 = (u8) ash::clamp((i64) v.u32 +
                                    (i64) (direction ? step.u32 : -step.u32),
                                (i64) min.u32, (i64) max.u32);
         return;
       case ScalarInputType::u16:
         v.u16 = (u16) ash::clamp((i64) v.u32 +
                                      (i64) (direction ? step.u32 : -step.u32),
                                  (i64) min.u32, (i64) max.u32);
         return;
       case ScalarInputType::u32:
         v.u32 = (u32) ash::clamp((i64) v.u32 +
                                      (i64) (direction ? step.u32 : -step.u32),
                                  (i64) min.u32, (i64) max.u32);
         return;
       case ScalarInputType::i8:
         v.i8 = (i8) ash::clamp((i64) v.i32 +
                                    (i64) (direction ? step.i32 : -step.i32),
                                (i64) min.i32, (i64) max.i32);
         return;
       case ScalarInputType::i16:
         v.i16 = (i16) ash::clamp((i64) v.i32 +
                                      (i64) (direction ? step.i32 : -step.i32),
                                  (i64) min.i32, (i64) max.i32);
         return;
       case ScalarInputType::i32:
         v.i32 = (i32) ash::clamp((i64) v.i32 +
                                      (i64) (direction ? step.i32 : -step.i32),
                                  (i64) min.i32, (i64) max.i32);
         return;
       case ScalarInputType::f32:
         v.f32 = (f32) ash::clamp((f64) v.f32 +
                                      (f64) (direction ? step.f32 : -step.f32),
                                  (f64) min.f32, (f64) max.f32);
         return;
       default:
         return;
     }*/
  }

  constexpr f32 uninterp() const;

  constexpr void interp(f32 t) const
  {
    /*     switch (a.type)
         {
           case ScalarInputType::u8:
             return ScalarInput{.u8   = clamp((u8) lerp(a.u8, b.u8, t), a.u8,
       b.u8), .type = ScalarInputType::u8}; case ScalarInputType::u16: return
       ScalarInput{.u16 = clamp((u16) lerp(a.u16, b.u16, t), a.u16, b.u16),
       .type = ScalarInputType::u16}; case ScalarInputType::u32: return
       ScalarInput{.u32 = clamp((u32) lerp(a.u32, b.u32, t), a.u32, b.u32),
       .type = ScalarInputType::u32}; case ScalarInputType::i8: return
       ScalarInput{.i8   = clamp((i8) lerp(a.i8, b.i8, t), a.i8, b.i8), .type =
       ScalarInputType::i8}; case ScalarInputType::i16: return ScalarInput{.i16
       = clamp((i16) lerp(a.i16, b.i16, t), a.i16, b.i16), .type =
       ScalarInputType::i16}; case ScalarInputType::i32: return ScalarInput{.i32
       = clamp((i32) lerp(a.i32, b.i32, t), a.i32, b.i32), .type =
       ScalarInputType::i32}; case ScalarInputType::f32: return ScalarInput{.f32
       = clamp((f32) lerp(a.f32, b.f32, t), a.f32, b.f32), .type =
       ScalarInputType::f32}; default: break;
         }*/
  }
};

struct ScalarDragBox : View, Pin<void>
{
  typedef Fn<void(fmt::Context &, ScalarInput)> Fmt;

  bool               disabled : 1   = false;
  bool               input_mode : 1 = false;
  bool               dragging : 1   = false;
  ScalarState const *scalar         = nullptr;
  Fmt                fmt            = fn(default_fmt);
  Fn<void(f32)>      on_dragged     = fn([](f32) {});
  Frame     frame   = {.width = {.offset = 100}, .height = {.offset = 20}};
  f32       padding = 5;
  TextInput input   = {};

  ScalarDragBox()
  {
    input.is_multiline  = false;
    input.tab_input     = false;
    input.enter_submits = false;
  }

  static void default_fmt(fmt::Context &ctx, ScalarInput v)
  {
    fmt::format(ctx, v);
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    dragging = false;

    f32 t = 0;

    if (!disabled && events.drag_update)
    {
      t = clamp(unlerp(region.begin().x, region.end().x, ctx.mouse_position.x),
                0.0F, 1.0F);
      dragging = true;
    }

    if (input.editing)
    {
      // [ ] update value
      // [ ] read text input and parse
      // [ ] action on clear
      // [ ] action on edit
      // [ ] placeholder or always set to value
      // [ ] what value to set input field to
      // [ ] input.content.text;
    }
    else
    {
      // update text content
      input.flush_text();
    }

    input_mode =
        !disabled && (!input.focus_out ||
                      (events.mouse_down && (ctx.key_down(KeyCode::LCtrl) ||
                                             ctx.key_down(KeyCode::RCtrl))));

    input.disabled = input_mode;

    if (dragging)
    {
      on_dragged(t);
    }

    return ViewState{.draggable = !input.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) const
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

  virtual void render(CRect const &region, Canvas &canvas) const override
  {
    canvas.rrect(
        ShapeDesc{.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = Vec4::splat(region.extent.y * 0.125),
                  .thickness    = 1.0f,
                  .stroke       = 1,
                  .tint = ColorGradient::y(colors::GREEN, colors::BLUE)});
    // slider
    canvas.rrect({});
  }
};

/// @brief
/// REQUIREMENTS:
/// Custom Scaling & Custom Stepping: i.e. log, linear, Angular Input
/// Drag-Based Input
/// Text-Field Input of exact values
/// Generic Numeric Input: Scalars, Vectors, Matrices, Tensors
struct ScalarBox : View, Pin<void>
{
  bool                  disabled : 1   = false;
  bool                  steppable : 1  = true;
  bool                  draggable : 1  = true;
  bool                  inputtable : 1 = true;
  ScalarState const    *scalar         = nullptr;
  ScalarInput           step           = {};
  TextButton            decr           = {};
  TextButton            incr           = {};
  ScalarDragBox         dragger        = {};
  Fn<void(ScalarInput)> on_changed     = fn([](ScalarInput) {});

  ScalarBox()
  {
    // decr.text.block.text = utf(U"-"_span);
    // incr.text.block.text = utf(U"+"_span);

    decr.on_clicked = fn(this, [](ScalarBox *b) {
      if (b->scalar != nullptr)
      {
        b->scalar->step(-1, b->step);
        b->on_changed(b->scalar->current);
      }
    });

    incr.on_clicked = fn(this, [](ScalarBox *b) {
      if (b->scalar != nullptr)
      {
        b->scalar->step(1, b->step);
        b->on_changed(b->scalar->current);
      }
    });

    dragger.on_dragged = fn(this, [](ScalarBox *b, f32 t) {
      if (b->scalar != nullptr)
      {
        // synchronize values
        b->scalar->interp(t);
      }
    });
  }
};

template <u32 N>
struct VectorInputBox : View
{
  ScalarBox scalars[N];
};

// struct ScrollableTextInput : ScrollBox, Pin<void>
// {
//   TextInput input;
//   ScrollableTextInput()          = default;
//   virtual ~ScrollableTextInput() = default;
//   // [ ] calculate lines per page
//   // [ ] viewport text with scrollable region, scroll direction
//   // [ ] text input while in view, i.e. page down
// };

}        // namespace ash
