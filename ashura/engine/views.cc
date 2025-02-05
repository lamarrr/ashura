/// SPDX-License-Identifier: MIT
#include "ashura/engine/views.h"
#include "ashura/engine/engine.h"
#include "fast_float/fast_float.h"

namespace ash
{

namespace ui
{

Space & Space::frame(Frame frame)
{
  style.frame = frame;
  return *this;
}

Space & Space::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

ViewLayout Space::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return ViewLayout{.extent = style.frame(allocated)};
}

Flex::Flex(AllocatorRef allocator) : items_{allocator}
{
}

Flex & Flex::axis(Axis a)
{
  style.axis = a;
  return *this;
}

Flex & Flex::wrap(bool w)
{
  style.wrap = w;
  return *this;
}

Flex & Flex::main_align(MainAlign align)
{
  style.main_align = align;
  return *this;
}

Flex & Flex::cross_align(f32 align)
{
  style.cross_align = align;
  return *this;
}

Flex & Flex::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

Flex & Flex::frame(Frame f)
{
  style.frame = f;
  return *this;
}

Flex & Flex::item_frame(Vec2 extent, bool constrain)
{
  style.item_frame = Frame{extent, constrain};
  return *this;
}

Flex & Flex::item_frame(Frame f)
{
  style.item_frame = f;
  return *this;
}

Flex & Flex::items(std::initializer_list<ref<View>> list)
{
  return items(span(list));
}

Flex & Flex::items(Span<ref<View> const> list)
{
  items_.extend(list).unwrap();
  return *this;
}

ViewState Flex::tick(ViewContext const &, CRect const &, f32,
                     ViewEvents const &, Fn<void(View &)> build)
{
  for (ref item : items_)
  {
    build(item);
  }

  return ViewState{};
}

void Flex::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 const frame = style.frame(allocated);
  fill(sizes, style.item_frame(frame));
}

ViewLayout Flex::fit(Vec2 allocated, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  u32 const  n            = sizes.size32();
  Vec2 const frame        = style.frame(allocated);
  u32 const  main_axis    = (style.axis == Axis::X) ? 0 : 1;
  u32 const  cross_axis   = (style.axis == Axis::X) ? 1 : 0;
  Vec2       span         = {};
  f32        cross_cursor = 0;

  for (u32 i = 0; i < n;)
  {
    u32 first        = i++;
    f32 main_extent  = sizes[first][main_axis];
    f32 cross_extent = sizes[first][cross_axis];
    f32 main_spacing = 0;

    while (i < n && !(style.wrap &&
                      (main_extent + sizes[i][main_axis]) > frame[main_axis]))
    {
      main_extent += sizes[i][main_axis];
      cross_extent = max(cross_extent, sizes[i][cross_axis]);
      i++;
    }

    u32 const count = i - first;

    if (style.main_align != MainAlign::Start)
    {
      main_spacing = max(frame[main_axis] - main_extent, 0.0F);
    }

    for (u32 b = first; b < first + count; b++)
    {
      f32 const pos =
        space_align(cross_extent, sizes[b][cross_axis], style.cross_align);
      centers[b][cross_axis] = cross_cursor + cross_extent * 0.5F + pos;
    }

    switch (style.main_align)
    {
      case MainAlign::Start:
      {
        f32 main_spacing_cursor = 0;
        for (u32 b = first; b < first + count; b++)
        {
          f32 const size        = sizes[b][main_axis];
          centers[b][main_axis] = main_spacing_cursor + size * 0.5F;
          main_spacing_cursor += size;
        }
      }
      break;

      case MainAlign::SpaceAround:
      {
        f32 spacing             = main_spacing / (count * 2);
        f32 main_spacing_cursor = 0;
        for (u32 b = first; b < first + count; b++)
        {
          f32 const size = sizes[b][main_axis];
          main_spacing_cursor += spacing;
          centers[b][main_axis] = main_spacing_cursor + size * 0.5F;
          main_spacing_cursor += size + spacing;
        }
      }
      break;

      case MainAlign::SpaceBetween:
      {
        f32 spacing             = main_spacing / (count - 1);
        f32 main_spacing_cursor = 0;
        for (u32 b = first; b < first + count; b++)
        {
          f32 const size        = sizes[b][main_axis];
          centers[b][main_axis] = main_spacing_cursor + size * 0.5F;
          main_spacing_cursor += size + spacing;
        }
      }
      break;

      case MainAlign::SpaceEvenly:
      {
        f32 spacing             = main_spacing / (count + 1);
        f32 main_spacing_cursor = spacing;
        for (u32 b = first; b < first + count; b++)
        {
          f32 const size        = sizes[b][main_axis];
          centers[b][main_axis] = main_spacing_cursor + size * 0.5F;
          main_spacing_cursor += size + spacing;
        }
      }
      break;

      case MainAlign::End:
      {
        f32 main_spacing_cursor = main_spacing;
        for (u32 b = first; b < first + count; b++)
        {
          f32 const size        = sizes[b][main_axis];
          centers[b][main_axis] = main_spacing_cursor + size * 0.5F;
          main_spacing_cursor += size;
        }
      }
      break;

      default:
        break;
    }

    cross_cursor += cross_extent;

    span[main_axis]  = max(span[main_axis], main_extent + main_spacing);
    span[cross_axis] = cross_cursor;
  }

  // convert from cursor space [0, w] to parent space [-0.5w, 0.5w]
  for (Vec2 & center : centers)
  {
    center -= span * 0.5F;
  }

  return {.extent = span};
}

Stack::Stack(AllocatorRef allocator) : items_{allocator}
{
}

Stack & Stack::reverse(bool r)
{
  style.reverse = r;
  return *this;
}

Stack & Stack::align(Vec2 a)
{
  style.alignment = a;
  return *this;
}

Stack & Stack::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

Stack & Stack::frame(Frame f)
{
  style.frame = f;
  return *this;
}

Stack & Stack::items(std::initializer_list<ref<View>> list)
{
  items(span(list));
  return *this;
}

Stack & Stack::items(Span<ref<View> const> list)
{
  items_.extend(span(list)).unwrap();
  return *this;
}

i32 Stack::stack_item(i32 base, u32 i, u32 num)
{
  // sequential stacking
  if (!style.reverse)
  {
    return base + (i32) i;
  }
  else
  {
    return base + (i32) (num - i);
  }
}

ViewState Stack::tick(ViewContext const &, CRect const &, f32,
                      ViewEvents const &, Fn<void(View &)> build)
{
  for (ref item : items_)
  {
    build(item);
  }

  return ViewState{};
}

void Stack::size(Vec2 allocated, Span<Vec2> sizes)
{
  fill(sizes, style.frame(allocated));
}

ViewLayout Stack::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  Vec2      span;
  u32 const n = sizes.size32();

  for (Vec2 style : sizes)
  {
    span.x = max(span.x, style.x);
    span.y = max(span.y, style.y);
  }

  for (u32 i = 0; i < n; i++)
  {
    centers[i] = space_align(span, sizes[i], style.alignment);
  }

  return {.extent = span};
}

i32 Stack::z_index(i32 allocated, Span<i32> indices)
{
  u32 const n = indices.size32();
  for (u32 i = 0; i < n; i++)
  {
    indices[i] = stack_item(allocated, i, n);
  }
  return allocated;
}

Text::Text(Span<c32 const> t, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator},
  compositor_{allocator}
{
  text(t).run(style, font);
}

Text::Text(Span<c8 const> t, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator},
  compositor_{allocator}
{
  text(t).run(style, font);
}

Text & Text::copyable(bool allow)
{
  state.copyable = allow;
  return *this;
}

Text & Text::highlight(TextHighlight highlight)
{
  text_.highlight(highlight);
  return *this;
}

Text & Text::clear_highlights()
{
  text_.clear_highlights();
  return *this;
}

Text & Text::run(TextStyle const & style, FontStyle const & font, u32 first,
                 u32 count)
{
  text_.run(style, font, first, count);
  return *this;
}

Text & Text::text(Span<c32 const> t)
{
  text_.text(t);
  return *this;
}

Text & Text::text(Span<c8 const> t)
{
  text_.text(t);
  return *this;
}

Span<c32 const> Text::text()
{
  return text_.get_text();
}

ViewState Text::tick(ViewContext const & ctx, CRect const & region, f32 zoom,
                     ViewEvents const & events, Fn<void(View &)>)
{
  TextCommand cmd = TextCommand::None;
  if (events.drag_start)
  {
    cmd = TextCommand::Hit;
  }
  else if (events.dragging)
  {
    cmd = TextCommand::HitSelect;
  }
  else if (events.mouse_down && !compositor_.get_cursor().is_empty())
  {
    cmd = TextCommand::Unselect;
  }

  compositor_.command(text_, cmd, noop, noop, {}, engine->clipboard, 1, region,
                      ctx.mouse.position, zoom);

  return ViewState{.draggable = state.copyable};
}

ViewLayout Text::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  text_.perform_layout(allocated.x);
  return {.extent = text_.layout_.extent};
}

void Text::render(Canvas & canvas, CRect const & region, f32 zoom,
                  Rect const & clip)
{
  text_.render(canvas, region, clip.centered(), zoom);
}

Cursor Text::cursor(CRect const &, f32, Vec2)
{
  return state.copyable ? Cursor::Text : Cursor::Default;
}

Input::Input(Span<c32 const> s, TextStyle const & style, FontStyle const & font,
             AllocatorRef allocator) :
  content_{allocator},
  stub_{allocator},
  compositor_{allocator}
{
  content(U""_str).content_run(style, font).stub(s).stub_run(style, font);
}

Input::Input(Span<c8 const> s, TextStyle const & style, FontStyle const & font,
             AllocatorRef allocator) :
  content_{allocator},
  stub_{allocator},
  compositor_{allocator}
{
  content(U""_str).content_run(style, font).stub(s).stub_run(style, font);
}

Input & Input::disable(bool disable)
{
  state.disabled = disable;
  return *this;
}

Input & Input::multiline(bool e)
{
  state.multiline = e;
  return *this;
}

Input & Input::enter_submits(bool e)
{
  state.enter_submits = e;
  return *this;
}

Input & Input::tab_input(bool e)
{
  state.tab_input = e;
  return *this;
}

Input & Input::highlight(TextHighlight const & highlight)
{
  content_.highlight(highlight);
  return *this;
}

Input & Input::clear_highlights()
{
  content_.clear_highlights();
  return *this;
}

Input & Input::on_edit(Fn<void()> f)
{
  cb.edit = f;
  return *this;
}

Input & Input::on_submit(Fn<void()> f)
{
  cb.submit = f;
  return *this;
}

Input & Input::on_focus_in(Fn<void()> f)
{
  cb.focus_in = f;
  return *this;
}

Input & Input::on_focus_out(Fn<void()> f)
{
  cb.focus_out = f;
  return *this;
}

Input & Input::content(Span<c8 const> t)
{
  content_.text(t);
  return *this;
}

Input & Input::content(Span<c32 const> t)
{
  content_.text(t);
  return *this;
}

Input & Input::content_run(TextStyle const & style, FontStyle const & font,
                           u32 first, u32 count)
{
  content_.run(style, font, first, count);
  return *this;
}

Input & Input::stub(Span<c8 const> t)
{
  stub_.text(t);
  return *this;
}

Input & Input::stub(Span<c32 const> t)
{
  stub_.text(t);
  return *this;
}

Input & Input::stub_run(TextStyle const & style, FontStyle const & font,
                        u32 first, u32 count)
{
  stub_.run(style, font, first, count);
  return *this;
}

constexpr TextCommand Input::command(ViewContext const & ctx) const
{
  if (ctx.key_state(KeyCode::Escape))
  {
    return TextCommand::Unselect;
  }
  if (ctx.key_state(KeyCode::Backspace))
  {
    return TextCommand::BackSpace;
  }
  if (ctx.key_state(KeyCode::Delete))
  {
    return TextCommand::Delete;
  }
  if (ctx.key_state(KeyCode::Left))
  {
    return TextCommand::Left;
  }
  if (ctx.key_state(KeyCode::Right))
  {
    return TextCommand::Right;
  }
  if (ctx.key_state(KeyCode::Home))
  {
    return TextCommand::LineStart;
  }
  if (ctx.key_state(KeyCode::End))
  {
    return TextCommand::LineEnd;
  }
  if (ctx.key_state(KeyCode::Up))
  {
    return TextCommand::Up;
  }
  if (ctx.key_state(KeyCode::Down))
  {
    return TextCommand::Down;
  }
  if (ctx.key_state(KeyCode::PageUp))
  {
    return TextCommand::PageUp;
  }
  if (ctx.key_state(KeyCode::PageDown))
  {
    return TextCommand::PageDown;
  }
  if ((ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) &&
      ctx.key_state(KeyCode::Left))
  {
    return TextCommand::SelectLeft;
  }
  if ((ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) &&
      ctx.key_state(KeyCode::Right))
  {
    return TextCommand::SelectRight;
  }
  if ((ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) &&
      ctx.key_state(KeyCode::Up))
  {
    return TextCommand::SelectUp;
  }
  if ((ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) &&
      ctx.key_state(KeyCode::Down))
  {
    return TextCommand::SelectDown;
  }
  if ((ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) &&
      ctx.key_state(KeyCode::PageUp))
  {
    return TextCommand::SelectPageUp;
  }
  if ((ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) &&
      ctx.key_state(KeyCode::PageDown))
  {
    return TextCommand::SelectPageDown;
  }
  if ((ctx.key_state(KeyCode::LCtrl) || ctx.key_state(KeyCode::RCtrl)) &&
      ctx.key_state(KeyCode::A))
  {
    return TextCommand::SelectAll;
  }
  if ((ctx.key_state(KeyCode::LCtrl) || ctx.key_state(KeyCode::RCtrl)) &&
      ctx.key_state(KeyCode::X))
  {
    return TextCommand::Cut;
  }
  if ((ctx.key_state(KeyCode::LCtrl) || ctx.key_state(KeyCode::RCtrl)) &&
      ctx.key_state(KeyCode::C))
  {
    return TextCommand::Copy;
  }
  if ((ctx.key_state(KeyCode::LCtrl) || ctx.key_state(KeyCode::RCtrl)) &&
      ctx.key_state(KeyCode::V))
  {
    return TextCommand::Paste;
  }
  if ((ctx.key_state(KeyCode::LCtrl) || ctx.key_state(KeyCode::RCtrl)) &&
      ctx.key_state(KeyCode::Z))
  {
    return TextCommand::Undo;
  }
  if ((ctx.key_state(KeyCode::LCtrl) || ctx.key_state(KeyCode::RCtrl)) &&
      ctx.key_state(KeyCode::Y))
  {
    return TextCommand::Redo;
  }
  if ((ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) &&
      ctx.key_state(KeyCode::Left) && ctx.mouse_state(MouseButton::Primary))
  {
    return TextCommand::HitSelect;
  }
  if (state.multiline && !state.enter_submits && ctx.key_state(KeyCode::Return))
  {
    return TextCommand::NewLine;
  }
  if (state.tab_input && ctx.key_state(KeyCode::Tab))
  {
    return TextCommand::Tab;
  }
  return TextCommand::None;
}

ViewState Input::tick(ViewContext const & ctx, CRect const & region, f32 zoom,
                      ViewEvents const & events, Fn<void(View &)>)
{
  bool edited = false;
  auto erase  = [&](Slice range) {
    this->content_.text_.erase(range);
    edited |= range.is_empty();
    this->content_.flush_text();
  };

  auto insert = [&](usize pos, Span<c32 const> t) {
    this->content_.text_.insert_span(pos, t).unwrap();
    edited |= t.is_empty();
    this->content_.flush_text();
  };

  state.editing = false;
  state.submit  = false;

  state.focus.tick(events);

  TextCommand cmd = TextCommand::None;
  if (events.text_input)
  {
    cmd = TextCommand::InputText;
  }
  else if (events.drag_start)
  {
    cmd = TextCommand::Hit;
  }
  else if (events.dragging)
  {
    cmd = TextCommand::HitSelect;
  }
  else if (state.focus.focused)
  {
    cmd = command(ctx);
  }

  Vec<c32> text_input_utf32{default_allocator};

  utf8_decode(ctx.text, text_input_utf32).unwrap();

  compositor_.command(content_, cmd, fn(insert), fn(erase), text_input_utf32,
                      *engine->clipboard, style.lines_per_page, region,
                      ctx.mouse.position, zoom);

  if (edited)
  {
    state.editing = true;
  }

  if (events.focus_out)
  {
    compositor_.unselect();
  }

  if (events.key_down && ctx.key_state(KeyCode::Return) && state.enter_submits)
  {
    state.submit = true;
  }

  if (state.focus.in)
  {
    cb.focus_in();
  }

  if (state.focus.out)
  {
    cb.focus_out();
  }

  if (state.submit)
  {
    cb.submit();
  }

  if (edited)
  {
    cb.edit();
  }

  return ViewState{
    .text =
      TextInputInfo{.multiline = state.multiline, .tab_input = state.tab_input},
    .draggable  = !state.disabled,
    .focusable  = !state.disabled,
    .grab_focus = events.mouse_down
  };
}

ViewLayout Input::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  if (content_.text_.is_empty())
  {
    stub_.perform_layout(allocated.x);
    return {.extent = stub_.layout_.extent};
  }
  content_.perform_layout(allocated.x);
  return {.extent = content_.layout_.extent};
}

void Input::render(Canvas & canvas, CRect const & region, f32 zoom,
                   Rect const & clip)
{
  if (content_.text_.is_empty())
  {
    stub_.render(canvas, region, clip.centered(), zoom);
  }
  else
  {
    content_.render(
      canvas, region, clip.centered(), zoom,
      span({
        TextHighlight{
                      .slice = compositor_.get_cursor().as_slice()(content_.text_.size()),
                      .style = style.highlight}
    }));
  }
}

Cursor Input::cursor(CRect const &, f32, Vec2)
{
  return Cursor::Text;
}

ViewState Button::tick(ViewContext const & ctx, CRect const &, f32,
                       ViewEvents const &  events, Fn<void(View &)>)
{
  state.press.tick(ctx, events);

  if (state.press.in)
  {
    cb.hovered();
  }

  if (state.press.down)
  {
    cb.pressed();
  }

  return ViewState{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

void Button::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 const frame = style.frame(allocated);
  Vec2       size  = frame - style.padding * 2;
  size.x           = max(size.x, 0.0F);
  size.y           = max(size.y, 0.0F);
  fill(sizes, size);
}

ViewLayout Button::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  fill(centers, Vec2{0, 0});
  Vec2 size = sizes.is_empty() ? Vec2{0, 0} : sizes[0];
  return {.extent = size + 2 * style.padding};
}

void Button::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  Vec4U8 tint;

  if (state.disabled)
  {
    tint = style.disabled_color;
  }
  else if (state.press.hovered && !state.press.held)
  {
    tint = style.hovered_color;
  }
  else
  {
    tint = style.color;
  }

  switch (style.shape)
  {
    case ButtonShape::RRect:
      canvas.rrect({.center       = region.center,
                    .extent       = region.extent,
                    .corner_radii = style.corner_radii,
                    .stroke       = style.stroke,
                    .thickness    = style.thickness,
                    .tint         = tint});
      break;
    case ButtonShape::Squircle:
      canvas.squircle({.center       = region.center,
                       .extent       = region.extent,
                       .corner_radii = style.corner_radii,
                       .stroke       = style.stroke,
                       .thickness    = style.thickness,
                       .tint         = tint},
                      style.corner_radii.tl, 256);
      break;
    case ButtonShape::Bevel:
      canvas.brect({.center       = region.center,
                    .extent       = region.extent,
                    .corner_radii = style.corner_radii,
                    .stroke       = style.stroke,
                    .thickness    = style.thickness,
                    .tint         = tint});
      break;
    default:
      break;
  }
}

Cursor Button::cursor(CRect const &, f32, Vec2)
{
  return state.disabled ? Cursor::Default : Cursor::Pointer;
}

TextButton::TextButton(Span<c32 const> text, TextStyle const & style,
                       FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
}

TextButton::TextButton(Span<c8 const> text, TextStyle const & style,
                       FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
}

TextButton & TextButton::disable(bool d)
{
  state.disabled = d;
  return *this;
}

TextButton & TextButton::run(TextStyle const & style, FontStyle const & font,
                             u32 first, u32 count)
{
  text_.run(style, font, first, count);
  return *this;
}

TextButton & TextButton::text(Span<c32 const> t)
{
  text_.text(t);
  return *this;
}

TextButton & TextButton::text(Span<c8 const> t)
{
  text_.text(t);
  return *this;
}

TextButton & TextButton::color(Vec4U8 c)
{
  style.color = c;
  return *this;
}

TextButton & TextButton::hovered_color(Vec4U8 c)
{
  style.hovered_color = c;
  return *this;
}

TextButton & TextButton::disabled_color(Vec4U8 c)
{
  style.disabled_color = c;
  return *this;
}

TextButton & TextButton::rrect(CornerRadii const & c)
{
  style.corner_radii = c;
  style.shape        = ButtonShape::RRect;
  return *this;
}

TextButton & TextButton::squircle(f32 elasticity)
{
  style.corner_radii =
    CornerRadii{elasticity, elasticity, elasticity, elasticity};
  return *this;
}

TextButton & TextButton::bevel(CornerRadii const & c)
{
  style.corner_radii = c;
  style.shape        = ButtonShape::Bevel;
  return *this;
}

TextButton & TextButton::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

TextButton & TextButton::frame(Frame f)
{
  style.frame = f;
  return *this;
}

TextButton & TextButton::stroke(f32 stroke)
{
  style.stroke = stroke;
  return *this;
}

TextButton & TextButton::thickness(f32 thickness)
{
  style.thickness = thickness;
  return *this;
}

TextButton & TextButton::padding(Vec2 p)
{
  style.padding = p;
  return *this;
}

TextButton & TextButton::on_pressed(Fn<void()> f)
{
  cb.pressed = f;
  return *this;
}

TextButton & TextButton::on_hovered(Fn<void()> f)
{
  cb.hovered = f;
  return *this;
}

ViewState TextButton::tick(ViewContext const & ctx, CRect const & region,
                           f32 zoom, ViewEvents const & events,
                           Fn<void(View &)> build)
{
  ViewState state = Button::tick(ctx, region, zoom, events, build);
  build(text_);
  return state;
}

CheckBox & CheckBox::disable(bool d)
{
  state.disabled = d;
  return *this;
}

CheckBox & CheckBox::box_color(Vec4U8 c)
{
  style.box_color = c;
  return *this;
}

CheckBox & CheckBox::box_hovered_color(Vec4U8 c)
{
  style.box_hovered_color = c;
  return *this;
}

CheckBox & CheckBox::tick_color(Vec4U8 c)
{
  style.tick_color = c;
  return *this;
}

CheckBox & CheckBox::stroke(f32 s)
{
  style.stroke = s;
  return *this;
}

CheckBox & CheckBox::thickness(f32 t)
{
  style.thickness = t;
  return *this;
}

CheckBox & CheckBox::tick_thickness(f32 t)
{
  style.tick_thickness = t;
  return *this;
}

CheckBox & CheckBox::corner_radii(CornerRadii const & r)
{
  style.corner_radii = r;
  return *this;
}

CheckBox & CheckBox::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

CheckBox & CheckBox::frame(Frame f)
{
  style.frame = f;
  return *this;
}

CheckBox & CheckBox::on_changed(Fn<void(bool)> f)
{
  cb.changed = f;
  return *this;
}

ViewState CheckBox::tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents const &  events, Fn<void(View &)>)
{
  state.press.tick(ctx, events);

  if (state.press.down)
  {
    state.value = !state.value;
    cb.changed(state.value);
  }

  return ViewState{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

ViewLayout CheckBox::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  Vec2 extent = style.frame(allocated);
  return {.extent = Vec2::splat(min(extent.x, extent.y))};
}

void CheckBox::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  Vec4U8 tint;
  if (state.press.hovered && !state.press.held && !state.disabled)
  {
    tint = style.box_hovered_color;
  }
  else
  {
    tint = style.box_color;
  }

  canvas.rrect({.center       = region.center,
                .extent       = region.extent,
                .corner_radii = style.corner_radii,
                .stroke       = 1,
                .thickness    = 2,
                .tint         = tint});

  if (state.value)
  {
    constexpr Vec2 TICK_VERTICES[] = {
      {-0.5F,   0    },
      {-0.125F, 0.5F },
      {0.5F,    -0.5F}
    };

    canvas.line({.center    = region.center,
                 .extent    = region.extent,
                 .stroke    = 0,
                 .thickness = style.tick_thickness,
                 .tint      = style.tick_color},
                TICK_VERTICES);
  }
}

Cursor CheckBox::cursor(CRect const &, f32, Vec2)
{
  return state.disabled ? Cursor::Default : Cursor::Pointer;
}

Slider & Slider::disable(bool disable)
{
  state.disabled = disable;
  return *this;
}

Slider & Slider::range(f32 low, f32 high)
{
  state.low  = low;
  state.high = high;
  return *this;
}

Slider & Slider::interp(f32 t)
{
  state.t = t;
  return *this;
}

Slider & Slider::axis(Axis a)
{
  style.axis = a;
  return *this;
}

Slider & Slider::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

Slider & Slider::frame(Frame f)
{
  style.frame = f;
  return *this;
}

Slider & Slider::thumb_size(f32 size)
{
  style.thumb_size = size;
  return *this;
}

Slider & Slider::track_size(f32 size)
{
  style.track_size = size;
  return *this;
}

Slider & Slider::thumb_color(Vec4U8 c)
{
  style.thumb_color = c;
  return *this;
}

Slider & Slider::thumb_hovered_color(Vec4U8 c)
{
  style.thumb_hovered_color = c;
  return *this;
}

Slider & Slider::thumb_dragging_color(Vec4U8 c)
{
  style.thumb_dragging_color = c;
  return *this;
}

Slider & Slider::thumb_corner_radii(CornerRadii const & c)
{
  style.thumb_corner_radii = c;
  return *this;
}

Slider & Slider::track_color(Vec4U8 c)
{
  style.track_color = c;
  return *this;
}

Slider & Slider::track_corner_radii(CornerRadii const & c)
{
  style.track_corner_radii = c;
  return *this;
}

Slider & Slider::on_changed(Fn<void(f32)> f)
{
  cb.changed = f;
  return *this;
}

ViewState Slider::tick(ViewContext const & ctx, CRect const & region, f32,
                       ViewEvents const & events, Fn<void(View &)>)
{
  u32 const main_axis = (style.axis == Axis::X) ? 0 : 1;

  state.drag.tick(events);

  if (state.drag.dragging)
  {
    f32 const thumb_begin = region.begin()[main_axis] + style.thumb_size * 0.5F;
    f32 const thumb_end   = region.end()[main_axis] - style.thumb_size * 0.5F;
    state.t =
      clamp(unlerp(thumb_begin, thumb_end, ctx.mouse.position[main_axis]), 0.0F,
            1.0F);
    f32 const value =
      clamp(lerp(state.low, state.high, state.t), state.low, state.high);
    cb.changed(value);
  }

  if (state.drag.focus.focused)
  {
    if ((style.axis == Axis::X && ctx.key_state(KeyCode::Left)) ||
        (style.axis == Axis::Y && ctx.key_state(KeyCode::Up)))
    {
      state.t = max(state.t - style.delta, 0.0F);
    }
    else if ((style.axis == Axis::X && ctx.key_state(KeyCode::Right)) ||
             (style.axis == Axis::Y && ctx.key_state(KeyCode::Down)))
    {
      state.t = min(state.t + style.delta, 1.0F);
    }
  }

  return ViewState{.pointable = !state.disabled,
                   .draggable = !state.disabled,
                   .focusable = !state.disabled};
}

ViewLayout Slider::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style.frame(allocated)};
}

void Slider::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  u32 const main_axis  = (style.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style.axis == Axis::Y) ? 0 : 1;

  Vec4U8 thumb_color;

  if (state.drag.dragging)
  {
    thumb_color = style.thumb_dragging_color;
  }
  else if (state.drag.hovered)
  {
    thumb_color = style.thumb_hovered_color;
  }
  else
  {
    thumb_color = style.thumb_color;
  }

  f32 dilation = 1.0F;

  if (state.drag.dragging || state.drag.hovered)
  {
    dilation = 1.0F;
  }
  else
  {
    dilation = 0.8F;
  }

  f32 const thumb_begin  = region.begin()[main_axis] + style.thumb_size * 0.5F;
  f32 const thumb_end    = region.end()[main_axis] - style.thumb_size * 0.5F;
  f32 const thumb_center = lerp(thumb_begin, thumb_end, state.t);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = region.center[cross_axis];
  thumb_rect.extent             = Vec2::splat(style.thumb_size);

  CRect track_rect;

  track_rect.center             = region.center;
  track_rect.extent[main_axis]  = thumb_end - thumb_begin;
  track_rect.extent[cross_axis] = style.track_size;

  Vec2 coverage_begin;
  coverage_begin[main_axis]  = thumb_begin;
  coverage_begin[cross_axis] = track_rect.begin()[cross_axis];

  Vec2 coverage_end;
  coverage_end[main_axis]  = thumb_center;
  coverage_end[cross_axis] = track_rect.end()[cross_axis];

  CRect const coverage_rect = CRect::from_range(coverage_begin, coverage_end);

  canvas
    .rrect({.center       = track_rect.center,
            .extent       = track_rect.extent,
            .corner_radii = style.track_corner_radii,
            .tint         = style.track_color})
    .rrect({.center       = coverage_rect.center,
            .extent       = coverage_rect.extent,
            .corner_radii = style.track_corner_radii,
            .tint         = thumb_color})
    .rrect({.center       = thumb_rect.center,
            .extent       = thumb_rect.extent * dilation,
            .corner_radii = style.thumb_corner_radii,
            .tint         = thumb_color});
}

Cursor Slider::cursor(CRect const &, f32, Vec2)
{
  return state.disabled ? Cursor::Default : Cursor::Pointer;
}

Switch & Switch::disable(bool disable)
{
  state.disabled = disable;
  return *this;
}

Switch & Switch::on()
{
  state.value = true;
  cb.changed(true);
  return *this;
}

Switch & Switch::off()
{
  state.value = false;
  cb.changed(false);
  return *this;
}

Switch & Switch::toggle()
{
  if (state.value)
  {
    on();
  }
  else
  {
    off();
  }
  return *this;
}

Switch & Switch::on_color(Vec4U8 c)
{
  style.on_color = c;
  return *this;
}

Switch & Switch::on_hovered_color(Vec4U8 c)
{
  style.on_hovered_color = c;
  return *this;
}

Switch & Switch::off_color(Vec4U8 c)
{
  style.off_color = c;
  return *this;
}

Switch & Switch::off_hovered_color(Vec4U8 c)
{
  style.off_hovered_color = c;
  return *this;
}

Switch & Switch::track_color(Vec4U8 c)
{
  style.track_color = c;
  return *this;
}

Switch & Switch::corner_radii(CornerRadii const & r)
{
  style.corner_radii = r;
  return *this;
}

Switch & Switch::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

Switch & Switch::frame(Frame f)
{
  style.frame = f;
  return *this;
}

ViewState Switch::tick(ViewContext const & ctx, CRect const &, f32,
                       ViewEvents const &  events, Fn<void(View &)>)
{
  state.press.tick(ctx, events);

  if (state.press.down)
  {
    state.value = !state.value;
    cb.changed(state.value);
  }

  return ViewState{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

ViewLayout Switch::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style.frame(allocated)};
}

void Switch::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  Vec2 thumb_extent = region.extent;
  thumb_extent.x *= 0.5F;
  Vec2 const alignment{state.value ? 1.0F : -1.0F, 0};
  Vec2 const thumb_center =
    region.center + space_align(region.extent, thumb_extent, alignment);

  Vec4U8 thumb_color;
  if (state.press.hovered)
  {
    thumb_color =
      state.value ? style.on_hovered_color : style.off_hovered_color;
  }
  else
  {
    thumb_color = state.value ? style.on_color : style.off_color;
  }

  canvas
    .rrect({.center       = region.center,
            .extent       = region.extent,
            .corner_radii = style.corner_radii,
            .tint         = style.track_color})
    .rrect({.center       = thumb_center,
            .extent       = thumb_extent,
            .corner_radii = style.corner_radii,
            .tint         = thumb_color});
}

Cursor Switch::cursor(CRect const &, f32, Vec2)
{
  return state.disabled ? Cursor::Default : Cursor::Pointer;
}

Radio & Radio::disable(bool disable)
{
  state.disabled = disable;
  return *this;
}

Radio & Radio::corner_radii(CornerRadii const & c)
{
  style.corner_radii = c;
  return *this;
}

Radio & Radio::thickness(f32 t)
{
  style.thickness = t;
  return *this;
}

Radio & Radio::color(Vec4U8 c)
{
  style.color = c;
  return *this;
}

Radio & Radio::inner_color(Vec4U8 c)
{
  style.inner_color = c;
  return *this;
}

Radio & Radio::inner_hovered_color(Vec4U8 c)
{
  style.inner_hovered_color = c;
  return *this;
}

Radio & Radio::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

Radio & Radio::frame(Frame f)
{
  style.frame = f;
  return *this;
}

Radio & Radio::on_changed(Fn<void(bool)> f)
{
  cb.changed = f;
  return *this;
}

ViewState Radio::tick(ViewContext const & ctx, CRect const &, f32,
                      ViewEvents const &  events, Fn<void(View &)>)
{
  state.press.tick(ctx, events);

  if (state.press.down)
  {
    state.value = !state.value;
    cb.changed(state.value);
  }

  return ViewState{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

ViewLayout Radio::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style.frame(allocated)};
}

void Radio::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  canvas.rrect({.center       = region.center,
                .extent       = region.extent,
                .corner_radii = style.corner_radii,
                .stroke       = 1,
                .thickness    = style.thickness,
                .tint         = style.color});

  if (state.value)
  {
    Vec2   inner_extent = region.extent * (state.press.hovered ? 0.75F : 0.5F);
    Vec4U8 inner_color =
      state.press.hovered ? style.inner_hovered_color : style.inner_color;

    canvas.circle(
      {.center = region.center, .extent = inner_extent, .tint = inner_color});
  }
}

Cursor Radio::cursor(CRect const &, f32, Vec2)
{
  return Cursor::Pointer;
}

void ScalarDragBox::scalar_parse(Span<c32 const> text, ScalarInfo const & spec,
                                 Scalar & scalar)
{
  if (text.is_empty())
  {
    return;
  }

  spec.match(
    [&](F32Info const & spec) {
      f32 value = 0;
      auto [ptr, ec] =
        fast_float::from_chars(text.pbegin(), text.pend(), value);
      if (ec != std::errc{} || value < spec.min || value > spec.max)
      {
        return;
      }
      scalar = value;
    },
    [&](I32Info const & spec) {
      i32 value = 0;
      auto [ptr, ec] =
        fast_float::from_chars(text.pbegin(), text.pend(), value);
      if (ec != std::errc{} || value < spec.min || value > spec.max)
      {
        return;
      }
      scalar = value;
    });
}

ViewState ScalarDragBox::tick(ViewContext const & ctx, CRect const & region,
                              f32, ViewEvents const &                events,
                              Fn<void(View &)> build)
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
      clamp(unlerp(region.begin().x, region.end().x, ctx.mouse.position.x),
            0.0F, 1.0F);
    state.scalar =
      state.spec.match([t](F32Info & v) -> Scalar { return v.interp(t); },
                       [t](I32Info & v) -> Scalar { return v.interp(t); });
    state.hash = 0;
  }

  if (input_.state.editing)
  {
    scalar_parse(input_.content_.get_text(), state.spec, state.scalar);
    state.hash = 0;
  }

  if (state.hash == 0)
  {
    char   text_[1'024];
    bool   is_full = false;
    Buffer text{text_};

    auto const sink = [&](Span<char const> str) {
      is_full = is_full | text.extend(str);
    };

    fmt::Op ops_[fmt::MAX_ARGS];
    Buffer  ops{ops_};

    fmt::Context ctx{fn(sink), std::move(ops)};

    if (auto result = ctx.format(style.format_str, state.scalar);
        result.error != fmt::Error::None)
    {
      input_.content_.text(text.view().as_c8());
    }

    state.hash = -1;
  }

  input_.state.disabled = !state.input_mode;

  if (input_.state.editing || state.dragging)
  {
    cb.update(state.scalar);
  }

  state.focus.tick(events);

  build(input_);

  return ViewState{.pointable = !state.disabled,
                   .draggable = !state.disabled,
                   .focusable = !state.disabled};
}

void ScalarDragBox::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 child = style.frame(allocated) - 2 * style.padding;
  child.x    = max(child.x, 0.0F);
  child.y    = max(child.y, 0.0F);
  fill(sizes, child);
}

ViewLayout ScalarDragBox::fit(Vec2 allocated, Span<Vec2 const> sizes,
                              Span<Vec2> centers)
{
  Vec2 frame         = style.frame(allocated);
  Vec2 padded_extent = sizes[0] + 2 * style.padding;
  frame.x            = max(frame.x, padded_extent.x);
  frame.y            = max(frame.y, padded_extent.y);
  fill(centers, Vec2{0, 0});
  return {.extent = frame};
}

void ScalarDragBox::render(Canvas & canvas, CRect const & region, f32,
                           Rect const &)
{
  canvas.rrect({.center       = region.center,
                .extent       = region.extent,
                .corner_radii = style.corner_radii,
                .stroke       = style.stroke,
                .thickness    = style.thickness,
                .tint         = style.color});

  if (!state.input_mode)
  {
    f32 const t = state.spec.match(
      [this](F32Info & v) { return v.uninterp(state.scalar[v0]); },
      [this](I32Info & v) { return v.uninterp(state.scalar[v1]); });

    CRect const thumb_rect =
      CRect::from_offset(region.begin(), region.extent * Vec2{t, 1});

    canvas.rrect({.center       = thumb_rect.center,
                  .extent       = thumb_rect.extent,
                  .corner_radii = style.corner_radii,
                  .tint         = style.thumb_color});
  }
}

Cursor ScalarDragBox::cursor(CRect const & region, f32, Vec2 offset)
{
  (void) region;
  (void) offset;
  return state.disabled ? Cursor::Default : Cursor::EWResize;
}

ScalarBox::ScalarBox(Span<c32 const>   decrease_text,
                     Span<c32 const>   increase_text,
                     TextStyle const & button_text_style,
                     TextStyle const & drag_text_style,
                     FontStyle const & icon_font, FontStyle const & text_font,
                     AllocatorRef allocator) :
  Flex{allocator},
  dec_{decrease_text, button_text_style, icon_font, allocator},
  inc_{increase_text, button_text_style, icon_font, allocator},
  drag_{drag_text_style, text_font, allocator}
{
  Flex::axis(Axis::X)
    .wrap(false)
    .main_align(MainAlign::Start)
    .cross_align(0)
    .frame(Frame{}.scale(1, 1));

  constexpr auto decrement = +[](ScalarBox * b) { b->step(-1); };

  constexpr auto increment = +[](ScalarBox * b) { b->step(1); };

  dec_.on_pressed(Fn{this, decrement});

  inc_.on_pressed(Fn{this, increment});

  padding({5, 5}).corner_radii(CornerRadii::all(6));

  drag_.cb.update =
    Fn{this, +[](ScalarBox * b, Scalar in) { b->cb.update(in); }};
}

ScalarBox & ScalarBox::step(i32 direction)
{
  auto & state = drag_.state;
  state.scalar = state.spec.match(
    [&](F32Info const & spec) -> Scalar {
      return spec.step_value(state.scalar[v0], direction);
    },
    [&](I32Info const & spec) -> Scalar {
      return spec.step_value(state.scalar[v1], direction);
    });
  state.hash = 0;
  cb.update(state.scalar);
  return *this;
}

ScalarBox & ScalarBox::stub(Span<c32 const> text)
{
  drag_.input_.stub(text);
  return *this;
}

ScalarBox & ScalarBox::stub(Span<c8 const> text)
{
  drag_.input_.stub(text);
  return *this;
}

ScalarBox & ScalarBox::format(Span<char const> format)
{
  drag_.style.format_str = format;
  drag_.state.hash       = 0;
  return *this;
}

ScalarBox & ScalarBox::spec(f32 scalar, F32Info info)
{
  drag_.state.scalar = scalar;
  drag_.state.spec   = info;
  drag_.state.hash   = 0;
  return *this;
}

ScalarBox & ScalarBox::spec(i32 scalar, I32Info info)
{
  drag_.state.scalar = scalar;
  drag_.state.spec   = info;
  drag_.state.hash   = 0;
  return *this;
}

ScalarBox & ScalarBox::stroke(f32 s)
{
  drag_.style.stroke = s;
  return *this;
}

ScalarBox & ScalarBox::thickness(f32 t)
{
  drag_.style.thickness = t;
  return *this;
}

ScalarBox & ScalarBox::padding(Vec2 p)
{
  dec_.padding(p);
  inc_.padding(p);
  drag_.style.padding = p;
  return *this;
}

ScalarBox & ScalarBox::frame(Vec2 extent, bool constrain)
{
  dec_.frame(extent, constrain);
  inc_.frame(extent, constrain);
  drag_.style.frame = Frame{extent, constrain};
  return *this;
}

ScalarBox & ScalarBox::frame(Frame f)
{
  dec_.frame(f);
  inc_.frame(f);
  drag_.style.frame = f;
  return *this;
}

ScalarBox & ScalarBox::corner_radii(CornerRadii const & r)
{
  dec_.rrect(r);
  inc_.rrect(r);
  drag_.style.corner_radii = r;
  return *this;
}

ScalarBox & ScalarBox::on_update(Fn<void(Scalar)> f)
{
  cb.update = f;
  return *this;
}

ScalarBox & ScalarBox::button_text_style(TextStyle const & style,
                                         FontStyle const & font, u32 first,
                                         u32 count)
{
  dec_.run(style, font, first, count);
  inc_.run(style, font, first, count);
  return *this;
}

ScalarBox & ScalarBox::drag_text_style(TextStyle const & style,
                                       FontStyle const & font, u32 first,
                                       u32 count)
{
  drag_.input_.content_run(style, font, first, count)
    .stub_run(style, font, first, count);
  return *this;
}

ViewState ScalarBox::tick(ViewContext const &, CRect const &, f32,
                          ViewEvents const &, Fn<void(View &)> build)
{
  build(dec_);
  build(drag_);
  build(inc_);
  return ViewState{};
}

ViewState ScrollBar::tick(ViewContext const & ctx, CRect const & region, f32,
                          ViewEvents const & events, Fn<void(View &)>)
{
  u32 const main_axis = (style.axis == Axis::X) ? 0 : 1;

  state.drag.tick(events);

  if (state.drag.dragging)
  {
    f32 const thumb_begin = region.begin()[main_axis] + style.thumb_size * 0.5F;
    f32 const thumb_end   = region.end()[main_axis] - style.thumb_size * 0.5F;
    state.t =
      clamp(unlerp(thumb_begin, thumb_end, ctx.mouse.position[main_axis]), 0.0F,
            1.0F);
  }

  if (state.drag.focus.focused)
  {
    if ((style.axis == Axis::X && ctx.key_state(KeyCode::Left)) ||
        (style.axis == Axis::Y && ctx.key_state(KeyCode::Up)))
    {
      state.t = max(state.t - style.delta, 0.0F);
    }
    else if ((style.axis == Axis::X && ctx.key_state(KeyCode::Right)) ||
             (style.axis == Axis::Y && ctx.key_state(KeyCode::Down)))
    {
      state.t = min(state.t + style.delta, 1.0F);
    }
  }

  return ViewState{.hidden    = state.hidden,
                   .pointable = !state.disabled,
                   .draggable = !state.disabled,
                   .focusable = !state.disabled};
}

ViewLayout ScrollBar::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = allocated};
}

i32 ScrollBar::stack(i32 allocated)
{
  // needs to be at a different stacking context since this will be placed
  // on top of the viewport
  return allocated + 1;
}

void ScrollBar::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  u32 const main_axis  = (style.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style.axis == Axis::X) ? 1 : 0;

  f32 const thumb_begin  = region.begin()[main_axis] + style.thumb_size * 0.5F;
  f32 const thumb_end    = region.end()[main_axis] - style.thumb_size * 0.5F;
  f32 const thumb_center = lerp(thumb_begin, thumb_end, state.t);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = region.center[cross_axis];
  thumb_rect.extent[main_axis]  = style.thumb_size;
  thumb_rect.extent[cross_axis] = region.extent[cross_axis];

  Vec4U8 thumb_color;
  if (state.drag.dragging)
  {
    thumb_color = style.thumb_dragging_color;
  }
  else if (state.drag.hovered)
  {
    thumb_color = style.thumb_hovered_color;
  }
  else
  {
    thumb_color = style.thumb_color;
  }

  canvas
    .rrect({.center       = region.center,
            .extent       = region.extent,
            .corner_radii = style.track_corner_radii,
            .stroke       = 0,
            .tint         = style.track_color})
    .rrect({.center       = thumb_rect.center,
            .extent       = thumb_rect.extent,
            .corner_radii = style.thumb_corner_radii,
            .stroke       = 0,
            .tint         = thumb_color});
}

ScrollView::ScrollView(View & child) : child_{child}
{
  x_bar_.style.axis = Axis::X;
  y_bar_.style.axis = Axis::Y;
}

ScrollView & ScrollView::disable(bool d)
{
  state.disabled        = d;
  x_bar_.state.disabled = d;
  y_bar_.state.disabled = d;
  return *this;
}

ScrollView & ScrollView::item(View & v)
{
  child_ = v;
  return *this;
}

ScrollView & ScrollView::thumb_size(f32 size)
{
  x_bar_.style.thumb_size = size;
  y_bar_.style.thumb_size = size;
  return *this;
}

ScrollView & ScrollView::thumb_color(Vec4U8 c)
{
  x_bar_.style.thumb_color = c;
  y_bar_.style.thumb_color = c;
  return *this;
}

ScrollView & ScrollView::thumb_hovered_color(Vec4U8 c)
{
  x_bar_.style.thumb_hovered_color = c;
  y_bar_.style.thumb_hovered_color = c;
  return *this;
}

ScrollView & ScrollView::thumb_dragging_color(Vec4U8 c)
{
  x_bar_.style.thumb_dragging_color = c;
  y_bar_.style.thumb_dragging_color = c;
  return *this;
}

ScrollView & ScrollView::thumb_corner_radii(CornerRadii const & c)
{
  x_bar_.style.thumb_corner_radii = c;
  y_bar_.style.thumb_corner_radii = c;
  return *this;
}

ScrollView & ScrollView::track_color(Vec4U8 c)
{
  x_bar_.style.track_color = c;
  y_bar_.style.track_color = c;
  return *this;
}

ScrollView & ScrollView::track_corner_radii(CornerRadii const & c)
{
  x_bar_.style.track_corner_radii = c;
  y_bar_.style.track_corner_radii = c;
  return *this;
}

ScrollView & ScrollView::axes(Axes a)
{
  x_bar_.state.hidden = has_bits(a, Axes::X);
  y_bar_.state.hidden = has_bits(a, Axes::Y);
  return *this;
}

ScrollView & ScrollView::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

ScrollView & ScrollView::frame(Frame f)
{
  style.frame = f;
  return *this;
}

ScrollView & ScrollView::inner_frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

ScrollView & ScrollView::inner_frame(Frame f)
{
  style.frame = f;
  return *this;
}

ScrollView & ScrollView::bar_size(f32 x, f32 y)
{
  style.x_bar_size = x;
  style.y_bar_size = y;
  return *this;
}

ViewState ScrollView::tick(ViewContext const & ctx, CRect const & region, f32,
                           ViewEvents const & events, Fn<void(View &)> build)
{
  if (events.mouse_scroll)
  {
    if (!x_bar_.state.disabled)
    {
      x_bar_.state.t =
        clamp(ctx.mouse.wheel_translation.x / region.extent.x, 0.0F, 1.0F);
    }

    if (!y_bar_.state.disabled)
    {
      y_bar_.state.t =
        clamp(ctx.mouse.wheel_translation.y / region.extent.y, 0.0F, 1.0F);
    }
  }

  build(child_);
  build(x_bar_);
  build(y_bar_);

  return ViewState{.viewport = true};
}

void ScrollView::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 const frame = style.frame(allocated);

  sizes[0] = style.inner_frame(frame);
  sizes[1] = {frame.x, style.x_bar_size};

  if (!x_bar_.state.disabled && !y_bar_.state.disabled)
  {
    sizes[1].x = max(sizes[1].x - style.y_bar_size, 0.0F);
  }

  sizes[2] = {style.y_bar_size, frame.y};
}

ViewLayout ScrollView::fit(Vec2 allocated, Span<Vec2 const> sizes,
                           Span<Vec2> centers)
{
  Vec2 const frame = style.frame(allocated);

  centers[0] = {0, 0};
  centers[1] = space_align(frame, sizes[1], ALIGNMENT_BOTTOM_LEFT);
  centers[2] = space_align(frame, sizes[2], ALIGNMENT_TOP_RIGHT);

  Vec2 const context_extent = sizes[0];

  return {.extent          = frame,
          .viewport_extent = context_extent,
          .viewport_transform =
            scroll_transform(context_extent, frame,
                             {x_bar_.state.t, y_bar_.state.t}, state.zoom)};
}

ViewState ComboItem::tick(ViewContext const &, CRect const &, f32,
                          ViewEvents const &, Fn<void(View &)>)
{
  return ViewState{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

void ComboItem::size(Vec2, Span<Vec2>)
{
}

ViewLayout ComboItem::fit(Vec2, Span<Vec2 const>, Span<Vec2>)
{
  return ViewLayout{};
}

void ComboItem::render(Canvas &, CRect const &, f32, Rect const &)
{
}

Cursor ComboItem::cursor(CRect const &, f32, Vec2)
{
  return Cursor::Pointer;
}

TextComboItem::TextComboItem(Span<c32 const> text, TextStyle const & style,
                             FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
  text_.copyable(false);
}

TextComboItem::TextComboItem(Span<c8 const> text, TextStyle const & style,
                             FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
  text_.copyable(false);
}

TextComboItem & TextComboItem::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

TextComboItem & TextComboItem::frame(Frame frame)
{
  style.frame = frame;
  return *this;
}

TextComboItem & TextComboItem::padding(Vec2 padding)
{
  style.padding = padding;
  return *this;
}

TextComboItem & TextComboItem::align(f32 alignment)
{
  style.alignment = alignment;
  return *this;
}

TextComboItem & TextComboItem::color(Vec4U8 color)
{
  style.color = color;
  return *this;
}

TextComboItem & TextComboItem::hover_color(Vec4U8 color)
{
  style.hover_color = color;
  return *this;
}

TextComboItem & TextComboItem::selected_color(Vec4U8 color)
{
  style.selected_color = color;
  return *this;
}

TextComboItem & TextComboItem::stroke(f32 stroke)
{
  style.stroke = stroke;
  return *this;
}

TextComboItem & TextComboItem::thickness(f32 thickness)
{
  style.thickness = thickness;
  return *this;
}

TextComboItem & TextComboItem::corner_radii(CornerRadii radii)
{
  style.corner_radii = radii;
  return *this;
}

ViewState TextComboItem::tick(ViewContext const & ctx, CRect const &, f32,
                              ViewEvents const & events, Fn<void(View &)> build)
{
  state.press.tick(ctx, events);

  if (state.press.down && !ComboItem::state.selected)
  {
    ComboItem::state.click_hook(ComboItem::state.id);
  }

  build(text_);

  return ViewState{.pointable = !ComboItem::state.disabled,
                   .clickable = !ComboItem::state.disabled,
                   .focusable = !ComboItem::state.disabled};
}

void TextComboItem::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 child_size = style.frame(allocated) - 2 * style.padding;
  child_size.x    = max(child_size.x, 0.0F);
  child_size.y    = max(child_size.y, 0.0F);
  sizes[0]        = child_size;
}

ViewLayout TextComboItem::fit(Vec2 allocated, Span<Vec2 const> sizes,
                              Span<Vec2> centers)
{
  Vec2 frame = style.frame(allocated);
  frame.x    = max(frame.x, sizes[0].x + 2 * style.padding.x);
  frame.y    = max(frame.y, sizes[0].y + 2 * style.padding.y);

  centers[0] = space_align(frame, sizes[0], Vec2{style.alignment, 0});

  return {.extent = frame};
}

void TextComboItem::render(Canvas & canvas, CRect const & region, f32,
                           Rect const &)
{
  Vec4U8 color;
  if (ComboItem::state.selected)
  {
    color = style.selected_color;
  }
  else if (state.press.hovered && !state.press.down)
  {
    color = style.color;
  }
  else if (state.press.hovered)
  {
    color = style.hover_color;
  }
  else
  {
    color = style.color;
  }

  canvas.rrect({.center       = region.center,
                .extent       = region.extent,
                .corner_radii = style.corner_radii,
                .stroke       = style.stroke,
                .thickness    = style.thickness,
                .tint         = color});
}

Combo::Combo(AllocatorRef allocator) : Flex{allocator}
{
  Flex::axis(Axis::Y)
    .main_align(MainAlign::Start)
    .frame(Frame{}.scale(1, 1))
    .item_frame(Frame{}.scale(1, 1))
    .cross_align(0);
}

Combo & Combo::stroke(f32 stroke)
{
  style.stroke = stroke;
  return *this;
}

Combo & Combo::thickness(f32 thickness)
{
  style.thickness = thickness;
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

Combo & Combo::frame(Vec2 extent, bool constrain)
{
  Flex::frame(extent, constrain);
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

Combo & Combo::item_frame(Vec2 extent, bool constrain)
{
  Flex::item_frame(extent, constrain);
  return *this;
}

Combo & Combo::disable(bool d)
{
  state.disabled = d;
  for (ComboItem & item : items_)
  {
    item.state.disabled = d;
  }
  return *this;
}

Combo & Combo::color(Vec4U8 c)
{
  style.color = c;
  return *this;
}

Combo & Combo::corner_radii(CornerRadii radii)
{
  style.corner_radii = radii;
  return *this;
}

Combo & Combo::on_selected(Fn<void(Option<u32>)> style)
{
  cb.selected = style;
  return *this;
}

Combo & Combo::items(std::initializer_list<ref<ComboItem>> list)
{
  return items(span(list));
}

Combo & Combo::items(Span<ref<ComboItem> const> list)
{
  for (auto [i, item] : enumerate<u32>(list))
  {
    item->state.disabled = state.disabled;
    item->state.selected = false;
    item->state.click_hook =
      fn(this, +[](Combo * c, u32 id) { c->select(id); });
    item->state.id = i;
  }

  items_.extend(list).unwrap();
  return *this;
}

u32 Combo::num_items() const
{
  return items_.size32();
}

Combo & Combo::select(Option<u32> i)
{
  if (i.is_some())
  {
    CHECK(i.value() < items_.size32(), "");
  }

  state.selected = i;

  for (ComboItem & it : items_)
  {
    it.state.selected = false;
  }

  if (i.is_some())
  {
    ComboItem & item    = items_[i.value()];
    item.state.selected = true;
  }

  cb.selected(i);
  return *this;
}

Option<u32> Combo::get_selection() const
{
  return state.selected;
}

ViewState Combo::tick(ViewContext const &, CRect const &, f32,
                      ViewEvents const &, Fn<void(View &)> build)
{
  for (View & item : items_)
  {
    build(item);
  }

  return ViewState{};
}

void Combo::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  canvas.rrect({.center       = region.center,
                .extent       = region.extent,
                .corner_radii = style.corner_radii,
                .stroke       = style.stroke,
                .thickness    = style.thickness,
                .tint         = style.color});
}

Image::Image(ImageSrc src) : src_{std::move(src)}
{
}

Image & Image::source(ImageSrc src)
{
  src_           = std::move(src);
  state.resolved = none;
  return *this;
}

Image & Image::aspect_ratio(f32 width, f32 height)
{
  style.aspect_ratio = (width == 0 || height == 0) ? 1 : (width / height);
  return *this;
}

Image & Image::aspect_ratio(Option<f32> ratio)
{
  style.aspect_ratio = ratio;
  return *this;
}

Image & Image::frame(Frame frame)
{
  style.frame = frame;
  return *this;
}

Image & Image::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

Image & Image::corner_radii(CornerRadii const & radii)
{
  style.radii = radii;
  return *this;
}

Image & Image::tint(ColorGradient const & color)
{
  style.tint = color;
  return *this;
}

Image & Image::fit(ImageFit fit)
{
  style.fit = fit;
  return *this;
}

Image & Image::align(Vec2 a)
{
  style.alignment = a;
  return *this;
}

ViewState Image::tick(ViewContext const &, CRect const &, f32,
                      ViewEvents const &, Fn<void(View &)>)
{
  state.resolved.match(
    [&](None) {
      src_.match(
        [&](None) { state.resolved = Option<ash::ImageInfo>{none}; },
        [&](ImageId id) { state.resolved = Option{sys->image.get(id)}; },
        [&](Future<Result<ImageId, ImageLoadErr>> & f) {
          f.poll().match(
            [&](Result<ImageId, ImageLoadErr> & r) {
              r.match(
                [&](ImageId id) {
                  state.resolved = Option{sys->image.get(id)};
                },
                [&](ImageLoadErr err) { state.resolved = err; });
            },
            [&](Void) { state.resolved = none; });
        });
    },
    [](auto &) {}, [](auto &) {});

  src_ = none;

  return ViewState{};
}

ViewLayout Image::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  Vec2 const frame = style.frame(allocated);

  if (style.aspect_ratio.is_none())
  {
    return ViewLayout{.extent = frame};
  }

  return ViewLayout{.extent = with_aspect(frame, style.aspect_ratio.value())};
}

static Tuple<Vec2, Vec2, Vec2> fit_image(Vec2 extent, Vec2 region_extent,
                                         ImageFit fit)
{
  switch (fit)
  {
    case ImageFit::Crop:
    {
      Vec2 const ar        = {extent.x / extent.y, 1};
      f32 const  dst_ar    = region_extent.x / region_extent.y;
      Vec2 const uv_extent = with_aspect(ar, dst_ar) / ar;
      Vec2 const space     = (1 - uv_extent) * 0.5F;
      return {region_extent, space, 1 - space};
    }
    case ImageFit::Fit:
    {
      return {
        region_extent, {0, 0},
         {1, 1}
      };
    }
    default:
    case ImageFit::Contain:
    {
      f32 const ar = extent.x / extent.y;
      return {
        with_aspect(region_extent, ar), {0, 0},
          {1, 1}
      };
    }
  }
}

static void render_image(Canvas & canvas, CRect const & region,
                         ash::ImageInfo const & img, Image::Style const & style)
{
  auto const [extent, uv0, uv1] =
    fit_image(as_vec2(img.info.extent.xy()), region.extent, style.fit);

  Vec2 const center = space_align(region.extent, extent, style.alignment);

  canvas.rrect(ShapeInfo{
    .center       = region.center + center,
    .extent       = extent,
    .corner_radii = style.radii,
    .tint         = style.tint,
    .sampler      = SamplerId::LinearClamped,
    .texture      = img.textures[0],
    .uv{uv0, uv1}
  });
}

void Image::render(Canvas & canvas, CRect const & region, f32, Rect const &)
{
  state.resolved.match(
    [&](None) {},
    [&](Option<ash::ImageInfo> & opt) {
      opt.match(
        [&](ash::ImageInfo & img) { render_image(canvas, region, img, style); },
        []() {});
    },
    [&](ImageLoadErr) {});
}

}    // namespace ui
}    // namespace ash
