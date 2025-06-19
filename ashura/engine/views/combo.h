/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

struct ComboItem : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool selected : 1 = false;

    bool hovered : 1 = false;

    bool pressed : 1 = false;

    usize id = 0;

    Fn<void(usize)> click_hook = noop;
  } state_;

  ComboItem()                              = default;
  ComboItem(ComboItem const &)             = delete;
  ComboItem(ComboItem &&)                  = default;
  ComboItem & operator=(ComboItem const &) = delete;
  ComboItem & operator=(ComboItem &&)      = default;
  virtual ~ComboItem() override            = default;

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

struct TextComboItem : ComboItem
{
  struct Style
  {
    Frame frame = Frame{}.abs(1, 1);

    Padding padding = Padding::all(5);

    f32 alignment = ALIGNMENT_LEFT;

    Vec4U8 color = theme.surface_variant;

    Vec4U8 hover_color = theme.primary_variant;

    Vec4U8 selected_color = theme.primary;

    f32 stroke = 0;

    f32 thickness = 1;

    CornerRadii corner_radii = CornerRadii::all(2);
  } style_;

  Text text_;

  TextComboItem(
    Str32 text, TextStyle const & style = TextStyle{.color = theme.on_surface},
    FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                            .height      = theme.body_font_height,
                                            .line_height = theme.line_height},
    AllocatorRef      allocator = default_allocator);

  TextComboItem(
    Str8 text, TextStyle const & style = TextStyle{.color = theme.on_surface},
    FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                            .height      = theme.body_font_height,
                                            .line_height = theme.line_height},
    AllocatorRef      allocator = default_allocator);

  TextComboItem(TextComboItem const &)             = delete;
  TextComboItem(TextComboItem &&)                  = default;
  TextComboItem & operator=(TextComboItem const &) = delete;
  TextComboItem & operator=(TextComboItem &&)      = default;
  virtual ~TextComboItem() override                = default;

  TextComboItem & frame(Frame frame);

  TextComboItem & padding(Padding padding);

  TextComboItem & align(f32 alignment);

  TextComboItem & color(Vec4U8 color);

  TextComboItem & hover_color(Vec4U8 color);

  TextComboItem & selected_color(Vec4U8 color);

  TextComboItem & stroke(f32 stroke);

  TextComboItem & thickness(f32 thickness);

  TextComboItem & corner_radii(CornerRadii radii);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;
};

struct Combo : Flex
{
  struct State
  {
    bool disabled : 1 = false;

    Option<usize> selected = none;
  } state_;

  struct Style
  {
    CornerRadii corner_radii = CornerRadii::all(2);

    Vec4U8 color = theme.surface;

    f32 stroke = 0;

    f32 thickness = 1;
  } style_;

  struct Callbacks
  {
    Fn<void(Option<usize>)> selected = noop;
  } cb;

  Vec<ref<ComboItem>> items_;

  Combo(AllocatorRef allocator = default_allocator);
  Combo(Combo const &)             = delete;
  Combo(Combo &&)                  = default;
  Combo & operator=(Combo const &) = delete;
  Combo & operator=(Combo &&)      = default;
  virtual ~Combo() override        = default;

  Combo & stroke(f32 stroke);

  Combo & thickness(f32 thickness);

  Combo & axis(Axis axis);

  Combo & wrap(bool wrap);

  Combo & main_align(MainAlign align);

  Combo & cross_align(f32 align);

  Combo & frame(Frame frame);

  Combo & item_frame(Frame frame);

  Combo & disable(bool disable);

  Combo & color(Vec4U8 color);

  Combo & corner_radii(CornerRadii radii);

  Combo & on_selected(Fn<void(Option<usize>)> style);

  Combo & items(std::initializer_list<ref<ComboItem>> list);

  Combo & items(Span<ref<ComboItem> const> list);

  usize num_items() const;

  Combo & select(Option<usize> item);

  Option<usize> get_selection() const;

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;
};

}    // namespace ui

}    // namespace ash
