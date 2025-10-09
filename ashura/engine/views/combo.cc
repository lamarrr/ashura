/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/combo.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

ui::State ComboItem::tick(Ctx const &, Events const &, Fn<void(View &)>)
{
  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void ComboItem::size(f32x2, Span<f32x2>)
{
}

Layout ComboItem::fit(f32x2, Span<f32x2 const>, Span<f32x2>)
{
  return Layout{};
}

void ComboItem::render(Canvas &, RenderInfo const &)
{
}

Cursor ComboItem::cursor(f32x2, f32x2)
{
  return Cursor::Pointer;
}

TextComboItem::TextComboItem(Str32 text, TextStyle const & style,
                             FontStyle const & font, Allocator allocator) :
  text_{text, style, font, allocator}
{
  text_.copyable(false);
}

TextComboItem::TextComboItem(Str8 text, TextStyle const & style,
                             FontStyle const & font, Allocator allocator) :
  text_{text, style, font, allocator}
{
  text_.copyable(false);
}

TextComboItem & TextComboItem::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

TextComboItem & TextComboItem::padding(Padding padding)
{
  style_.padding = padding;
  return *this;
}

TextComboItem & TextComboItem::align(f32 alignment)
{
  style_.alignment = alignment;
  return *this;
}

TextComboItem & TextComboItem::color(u8x4 color)
{
  style_.color = color;
  return *this;
}

TextComboItem & TextComboItem::hover_color(u8x4 color)
{
  style_.hover_color = color;
  return *this;
}

TextComboItem & TextComboItem::selected_color(u8x4 color)
{
  style_.selected_color = color;
  return *this;
}

TextComboItem & TextComboItem::stroke(f32 stroke)
{
  style_.stroke = stroke;
  return *this;
}

TextComboItem & TextComboItem::thickness(f32 thickness)
{
  style_.thickness = thickness;
  return *this;
}

TextComboItem & TextComboItem::corner_radii(CornerRadii radii)
{
  style_.corner_radii = radii;
  return *this;
}

ui::State TextComboItem::tick(Ctx const & ctx, Events const & events,
                              Fn<void(View &)> build)
{
  if (events.pointer_over() && ctx.mouse.down(MouseButton::Primary) &&
      !ComboItem::state_.selected)
  {
    ComboItem::state_.click_hook(ComboItem::state_.id);
  }

  state_.hovered = events.pointer_over();
  state_.pressed =
    events.pointer_over() && ctx.mouse.held(MouseButton::Primary);

  build(text_);

  return ui::State{.pointable = !ComboItem::state_.disabled,
                   .clickable = !ComboItem::state_.disabled,
                   .focusable = !ComboItem::state_.disabled};
}

void TextComboItem::size(f32x2 allocated, Span<f32x2> sizes)
{
  auto child_size = style_.frame(allocated) - style_.padding.axes();
  child_size.x    = max(child_size.x, 0.0F);
  child_size.y    = max(child_size.y, 0.0F);
  sizes[0]        = child_size;
}

Layout TextComboItem::fit(f32x2 allocated, Span<f32x2 const> sizes,
                          Span<f32x2> centers)
{
  auto       frame  = style_.frame(allocated);
  auto const padded = sizes[0] + style_.padding.axes();
  frame.x           = max(frame.x, padded.x);
  frame.y           = max(frame.y, padded.y);

  centers[0] = space_align(frame, sizes[0], f32x2{style_.alignment, 0});

  return {.extent = frame};
}

void TextComboItem::render(Canvas & canvas, RenderInfo const & info)
{
  u8x4 color;
  if (ComboItem::state_.selected)
  {
    color = style_.selected_color;
  }
  else if (state_.hovered && !state_.pressed)
  {
    color = style_.color;
  }
  else if (state_.hovered)
  {
    color = style_.hover_color;
  }
  else
  {
    color = style_.color;
  }

  canvas.rrect({.area         = info.canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = style_.stroke,
                .thickness    = f32x2::splat(style_.thickness),
                .tint         = color,
                .clip         = info.clip});
}

Combo::Combo(Allocator allocator) : Flex{allocator}
{
  Flex::axis(Axis::Y)
    .main_align(MainAlign::Start)
    .frame(Frame{}.rel(1, 1))
    .item_frame(Frame{}.rel(1, 1))
    .cross_align(0);
}

Combo & Combo::stroke(f32 stroke)
{
  style_.stroke = stroke;
  return *this;
}

Combo & Combo::thickness(f32 thickness)
{
  style_.thickness = thickness;
  return *this;
}

Combo & Combo::axis(Axis a)
{
  Flex::axis(a);
  return *this;
}

Combo & Combo::wrap(bool w)
{
  Flex::wrap(w);
  return *this;
}

Combo & Combo::main_align(MainAlign align)
{
  Flex::main_align(align);
  return *this;
}

Combo & Combo::cross_align(f32 a)
{
  Flex::cross_align(a);
  return *this;
}

Combo & Combo::frame(Frame frame)
{
  Flex::frame(frame);
  return *this;
}

Combo & Combo::item_frame(Frame frame)
{
  Flex::item_frame(frame);
  return *this;
}

Combo & Combo::disable(bool d)
{
  state_.disabled = d;
  for (ComboItem & item : items_)
  {
    item.state_.disabled = d;
  }
  return *this;
}

Combo & Combo::color(u8x4 c)
{
  style_.color = c;
  return *this;
}

Combo & Combo::corner_radii(CornerRadii radii)
{
  style_.corner_radii = radii;
  return *this;
}

Combo & Combo::on_selected(Fn<void(Option<usize>)> style)
{
  cb.selected = style;
  return *this;
}

Combo & Combo::items(InitList<ref<ComboItem>> list)
{
  return items(span(list));
}

Combo & Combo::items(Span<ref<ComboItem> const> list)
{
  for (auto [i, item] : enumerate(list))
  {
    item->state_.disabled   = state_.disabled;
    item->state_.selected   = false;
    item->state_.click_hook = {this,
                               [](Combo * c, usize id) { c->select(id); }};
    item->state_.id         = i;
  }

  items_.extend(list).unwrap();
  return *this;
}

usize Combo::num_items() const
{
  return size32(items_);
}

Combo & Combo::select(Option<usize> i)
{
  if (i.is_some())
  {
    CHECK(i.v() < size32(items_), "");
  }

  state_.selected = i;

  for (ComboItem & it : items_)
  {
    it.state_.selected = false;
  }

  if (i.is_some())
  {
    ComboItem & item     = items_[i.v()];
    item.state_.selected = true;
  }

  cb.selected(i);
  return *this;
}

Option<usize> Combo::get_selection() const
{
  return state_.selected;
}

ui::State Combo::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  for (View & item : items_)
  {
    build(item);
  }

  return ui::State{};
}

void Combo::render(Canvas & canvas, RenderInfo const & info)
{
  canvas.rrect({.area         = info.canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = style_.stroke,
                .thickness    = f32x2::splat(style_.thickness),
                .tint         = style_.color,
                .clip         = info.clip});
}

Image::Image(ImageSrc src) : src_{std::move(src)}
{
}

}    // namespace ui

}    // namespace ash
