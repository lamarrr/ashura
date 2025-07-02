/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/engine/views/text.h"
#include "ashura/std/types.h"

namespace ash
{
namespace ui
{

enum class ButtonShape : u8
{
  RRect    = 0,
  Squircle = 1,
  Bevel    = 2
};

struct Button : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool hovered : 1 = false;

    bool held : 1 = false;
  } state_;

  struct Style
  {
    Vec4U8 color = theme.primary;

    Vec4U8 hovered_color = theme.primary_variant;

    Vec4U8 disabled_color = theme.inactive;

    f32 stroke = 0.0F;

    f32 thickness = 1.0F;

    ButtonShape shape = ButtonShape::RRect;

    Padding padding = {};

    CornerRadii corner_radii = CornerRadii::all(2);

    Frame frame = Frame{}.rel(1, 1);
  } style_;

  struct Callbacks
  {
    Fn<void()> pressed = noop;
    Fn<void()> hovered = noop;
  } cb;

  Button()                           = default;
  Button(Button const &)             = delete;
  Button(Button &&)                  = default;
  Button & operator=(Button const &) = delete;
  Button & operator=(Button &&)      = default;
  virtual ~Button() override         = default;

  Button & disable(bool disable);

  Button & color(Vec4U8 color);

  Button & hovered_color(Vec4U8 color);

  Button & disabled_color(Vec4U8 color);

  Button & rrect(CornerRadii const & radii);

  Button & squircle(f32 degree = 5);

  Button & bevel(CornerRadii const & radii);

  Button & frame(Frame frame);

  Button & stroke(f32 stroke);

  Button & thickness(f32 thickness);

  Button & padding(Padding padding);

  Button & on_pressed(Fn<void()> fn);

  Button & on_hovered(Fn<void()> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

struct TextButton : Button
{
  Text text_;

  TextButton(
    Str32             text      = U""_str,
    TextStyle const & style     = TextStyle{.color = theme.on_surface},
    FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                            .height      = theme.body_font_height,
                                            .line_height = theme.line_height},
    AllocatorRef      allocator = default_allocator);

  TextButton(
    Str8 text, TextStyle const & style = TextStyle{.color = theme.on_surface},
    FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                            .height      = theme.body_font_height,
                                            .line_height = theme.line_height},
    AllocatorRef      allocator = default_allocator);

  TextButton(TextButton const &)             = delete;
  TextButton(TextButton &&)                  = default;
  TextButton & operator=(TextButton const &) = delete;
  TextButton & operator=(TextButton &&)      = default;
  virtual ~TextButton() override             = default;

  TextButton & disable(bool disable);

  TextButton & run(TextStyle const & style, FontStyle const & font,
                   usize first = 0, usize count = USIZE_MAX);

  TextButton & text(Str32 text);

  TextButton & text(Str8 text);

  TextButton & color(Vec4U8 color);

  TextButton & hovered_color(Vec4U8 color);

  TextButton & disabled_color(Vec4U8 color);

  TextButton & rrect(CornerRadii const & radii);

  TextButton & squircle(f32 degree = 5);

  TextButton & bevel(CornerRadii const & radii);

  TextButton & frame(Frame frame);

  TextButton & stroke(f32 stroke);

  TextButton & thickness(f32 thickness);

  TextButton & padding(Padding padding);

  TextButton & on_pressed(Fn<void()> fn);

  TextButton & on_hovered(Fn<void()> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;
};

}    // namespace ui

}    // namespace ash
