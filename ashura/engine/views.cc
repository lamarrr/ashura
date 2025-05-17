/// SPDX-License-Identifier: MIT
#include "ashura/engine/views.h"
#include "ashura/engine/engine.h"
#include "ashura/std/sformat.h"
#include "fast_float/fast_float.h"

namespace ash
{

namespace ui
{

Space & Space::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

Space & Space::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Layout Space::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return Layout{.extent = style_.frame(allocated)};
}

Flex::Flex(AllocatorRef allocator) : items_{allocator}
{
}

Flex & Flex::axis(Axis a)
{
  style_.axis = a;
  return *this;
}

Flex & Flex::wrap(bool w)
{
  style_.wrap = w;
  return *this;
}

Flex & Flex::main_align(MainAlign align)
{
  style_.main_align = align;
  return *this;
}

Flex & Flex::cross_align(f32 align)
{
  style_.cross_align = align;
  return *this;
}

Flex & Flex::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Flex & Flex::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Flex & Flex::item_frame(Vec2 extent, bool constrain)
{
  style_.item_frame = Frame{extent, constrain};
  return *this;
}

Flex & Flex::item_frame(Frame f)
{
  style_.item_frame = f;
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

ui::State Flex::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  for (ref item : items_)
  {
    build(item);
  }

  return ui::State{};
}

void Flex::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 const frame = style_.frame(allocated);
  fill(sizes, style_.item_frame(frame));
}

Layout Flex::fit(Vec2 allocated, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  auto const n            = sizes.size();
  Vec2 const frame        = style_.frame(allocated);
  u32 const  main_axis    = (style_.axis == Axis::X) ? 0 : 1;
  u32 const  cross_axis   = (style_.axis == Axis::X) ? 1 : 0;
  Vec2       span         = {};
  f32        cross_cursor = 0;

  for (usize i = 0; i < n;)
  {
    auto first        = i++;
    f32  main_extent  = sizes[first][main_axis];
    f32  cross_extent = sizes[first][cross_axis];
    f32  main_spacing = 0;

    while (i < n && !(style_.wrap &&
                      (main_extent + sizes[i][main_axis]) > frame[main_axis]))
    {
      main_extent += sizes[i][main_axis];
      cross_extent = max(cross_extent, sizes[i][cross_axis]);
      i++;
    }

    auto const count = i - first;

    if (style_.main_align != MainAlign::Start)
    {
      main_spacing = max(frame[main_axis] - main_extent, 0.0F);
    }

    for (auto [center, size] :
         zip(centers.slice(first, count), sizes.slice(first, count)))
    {
      f32 const pos =
        space_align(cross_extent, size[cross_axis], style_.cross_align);
      center[cross_axis] = cross_cursor + cross_extent * 0.5F + pos;
    }

    switch (style_.main_align)
    {
      case MainAlign::Start:
      {
        f32 main_spacing_cursor = 0;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis];
        }
      }
      break;

      case MainAlign::SpaceAround:
      {
        f32 spacing             = main_spacing / (count * 2);
        f32 main_spacing_cursor = 0;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          main_spacing_cursor += spacing;
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis] + spacing;
        }
      }
      break;

      case MainAlign::SpaceBetween:
      {
        f32 spacing             = main_spacing / (count - 1);
        f32 main_spacing_cursor = 0;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis] + spacing;
        }
      }
      break;

      case MainAlign::SpaceEvenly:
      {
        f32 spacing             = main_spacing / (count + 1);
        f32 main_spacing_cursor = spacing;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis] + spacing;
        }
      }
      break;

      case MainAlign::End:
      {
        f32 main_spacing_cursor = main_spacing;
        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
          center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
          main_spacing_cursor += size[main_axis];
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
  for (auto & center : centers)
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
  style_.reverse = r;
  return *this;
}

Stack & Stack::align(Vec2 a)
{
  style_.alignment = a;
  return *this;
}

Stack & Stack::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Stack & Stack::frame(Frame f)
{
  style_.frame = f;
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
  if (!style_.reverse)
  {
    return base + (i32) i;
  }
  else
  {
    return base + (i32) (num - i);
  }
}

ui::State Stack::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  for (ref item : items_)
  {
    build(item);
  }

  return ui::State{};
}

void Stack::size(Vec2 allocated, Span<Vec2> sizes)
{
  fill(sizes, style_.frame(allocated));
}

Layout Stack::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  Vec2 span;

  for (Vec2 style : sizes)
  {
    span.x = max(span.x, style.x);
    span.y = max(span.y, style.y);
  }

  for (auto [center, size] : zip(centers, sizes))
  {
    center = space_align(span, size, style_.alignment);
  }

  return {.extent = span};
}

i32 Stack::z_index(i32 allocated, Span<i32> indices)
{
  auto n = indices.size();
  for (auto [i, stack_index] : enumerate(indices))
  {
    stack_index = stack_item(allocated, i, n);
  }

  return allocated;
}

struct TextCfg
{
  bool multiline_input : 1 = false;
  bool enter_submits   : 1 = false;
  bool tab_input       : 1 = false;
  bool copyable        : 1 = false;
  bool editable        : 1 = false;
  bool highlightable   : 1 = false;
};

TextCommand text_command(Ctx const & ctx, Events const & events,
                         TextCfg const & cfg)
{
  if (events.focus_out())
  {
    return TextCommand::Escape;
  }

  if (cfg.editable && events.text_input())
  {
    return TextCommand::InputText;
  }

  auto const shift = ctx.key.held(KeyModifiers::LeftShift) ||
                     ctx.key.held(KeyModifiers::RightShift);
  auto const ctrl = ctx.key.held(KeyModifiers::LeftCtrl) ||
                    ctx.key.held(KeyModifiers::RightCtrl);

  if (events.key_down())
  {
    if (cfg.highlightable)
    {
      if (shift && ctx.key.down(KeyCode::Left))
      {
        return TextCommand::SelectLeft;
      }

      if (shift && ctx.key.down(KeyCode::Right))
      {
        return TextCommand::SelectRight;
      }

      if (shift && ctx.key.down(KeyCode::Up))
      {
        return TextCommand::SelectUp;
      }

      if (shift && ctx.key.down(KeyCode::Down))
      {
        return TextCommand::SelectDown;
      }

      if (shift && ctx.key.down(KeyCode::PageUp))
      {
        return TextCommand::SelectPageUp;
      }

      if (shift && ctx.key.down(KeyCode::PageDown))
      {
        return TextCommand::SelectPageDown;
      }

      if (ctrl && ctx.key.down(KeyCode::A))
      {
        return TextCommand::SelectAll;
      }

      if (ctx.key.down(KeyCode::Escape))
      {
        return TextCommand::Unselect;
      }
    }

    if (cfg.editable)
    {
      if (ctrl && ctx.key.down(KeyCode::X))
      {
        return TextCommand::Cut;
      }

      if (cfg.copyable && ctrl && ctx.key.down(KeyCode::C))
      {
        return TextCommand::Copy;
      }

      if (ctrl && ctx.key.down(KeyCode::V))
      {
        return TextCommand::Paste;
      }

      if (ctrl && ctx.key.down(KeyCode::Z))
      {
        return TextCommand::Undo;
      }

      if (ctrl && ctx.key.down(KeyCode::Y))
      {
        return TextCommand::Redo;
      }

      if (cfg.multiline_input && !cfg.enter_submits &&
          ctx.key.down(KeyCode::Return))
      {
        return TextCommand::NewLine;
      }

      if (cfg.tab_input && ctx.key.down(KeyCode::Tab))
      {
        return TextCommand::Tab;
      }

      if (ctx.key.down(KeyCode::Backspace))
      {
        return TextCommand::BackSpace;
      }

      if (ctx.key.down(KeyCode::Delete))
      {
        return TextCommand::Delete;
      }

      if (ctx.key.down(KeyCode::Left))
      {
        return TextCommand::Left;
      }

      if (ctx.key.down(KeyCode::Right))
      {
        return TextCommand::Right;
      }

      if (ctx.key.down(KeyCode::Home))
      {
        return TextCommand::LineStart;
      }

      if (ctx.key.down(KeyCode::End))
      {
        return TextCommand::LineEnd;
      }

      if (ctx.key.down(KeyCode::Up))
      {
        return TextCommand::Up;
      }

      if (ctx.key.down(KeyCode::Down))
      {
        return TextCommand::Down;
      }

      if (ctx.key.down(KeyCode::PageUp))
      {
        return TextCommand::PageUp;
      }

      if (ctx.key.down(KeyCode::PageDown))
      {
        return TextCommand::PageDown;
      }
    }

    if (cfg.enter_submits && ctx.key.down(KeyCode::Return))
    {
      return TextCommand::Submit;
    }
  }

  if (events.drag_start())
  {
    return TextCommand::Hit;
  }
  else if (events.drag_update())
  {
    if (cfg.highlightable)
    {
      if (ctx.mouse.down(MouseButton::Primary) &&
          ctx.mouse.clicks(MouseButton::Primary) == 2)
      {
        return TextCommand::SelectWord;
      }

      if (ctx.mouse.down(MouseButton::Primary) &&
          ctx.mouse.clicks(MouseButton::Primary) == 3)
      {
        return TextCommand::SelectLine;
      }

      if (ctx.mouse.down(MouseButton::Primary) &&
          ctx.mouse.clicks(MouseButton::Primary) == 4)
      {
        return TextCommand::SelectAll;
      }

      return TextCommand::HitSelect;
    }
  }
  else if (events.focus_out())
  {
    if (cfg.highlightable)
    {
      return TextCommand::Unselect;
    }
  }

  return TextCommand::None;
}

Text::Text(Str32 t, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  text(t).run(style, font);
}

Text::Text(Str8 t, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  text(t).run(style, font);
}

Text & Text::copyable(bool allow)
{
  state_.copyable = allow;
  return *this;
}

Text & Text::highlight_style(TextHighlightStyle highlight)
{
  style_.highlight = highlight;
  return *this;
}

Text & Text::run(TextStyle const & style, FontStyle const & font, usize first,
                 usize count)
{
  text_.run(style, font, first, count);
  return *this;
}

Text & Text::text(Str32 t)
{
  text_.text(t);
  return *this;
}

Text & Text::text(Str8 t)
{
  text_.text(t);
  return *this;
}

Str32 Text::text() const
{
  return text_.get_text();
}

ui::State Text::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  TextCommand cmd = text_command(ctx, events,
                                 TextCfg{.multiline_input = false,
                                         .enter_submits   = false,
                                         .tab_input       = false,
                                         .copyable        = state_.copyable,
                                         .editable        = false,
                                         .highlightable   = state_.copyable});

  auto hit_info = events.hit_info.map([](auto s) { return s; }).unwrap_or();

  bool modified = compositor_.command(
    text_, cmd, {}, engine->clipboard, 1, 1, hit_info.canvas_region.center,
    hit_info.viewport_region.extent.x, hit_info.viewport_hit,
    scale3d(vec3(hit_info.zoom(), 1)), default_allocator);
  CHECK(!modified, "");

  // [ ] copyable for input
  text_.clear_highlights()
    .add_highlight(compositor_.cursor().selection())
    .highlight_style(style_.highlight);

  return ui::State{.draggable = state_.copyable};
}

Layout Text::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  text_.layout(allocated.x);
  return {.extent = text_.layout_.extent};
}

void Text::render(Canvas & canvas, CRect const & viewport_region,
                  CRect const & canvas_region, CRect const & clip)
{
  text_.render(
    canvas.text_renderer(), canvas_region.center, viewport_region.extent.x,
    scale3d(vec3(canvas_region.extent / viewport_region.extent, 1)), clip);
}

Cursor Text::cursor(Vec2, Vec2)
{
  return state_.copyable ? Cursor::Text : Cursor::Default;
}

Input::Input(Str32 s, TextStyle const & style, FontStyle const & font,
             AllocatorRef allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  content(U""_str).content_run(style, font).stub(s).stub_run(style, font);
}

Input::Input(Str8 s, TextStyle const & style, FontStyle const & font,
             AllocatorRef allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  content(U""_str).content_run(style, font).stub(s).stub_run(style, font);
}

Input & Input::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Input & Input::multiline(bool e)
{
  state_.multiline = e;
  return *this;
}

Input & Input::enter_submits(bool e)
{
  state_.enter_submits = e;
  return *this;
}

Input & Input::tab_input(bool e)
{
  state_.tab_input = e;
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

Input & Input::content(Str8 t)
{
  content_.text(t);
  return *this;
}

Input & Input::content(Str32 t)
{
  content_.text(t);
  return *this;
}

Input & Input::content_run(TextStyle const & style, FontStyle const & font,
                           usize first, usize count)
{
  content_.run(style, font, first, count);
  return *this;
}

Input & Input::stub(Str8 t)
{
  stub_.text(t);
  return *this;
}

Input & Input::stub(Str32 t)
{
  stub_.text(t);
  return *this;
}

Input & Input::stub_run(TextStyle const & style, FontStyle const & font,
                        usize first, usize count)
{
  stub_.run(style, font, first, count);
  return *this;
}

ui::State Input::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  bool edited = false;

  state_.editing = false;
  state_.submit  = false;

  u8 buffer[512];

  FallbackAllocator allocator{Arena::from(buffer), allocator_};

  Vec<c32> input_u32{allocator};

  if (events.text_input())
  {
    utf8_decode(ctx.key.text, input_u32).unwrap();
  }

  TextCommand cmd = text_command(ctx, events,
                                 TextCfg{.multiline_input = state_.multiline,
                                         .enter_submits = state_.enter_submits,
                                         .tab_input     = state_.tab_input,
                                         .copyable      = true,
                                         .editable      = true,
                                         .highlightable = true});

  auto hit_info = events.hit_info.map([](auto s) { return s; }).unwrap_or();

  bool modified = compositor_.command(
    content_, cmd, input_u32, engine->clipboard, style_.lines_per_page,
    style_.tab_width, hit_info.canvas_region.center,
    hit_info.viewport_region.extent.x, hit_info.viewport_hit,
    scale3d(vec3(hit_info.zoom(), 1)), allocator);

  auto cursor = compositor_.cursor();

  content_.clear_highlights()
    .clear_carets()
    .add_highlight(cursor.selection())
    .highlight_style(style_.highlight)
    .caret_style(style_.caret);

  if (events.focus_over())
  {
    content_.add_caret(cursor.caret());
  }

  if (modified)
  {
    content_.flush_text();
  }

  if (edited)
  {
    state_.editing = true;
  }

  if (cmd == TextCommand::Submit)
  {
    state_.submit = true;
    cb.submit();
  }

  if (events.focus_in())
  {
    cb.focus_in();
  }

  if (events.focus_out())
  {
    cb.focus_out();
  }

  if (edited)
  {
    cb.edit();
  }

  return ui::State{
    .text       = TextInputInfo{.multiline = state_.multiline,
                                .tab_input = state_.tab_input},
    .draggable  = !state_.disabled,
    .focusable  = !state_.disabled,
    .grab_focus = events.pointer_down()
  };
}

Layout Input::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  content_.layout(allocated.x);
  stub_.layout(allocated.x);
  if (content_.text_.is_empty())
  {
    return {.extent = stub_.layout_.extent};
  }
  return {.extent = content_.layout_.extent};
}

void Input::render(Canvas & canvas, CRect const & viewport_region,
                   CRect const & canvas_region, CRect const & clip)
{
  if (content_.text_.is_empty())
  {
    // [ ] ellipsis; ellipsis-wrap on max-lines; LTR & RTL-sensitive
    // [ ] do not layout paragraph if the text break on clip or ellipsis??
    stub_.render(
      canvas.text_renderer(), canvas_region.center, viewport_region.extent.x,
      scale3d(vec3(canvas_region.extent / viewport_region.extent, 1)), clip);
  }
  else
  {
    // [ ] need to draw caret even if line is empty; SET placeholder caret to 0;  use place holder when focused
    content_.render(
      canvas.text_renderer(), canvas_region.center, viewport_region.extent.x,
      scale3d(vec3(canvas_region.extent / viewport_region.extent, 1)), clip);
  }
}

Cursor Input::cursor(Vec2, Vec2)
{
  return Cursor::Text;
}

Button & Button::disable(bool d)
{
  state_.disabled = d;
  return *this;
}

Button & Button::color(Vec4U8 c)
{
  style_.color = c;
  return *this;
}

Button & Button::hovered_color(Vec4U8 c)
{
  style_.hovered_color = c;
  return *this;
}

Button & Button::disabled_color(Vec4U8 c)
{
  style_.disabled_color = c;
  return *this;
}

Button & Button::rrect(CornerRadii const & c)
{
  style_.corner_radii = c;
  style_.shape        = ButtonShape::RRect;
  return *this;
}

Button & Button::squircle(f32 degree)
{
  // [ ] fix shape for button
  style_.corner_radii = CornerRadii{degree, degree, degree, degree};
  style_.shape        = ButtonShape::Squircle;
  return *this;
}

Button & Button::bevel(CornerRadii const & c)
{
  style_.corner_radii = c;
  style_.shape        = ButtonShape::Bevel;
  return *this;
}

Button & Button::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Button & Button::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Button & Button::stroke(f32 stroke)
{
  style_.stroke = stroke;
  return *this;
}

Button & Button::thickness(f32 thickness)
{
  style_.thickness = thickness;
  return *this;
}

Button & Button::padding(Vec2 p)
{
  style_.padding = p;
  return *this;
}

Button & Button::on_pressed(Fn<void()> f)
{
  cb.pressed = f;
  return *this;
}

Button & Button::on_hovered(Fn<void()> f)
{
  cb.hovered = f;
  return *this;
}

ui::State Button::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_over())
  {
    cb.hovered();
  }

  if (events.pointer_down() ||
      (events.focus_over() && ctx.key.down(KeyCode::Return)))
  {
    cb.pressed();
  }

  state_.held = events.pointer_over() && ctx.mouse.held(MouseButton::Primary);
  state_.hovered = events.pointer_over();

  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void Button::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 const frame = style_.frame(allocated);
  Vec2       size  = frame - style_.padding * 2;
  size.x           = max(size.x, 0.0F);
  size.y           = max(size.y, 0.0F);
  fill(sizes, size);
}

Layout Button::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  fill(centers, Vec2{0, 0});
  Vec2 size = sizes.is_empty() ? Vec2{0, 0} : sizes[0];
  return {.extent = size + 2 * style_.padding};
}

void Button::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                    CRect const &)
{
  Vec4U8 tint;

  if (state_.disabled)
  {
    tint = style_.disabled_color;
  }
  else if (state_.hovered && !state_.held)
  {
    tint = style_.hovered_color;
  }
  else
  {
    tint = style_.color;
  }

  switch (style_.shape)
  {
    case ButtonShape::RRect:
      canvas.rrect({.area         = canvas_region,
                    .corner_radii = style_.corner_radii,
                    .stroke       = style_.stroke,
                    .thickness    = style_.thickness,
                    .tint         = tint});
      break;
    case ButtonShape::Squircle:
      canvas.squircle({.area         = canvas_region,
                       .corner_radii = style_.corner_radii,
                       .stroke       = style_.stroke,
                       .thickness    = style_.thickness,
                       .tint         = tint});
      break;
    case ButtonShape::Bevel:
      canvas.brect({.area         = canvas_region,
                    .corner_radii = style_.corner_radii,
                    .stroke       = style_.stroke,
                    .thickness    = style_.thickness,
                    .tint         = tint});
      break;
    default:
      break;
  }
}

Cursor Button::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

TextButton::TextButton(Str32 text, TextStyle const & style,
                       FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
}

TextButton::TextButton(Str8 text, TextStyle const & style,
                       FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
}

TextButton & TextButton::disable(bool d)
{
  Button::disable(d);
  return *this;
}

TextButton & TextButton::run(TextStyle const & style, FontStyle const & font,
                             usize first, usize count)
{
  text_.run(style, font, first, count);
  return *this;
}

TextButton & TextButton::text(Str32 t)
{
  text_.text(t);
  return *this;
}

TextButton & TextButton::text(Str8 t)
{
  text_.text(t);
  return *this;
}

TextButton & TextButton::color(Vec4U8 c)
{
  Button::color(c);
  return *this;
}

TextButton & TextButton::hovered_color(Vec4U8 c)
{
  Button::color(c);
  return *this;
}

TextButton & TextButton::disabled_color(Vec4U8 c)
{
  Button::color(c);
  return *this;
}

TextButton & TextButton::rrect(CornerRadii const & c)
{
  Button::rrect(c);
  return *this;
}

TextButton & TextButton::squircle(f32 degree)
{
  Button::squircle(degree);
  return *this;
}

TextButton & TextButton::bevel(CornerRadii const & c)
{
  Button::bevel(c);
  return *this;
}

TextButton & TextButton::frame(Vec2 extent, bool constrain)
{
  Button::frame(extent, constrain);
  return *this;
}

TextButton & TextButton::frame(Frame f)
{
  Button::frame(f);
  return *this;
}

TextButton & TextButton::stroke(f32 stroke)
{
  Button::stroke(stroke);
  return *this;
}

TextButton & TextButton::thickness(f32 thickness)
{
  Button::thickness(thickness);
  return *this;
}

TextButton & TextButton::padding(Vec2 p)
{
  Button::padding(p);
  return *this;
}

TextButton & TextButton::on_pressed(Fn<void()> f)
{
  Button::on_pressed(f);
  return *this;
}

TextButton & TextButton::on_hovered(Fn<void()> f)
{
  Button::on_hovered(f);
  return *this;
}

ui::State TextButton::tick(Ctx const & ctx, Events const & events,
                           Fn<void(View &)> build)
{
  ui::State state_ = Button::tick(ctx, events, build);
  build(text_);
  return state_;
}

Icon::Icon(Str32 text, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator}
{
  text_.text(text).run(style, font);
}

Icon::Icon(Str8 text, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator}
{
  text_.text(text).run(style, font);
}

Icon & Icon::hide(bool hide)
{
  state_.hidden = hide;
  return *this;
}

Icon & Icon::icon(Str8 text, TextStyle const & style, FontStyle const & font)
{
  text_.text(text).run(style, font);
  return *this;
}

Icon & Icon::icon(Str32 text, TextStyle const & style, FontStyle const & font)
{
  text_.text(text).run(style, font);
  return *this;
}

ui::State Icon::tick(Ctx const &, Events const &, Fn<void(View &)>)
{
  return ui::State{.hidden = state_.hidden};
}

Layout Icon::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  text_.layout(allocated.x);
  return Layout{.extent = text_.get_layout().extent};
}

void Icon::render(Canvas & canvas, CRect const & viewport_region,
                  CRect const & canvas_region, CRect const & clip)
{
  text_.render(
    canvas.text_renderer(), canvas_region.center, viewport_region.extent.x,
    scale3d(vec3(canvas_region.extent / viewport_region.extent, 1)), clip);
}

CheckBox::CheckBox(Str32 text, TextStyle const & style, FontStyle const & font,
                   AllocatorRef allocator) :
  icon_{text, style, font, allocator}
{
}

CheckBox::CheckBox(Str8 text, TextStyle const & style, FontStyle const & font,
                   AllocatorRef allocator) :
  icon_{text, style, font, allocator}
{
}

Icon & CheckBox::icon()
{
  return icon_;
}

CheckBox & CheckBox::disable(bool d)
{
  state_.disabled = d;
  return *this;
}

CheckBox & CheckBox::box_color(Vec4U8 c)
{
  style_.box_color = c;
  return *this;
}

CheckBox & CheckBox::box_hovered_color(Vec4U8 c)
{
  style_.box_hovered_color = c;
  return *this;
}

CheckBox & CheckBox::stroke(f32 s)
{
  style_.stroke = s;
  return *this;
}

CheckBox & CheckBox::thickness(f32 t)
{
  style_.thickness = t;
  return *this;
}

CheckBox & CheckBox::corner_radii(CornerRadii const & r)
{
  style_.corner_radii = r;
  return *this;
}

CheckBox & CheckBox::padding(f32 p)
{
  style_.padding = p;
  return *this;
}

CheckBox & CheckBox::on_changed(Fn<void(bool)> f)
{
  cb.changed = f;
  return *this;
}

ui::State CheckBox::tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build)
{
  if (events.pointer_down() ||
      (events.focus_over() && ctx.key.down(KeyCode::Return)))
  {
    state_.value = !state_.value;
    cb.changed(state_.value);
  }

  icon_.hide(!state_.value);

  build(icon_);

  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void CheckBox::size(Vec2 allocated, Span<Vec2> sizes)
{
  fill(sizes, allocated - 2 * style_.padding);
}

Layout CheckBox::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  fill(centers, Vec2{});
  return {.extent = style_.padding + sizes[0]};
}

void CheckBox::render(Canvas &      canvas, CRect const &,
                      CRect const & canvas_region, CRect const &)
{
  Vec4U8 tint;
  if (state_.hovered && !state_.held && !state_.disabled)
  {
    tint = style_.box_hovered_color;
  }
  else
  {
    tint = style_.box_color;
  }

  canvas.rrect({.area         = canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = 1,
                .thickness    = style_.thickness,
                .tint         = tint});
}

Cursor CheckBox::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

Slider & Slider::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Slider & Slider::range(f32 low, f32 high)
{
  state_.low  = low;
  state_.high = high;
  return *this;
}

Slider & Slider::interp(f32 t)
{
  state_.t = t;
  return *this;
}

Slider & Slider::axis(Axis a)
{
  style_.axis = a;
  return *this;
}

Slider & Slider::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Slider & Slider::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Slider & Slider::thumb_size(f32 size)
{
  style_.thumb_size = size;
  return *this;
}

Slider & Slider::track_size(f32 size)
{
  style_.track_size = size;
  return *this;
}

Slider & Slider::thumb_color(Vec4U8 c)
{
  style_.thumb_color = c;
  return *this;
}

Slider & Slider::thumb_hovered_color(Vec4U8 c)
{
  style_.thumb_hovered_color = c;
  return *this;
}

Slider & Slider::thumb_dragging_color(Vec4U8 c)
{
  style_.thumb_dragging_color = c;
  return *this;
}

Slider & Slider::thumb_corner_radii(CornerRadii const & c)
{
  style_.thumb_corner_radii = c;
  return *this;
}

Slider & Slider::track_color(Vec4U8 c)
{
  style_.track_color = c;
  return *this;
}

Slider & Slider::track_corner_radii(CornerRadii const & c)
{
  style_.track_corner_radii = c;
  return *this;
}

Slider & Slider::on_changed(Fn<void(f32)> f)
{
  cb.changed = f;
  return *this;
}

ui::State Slider::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  u32 const main_axis = (style_.axis == Axis::X) ? 0 : 1;

  if (events.drag_update())
  {
    auto      h = events.hit_info.unwrap_or();
    f32 const thumb_begin =
      h.viewport_region.begin()[main_axis] + style_.thumb_size * 0.5F;
    f32 const thumb_end =
      h.viewport_region.end()[main_axis] - style_.thumb_size * 0.5F;
    state_.t = clamp(unlerp(thumb_begin, thumb_end, h.viewport_hit[main_axis]),
                     0.0F, 1.0F);
    f32 const value =
      clamp(lerp(state_.low, state_.high, state_.t), state_.low, state_.high);
    cb.changed(value);
  }

  if (events.focus_over())
  {
    if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Left)) ||
        (style_.axis == Axis::Y && ctx.key.down(KeyCode::Up)))
    {
      state_.t = max(state_.t - style_.delta, 0.0F);
    }
    else if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Right)) ||
             (style_.axis == Axis::Y && ctx.key.down(KeyCode::Down)))
    {
      state_.t = min(state_.t + style_.delta, 1.0F);
    }
  }

  state_.dragging = events.drag_update();
  state_.hovered  = events.pointer_over();

  return ui::State{.pointable = !state_.disabled,
                   .draggable = !state_.disabled,
                   .focusable = !state_.disabled};
}

Layout Slider::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style_.frame(allocated)};
}

void Slider::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                    CRect const &)
{
  u32 const main_axis  = (style_.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style_.axis == Axis::Y) ? 0 : 1;

  Vec4U8 thumb_color;

  if (state_.dragging)
  {
    thumb_color = style_.thumb_dragging_color;
  }
  else if (state_.hovered)
  {
    thumb_color = style_.thumb_hovered_color;
  }
  else
  {
    thumb_color = style_.thumb_color;
  }

  f32 dilation = 1.0F;

  if (state_.dragging || state_.hovered)
  {
    dilation = 1.0F;
  }
  else
  {
    dilation = 0.8F;
  }

  f32 const thumb_begin =
    canvas_region.begin()[main_axis] + style_.thumb_size * 0.5F;
  f32 const thumb_end =
    canvas_region.end()[main_axis] - style_.thumb_size * 0.5F;
  f32 const thumb_center = lerp(thumb_begin, thumb_end, state_.t);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = canvas_region.center[cross_axis];
  thumb_rect.extent             = Vec2::splat(style_.thumb_size);

  CRect track_rect;

  track_rect.center             = canvas_region.center;
  track_rect.extent[main_axis]  = thumb_end - thumb_begin;
  track_rect.extent[cross_axis] = style_.track_size;

  Vec2 coverage_begin;
  coverage_begin[main_axis]  = thumb_begin;
  coverage_begin[cross_axis] = track_rect.begin()[cross_axis];

  Vec2 coverage_end;
  coverage_end[main_axis]  = thumb_center;
  coverage_end[cross_axis] = track_rect.end()[cross_axis];

  CRect const coverage_rect = CRect::range(coverage_begin, coverage_end);

  canvas
    .rrect({
      .area         = track_rect,
      .corner_radii = style_.track_corner_radii,
      .tint         = style_.track_color
  })
    .rrect({.area         = coverage_rect,
            .corner_radii = style_.track_corner_radii,
            .tint         = thumb_color})
    .rrect({.area{thumb_rect.center, thumb_rect.extent * dilation},
            .corner_radii = style_.thumb_corner_radii * dilation,
            .tint         = thumb_color});
}

Cursor Slider::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

Switch & Switch::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Switch & Switch::on()
{
  state_.value = true;
  cb.changed(true);
  return *this;
}

Switch & Switch::off()
{
  state_.value = false;
  cb.changed(false);
  return *this;
}

Switch & Switch::toggle()
{
  if (state_.value)
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
  style_.on_color = c;
  return *this;
}

Switch & Switch::on_hovered_color(Vec4U8 c)
{
  style_.on_hovered_color = c;
  return *this;
}

Switch & Switch::off_color(Vec4U8 c)
{
  style_.off_color = c;
  return *this;
}

Switch & Switch::off_hovered_color(Vec4U8 c)
{
  style_.off_hovered_color = c;
  return *this;
}

Switch & Switch::track_color(Vec4U8 c)
{
  style_.track_color = c;
  return *this;
}

Switch & Switch::corner_radii(CornerRadii const & r)
{
  style_.corner_radii = r;
  return *this;
}

Switch & Switch::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Switch & Switch::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

ui::State Switch::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_down() ||
      (events.focus_over() && ctx.key.down(KeyCode::Return)))
  {
    state_.value = !state_.value;
    cb.changed(state_.value);
  }

  state_.hovered = events.pointer_over();

  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

Layout Switch::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style_.frame(allocated)};
}

void Switch::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                    CRect const &)
{
  Vec2 thumb_extent = canvas_region.extent;
  thumb_extent.x *= 0.5F;
  Vec2 const alignment{state_.value ? ALIGNMENT_RIGHT : ALIGNMENT_LEFT,
                       ALIGNMENT_CENTER};
  Vec2 const thumb_center =
    canvas_region.center +
    space_align(canvas_region.extent, thumb_extent, alignment);

  Vec4U8 thumb_color;
  if (state_.hovered)
  {
    thumb_color =
      state_.value ? style_.on_hovered_color : style_.off_hovered_color;
  }
  else
  {
    thumb_color = state_.value ? style_.on_color : style_.off_color;
  }

  canvas
    .rrect({
      .area         = canvas_region,
      .corner_radii = style_.corner_radii,
      .tint         = style_.track_color
  })
    .rrect({.area{thumb_center, thumb_extent},
            .corner_radii = style_.corner_radii,
            .tint         = thumb_color});
}

Cursor Switch::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

Radio & Radio::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Radio & Radio::corner_radii(CornerRadii const & c)
{
  style_.corner_radii = c;
  return *this;
}

Radio & Radio::thickness(f32 t)
{
  style_.thickness = t;
  return *this;
}

Radio & Radio::color(Vec4U8 c)
{
  style_.color = c;
  return *this;
}

Radio & Radio::inner_color(Vec4U8 c)
{
  style_.inner_color = c;
  return *this;
}

Radio & Radio::inner_hovered_color(Vec4U8 c)
{
  style_.inner_hovered_color = c;
  return *this;
}

Radio & Radio::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Radio & Radio::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Radio & Radio::on_changed(Fn<void(bool)> f)
{
  cb.changed = f;
  return *this;
}

ui::State Radio::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_down() ||
      (events.focus_over() && ctx.key.down(KeyCode::Return)))
  {
    state_.value = !state_.value;
    cb.changed(state_.value);
  }

  state_.hovered = events.pointer_over();

  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

Layout Radio::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style_.frame(allocated)};
}

void Radio::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                   CRect const &)
{
  canvas.rrect({.area         = canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = 1,
                .thickness    = style_.thickness,
                .tint         = style_.color});

  if (state_.value)
  {
    Vec2 inner_extent = canvas_region.extent * (state_.hovered ? 0.75F : 0.5F);
    Vec4U8 inner_color =
      state_.hovered ? style_.inner_hovered_color : style_.inner_color;

    canvas.circle({
      .area{canvas_region.center, inner_extent},
      .tint = inner_color
    });
  }
}

Cursor Radio::cursor(Vec2, Vec2)
{
  return Cursor::Pointer;
}

ScalarDragBox::ScalarDragBox(TextStyle const & style, FontStyle const & font,
                             AllocatorRef allocator) :
  input_{U""_str, style, font, allocator}
{
  input_.multiline(false).tab_input(false).enter_submits(false);
}

void ScalarDragBox::scalar_parse(Str32 text, ScalarInfo const & spec,
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

void ScalarDragBox::format_()
{
  u8                buffer[1'024];
  FallbackAllocator allocator{Arena::from(buffer), default_allocator};
  sformat(allocator, style_.format, state_.scalar)
    .match([&](auto & text) { input_.content(text.view().as_c8()); },
           [&](auto &) { input_.content(U"[Truncated]"_str); });
}

ScalarDragBox & ScalarDragBox::on_update(Fn<void(Scalar)> fn)
{
  cb.update = fn;
  return *this;
}

ui::State ScalarDragBox::tick(Ctx const & ctx, Events const & events,
                              Fn<void(View &)> build)
{
  state_.dragging = events.drag_update();

  // [ ] fix input

  if (events.drag_start() &&
      (ctx.key.down(KeyCode::LeftCtrl) || ctx.key.down(KeyCode::RightCtrl)))
  {
    state_.input_mode = !state_.input_mode;
  }

  if (state_.dragging && !state_.input_mode)
  {
    auto      h = events.hit_info.unwrap_or();
    f32 const t = clamp(unlerp(h.viewport_region.begin().x,
                               h.viewport_region.end().x, h.viewport_hit.x),
                        0.0F, 1.0F);
    state_.scalar =
      state_.spec.match([t](F32Info & v) -> Scalar { return v.interp(t); },
                        [t](I32Info & v) -> Scalar { return v.interp(t); });

    format_();
    cb.update(state_.scalar);
  }
  else if (input_.state_.editing)
  {
    scalar_parse(input_.content_.get_text(), state_.spec, state_.scalar);
    cb.update(state_.scalar);
  }

  input_.state_.disabled = !state_.input_mode;

  build(input_);

  return ui::State{.pointable = !state_.disabled,
                   .draggable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void ScalarDragBox::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 child = style_.frame(allocated) - 2 * style_.padding;
  child.x    = max(child.x, 0.0F);
  child.y    = max(child.y, 0.0F);
  fill(sizes, child);
}

Layout ScalarDragBox::fit(Vec2 allocated, Span<Vec2 const> sizes,
                          Span<Vec2> centers)
{
  Vec2 frame         = style_.frame(allocated);
  Vec2 padded_extent = sizes[0] + 2 * style_.padding;
  frame.x            = max(frame.x, padded_extent.x);
  frame.y            = max(frame.y, padded_extent.y);
  fill(centers, Vec2{0, 0});

  return {.extent = frame};
}

void ScalarDragBox::render(Canvas &      canvas, CRect const &,
                           CRect const & canvas_region, CRect const &)
{
  canvas.rrect({.area         = canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = style_.stroke,
                .thickness    = style_.thickness,
                .tint         = style_.color});

  if (!state_.input_mode)
  {
    f32 const t = state_.spec.match(
      [&](F32Info & v) { return v.uninterp(state_.scalar[v0]); },
      [&](I32Info & v) { return v.uninterp(state_.scalar[v1]); });

    CRect const thumb_rect = CRect::from_offset(
      canvas_region.begin(), canvas_region.extent * Vec2{t, 1});

    canvas.rrect({.area         = thumb_rect,
                  .corner_radii = style_.corner_radii,
                  .tint         = style_.thumb_color});
  }
}

Cursor ScalarDragBox::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::EWResize;
}

ScalarBox::ScalarBox(Str32 decrease_text, Str32 increase_text,
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

  dec_.on_pressed({this, [](ScalarBox * b) { b->step(-1); }});

  inc_.on_pressed({this, [](ScalarBox * b) { b->step(1); }});

  padding({5, 5}).corner_radii(CornerRadii::all(7.5F));

  drag_.on_update({this, +[](ScalarBox * b, Scalar in) { b->cb.update(in); }});
}

ScalarBox & ScalarBox::step(i32 direction)
{
  auto & state_ = drag_.state_;
  state_.scalar = state_.spec.match(
    [&](F32Info const & spec) -> Scalar {
      return spec.step_value(state_.scalar[v0], direction);
    },
    [&](I32Info const & spec) -> Scalar {
      return spec.step_value(state_.scalar[v1], direction);
    });
  drag_.format_();
  cb.update(state_.scalar);
  return *this;
}

ScalarBox & ScalarBox::stub(Str32 text)
{
  drag_.input_.stub(text);
  return *this;
}

ScalarBox & ScalarBox::stub(Str8 text)
{
  drag_.input_.stub(text);
  return *this;
}

ScalarBox & ScalarBox::format(Str format)
{
  drag_.style_.format = format;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::spec(f32 scalar, F32Info info)
{
  drag_.state_.scalar = scalar;
  drag_.state_.spec   = info;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::spec(i32 scalar, I32Info info)
{
  drag_.state_.scalar = scalar;
  drag_.state_.spec   = info;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::stroke(f32 s)
{
  drag_.style_.stroke = s;
  return *this;
}

ScalarBox & ScalarBox::thickness(f32 t)
{
  drag_.style_.thickness = t;
  return *this;
}

ScalarBox & ScalarBox::padding(Vec2 p)
{
  dec_.padding(p);
  inc_.padding(p);
  drag_.style_.padding = p;
  return *this;
}

ScalarBox & ScalarBox::frame(Vec2 extent, bool constrain)
{
  dec_.frame(extent, constrain);
  inc_.frame(extent, constrain);
  drag_.style_.frame = Frame{extent, constrain};
  return *this;
}

ScalarBox & ScalarBox::frame(Frame f)
{
  dec_.frame(f);
  inc_.frame(f);
  drag_.style_.frame = f;
  return *this;
}

ScalarBox & ScalarBox::corner_radii(CornerRadii const & r)
{
  dec_.rrect(r);
  inc_.rrect(r);
  drag_.style_.corner_radii = r;
  return *this;
}

ScalarBox & ScalarBox::on_update(Fn<void(Scalar)> f)
{
  cb.update = f;
  return *this;
}

ScalarBox & ScalarBox::button_text_style(TextStyle const & style,
                                         FontStyle const & font, usize first,
                                         usize count)
{
  dec_.run(style, font, first, count);
  inc_.run(style, font, first, count);
  return *this;
}

ScalarBox & ScalarBox::drag_text_style(TextStyle const & style,
                                       FontStyle const & font, usize first,
                                       usize count)
{
  drag_.input_.content_run(style, font, first, count)
    .stub_run(style, font, first, count);
  return *this;
}

ui::State ScalarBox::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  build(dec_);
  build(drag_);
  build(inc_);
  return ui::State{};
}

ScrollBar & ScrollBar::update(f32 center, f32 delta, f32 visibile, f32 total)
{
  // [ ] clamp
  state_.center         = center;
  state_.delta          = delta;
  state_.visible_extent = visibile;
  state_.total_extent   = total;
  return *this;
}

f32 ScrollBar::center() const
{
  return state_.center;
}

ui::State ScrollBar::tick(Ctx const & ctx, Events const & events,
                          Fn<void(View &)>)
{
  u32 const main_axis = (style_.axis == Axis::X) ? 0 : 1;

  if (events.drag_update())
  {
    auto h     = events.hit_info.unwrap_or();
    auto begin = h.viewport_region.begin()[main_axis];
    auto end   = h.viewport_region.end()[main_axis];
    auto scale = h.viewport_region.extent[main_axis] / state_.total_extent;
    auto thumb_extent = scale * state_.visible_extent;
    auto track_begin  = begin + 0.5F * thumb_extent;
    auto track_end    = end - 0.5F * thumb_extent;
    auto thumb_pos = clamp(h.viewport_hit[main_axis], track_begin, track_end);
    auto t         = unlerp(track_begin, track_end, thumb_pos);
    state_.center  = lerp(0.0F, state_.total_extent - state_.visible_extent, t);
  }

  if (events.focus_over())
  {
    if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Left)) ||
        (style_.axis == Axis::Y && ctx.key.down(KeyCode::Up)))
    {
      state_.center =
        clamp(state_.center - state_.delta * state_.visible_extent, 0.0F,
              state_.total_extent - state_.visible_extent);
    }
    else if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Right)) ||
             (style_.axis == Axis::Y && ctx.key.down(KeyCode::Down)))
    {
      state_.center =
        clamp(state_.center + state_.delta * state_.visible_extent, 0.0F,
              state_.total_extent - state_.visible_extent);
    }
  }

  state_.dragging = events.drag_update();
  state_.hovered  = events.pointer_over();
  state_.focused  = events.focus_over();

  return ui::State{.hidden    = state_.hidden,
                   .pointable = !state_.disabled,
                   .draggable = !state_.disabled,
                   .focusable = !state_.disabled};
}

Layout ScrollBar::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = allocated};
}

void ScrollBar::render(Canvas &      canvas, CRect const &,
                       CRect const & canvas_region, CRect const &)
{
  u32 const main_axis  = (style_.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style_.axis == Axis::X) ? 1 : 0;

  auto const scale = canvas_region.extent[main_axis] / state_.total_extent;
  auto const thumb_extent = state_.visible_extent * scale;
  auto const t = state_.center / (state_.total_extent - state_.visible_extent);
  f32 const  thumb_center = canvas_region.begin()[main_axis] +
                           0.5F * thumb_extent +
                           t * (canvas_region.extent[main_axis] - thumb_extent);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = canvas_region.center[cross_axis];
  thumb_rect.extent[main_axis]  = thumb_extent;
  thumb_rect.extent[cross_axis] = canvas_region.extent[cross_axis];

  Vec4U8 thumb_color;
  if (state_.dragging)
  {
    thumb_color = style_.thumb_dragging_color;
  }
  else if (state_.hovered)
  {
    thumb_color = style_.thumb_hovered_color;
  }
  else
  {
    thumb_color = style_.thumb_color;
  }

  canvas
    .rrect({.area         = canvas_region,
            .corner_radii = style_.track_corner_radii,
            .stroke       = 0,
            .tint         = style_.track_color})
    .rrect({.area         = thumb_rect,
            .corner_radii = style_.thumb_corner_radii,
            .stroke       = 0,
            .tint         = thumb_color});
}

ScrollView::ScrollView(View & child) : child_{child}
{
  x_bar_.style_.axis = Axis::X;
  y_bar_.style_.axis = Axis::Y;
}

ScrollView & ScrollView::disable(bool d)
{
  state_.disabled        = d;
  x_bar_.state_.disabled = d;
  y_bar_.state_.disabled = d;
  return *this;
}

ScrollView & ScrollView::item(View & v)
{
  child_ = v;
  return *this;
}

ScrollView & ScrollView::thumb_color(Vec4U8 c)
{
  x_bar_.style_.thumb_color = c;
  y_bar_.style_.thumb_color = c;
  return *this;
}

ScrollView & ScrollView::thumb_hovered_color(Vec4U8 c)
{
  x_bar_.style_.thumb_hovered_color = c;
  y_bar_.style_.thumb_hovered_color = c;
  return *this;
}

ScrollView & ScrollView::thumb_dragging_color(Vec4U8 c)
{
  x_bar_.style_.thumb_dragging_color = c;
  y_bar_.style_.thumb_dragging_color = c;
  return *this;
}

ScrollView & ScrollView::thumb_corner_radii(CornerRadii const & c)
{
  x_bar_.style_.thumb_corner_radii = c;
  y_bar_.style_.thumb_corner_radii = c;
  return *this;
}

ScrollView & ScrollView::track_color(Vec4U8 c)
{
  x_bar_.style_.track_color = c;
  y_bar_.style_.track_color = c;
  return *this;
}

ScrollView & ScrollView::track_corner_radii(CornerRadii const & c)
{
  x_bar_.style_.track_corner_radii = c;
  y_bar_.style_.track_corner_radii = c;
  return *this;
}

ScrollView & ScrollView::axes(Axes a)
{
  x_bar_.state_.hidden = has_bits(a, Axes::X);
  y_bar_.state_.hidden = has_bits(a, Axes::Y);
  return *this;
}

ScrollView & ScrollView::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

ScrollView & ScrollView::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

ScrollView & ScrollView::inner_frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

ScrollView & ScrollView::inner_frame(Frame f)
{
  style_.frame = f;
  return *this;
}

ScrollView & ScrollView::bar_size(f32 x, f32 y)
{
  style_.x_bar_size = x;
  style_.y_bar_size = y;
  return *this;
}

ui::State ScrollView::tick(Ctx const &, Events const & events,
                           Fn<void(View &)> build)
{
  if (events.scroll())
  {
    auto scroll = events.scroll_info.unwrap();

    if (!x_bar_.state_.disabled)
    {
      x_bar_.update(scroll.center.x, x_bar_.state_.delta,
                    x_bar_.state_.visible_extent, x_bar_.state_.total_extent);
    }

    if (!y_bar_.state_.disabled)
    {
      y_bar_.update(scroll.center.y, y_bar_.state_.delta,
                    y_bar_.state_.visible_extent, y_bar_.state_.total_extent);
    }
  }

  build(child_);
  build(x_bar_);
  build(y_bar_);

  return ui::State{.viewport = true};
}

void ScrollView::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 const frame = style_.frame(allocated);

  sizes[0] = style_.inner_frame(frame);
  sizes[1] = {frame.x, style_.x_bar_size};

  if (!x_bar_.state_.disabled && !y_bar_.state_.disabled)
  {
    sizes[1].x = max(sizes[1].x - style_.y_bar_size, 0.0F);
  }

  sizes[2] = {style_.y_bar_size, frame.y};
}

Layout ScrollView::fit(Vec2 allocated, Span<Vec2 const> sizes,
                       Span<Vec2> centers)
{
  Vec2 const frame = style_.frame(allocated);

  centers[0] = {0, 0};
  centers[1] = space_align(frame, sizes[1], ALIGNMENT_BOTTOM_LEFT);
  centers[2] = space_align(frame, sizes[2], ALIGNMENT_TOP_RIGHT);

  // [ ] still has extent
  Vec2 const context_extent = sizes[0];

  x_bar_.update(x_bar_.state_.center, x_bar_.state_.delta, frame.x,
                context_extent.x);
  y_bar_.update(y_bar_.state_.center, y_bar_.state_.delta, frame.y,
                context_extent.y);

  return {
    .extent          = frame,
    .viewport_extent = context_extent,
    .viewport_center{x_bar_.state_.center, y_bar_.state_.center}
  };
}

i32 ScrollView::layer(i32 allocated, Span<i32> layers)
{
  // needs to be at a different stacking context since this will be placed
  // on top of the viewport
  layers[0] = allocated + 1;
  layers[1] = allocated + 1;
  layers[2] = allocated;
  return allocated;
}

ui::State ComboItem::tick(Ctx const &, Events const &, Fn<void(View &)>)
{
  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void ComboItem::size(Vec2, Span<Vec2>)
{
}

Layout ComboItem::fit(Vec2, Span<Vec2 const>, Span<Vec2>)
{
  return Layout{};
}

void ComboItem::render(Canvas &, CRect const &, CRect const &, CRect const &)
{
}

Cursor ComboItem::cursor(Vec2, Vec2)
{
  return Cursor::Pointer;
}

TextComboItem::TextComboItem(Str32 text, TextStyle const & style,
                             FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
  text_.copyable(false);
}

TextComboItem::TextComboItem(Str8 text, TextStyle const & style,
                             FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
  text_.copyable(false);
}

TextComboItem & TextComboItem::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

TextComboItem & TextComboItem::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

TextComboItem & TextComboItem::padding(Vec2 padding)
{
  style_.padding = padding;
  return *this;
}

TextComboItem & TextComboItem::align(f32 alignment)
{
  style_.alignment = alignment;
  return *this;
}

TextComboItem & TextComboItem::color(Vec4U8 color)
{
  style_.color = color;
  return *this;
}

TextComboItem & TextComboItem::hover_color(Vec4U8 color)
{
  style_.hover_color = color;
  return *this;
}

TextComboItem & TextComboItem::selected_color(Vec4U8 color)
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

void TextComboItem::size(Vec2 allocated, Span<Vec2> sizes)
{
  Vec2 child_size = style_.frame(allocated) - 2 * style_.padding;
  child_size.x    = max(child_size.x, 0.0F);
  child_size.y    = max(child_size.y, 0.0F);
  sizes[0]        = child_size;
}

Layout TextComboItem::fit(Vec2 allocated, Span<Vec2 const> sizes,
                          Span<Vec2> centers)
{
  Vec2 frame = style_.frame(allocated);
  frame.x    = max(frame.x, sizes[0].x + 2 * style_.padding.x);
  frame.y    = max(frame.y, sizes[0].y + 2 * style_.padding.y);

  centers[0] = space_align(frame, sizes[0], Vec2{style_.alignment, 0});

  return {.extent = frame};
}

void TextComboItem::render(Canvas &      canvas, CRect const &,
                           CRect const & canvas_region, CRect const &)
{
  Vec4U8 color;
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

  canvas.rrect({.area         = canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = style_.stroke,
                .thickness    = style_.thickness,
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
  state_.disabled = d;
  for (ComboItem & item : items_)
  {
    item.state_.disabled = d;
  }
  return *this;
}

Combo & Combo::color(Vec4U8 c)
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

Combo & Combo::items(std::initializer_list<ref<ComboItem>> list)
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
  return items_.size32();
}

Combo & Combo::select(Option<usize> i)
{
  if (i.is_some())
  {
    CHECK(i.v() < items_.size32(), "");
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

void Combo::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                   CRect const &)
{
  canvas.rrect({.area         = canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = style_.stroke,
                .thickness    = style_.thickness,
                .tint         = style_.color});
}

Image::Image(ImageSrc src) : src_{std::move(src)}
{
}

Image & Image::source(ImageSrc src)
{
  src_            = std::move(src);
  state_.resolved = none;
  return *this;
}

Image & Image::aspect_ratio(f32 width, f32 height)
{
  style_.aspect_ratio = (width == 0 || height == 0) ? 1 : (width / height);
  return *this;
}

Image & Image::aspect_ratio(Option<f32> ratio)
{
  style_.aspect_ratio = ratio;
  return *this;
}

Image & Image::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

Image & Image::frame(Vec2 extent, bool constrain)
{
  style_.frame = Frame{extent, constrain};
  return *this;
}

Image & Image::corner_radii(CornerRadii const & radii)
{
  style_.radii = radii;
  return *this;
}

Image & Image::tint(ColorGradient const & color)
{
  style_.tint = color;
  return *this;
}

Image & Image::fit(ImageFit fit)
{
  style_.fit = fit;
  return *this;
}

Image & Image::align(Vec2 a)
{
  style_.alignment = a;
  return *this;
}

ui::State Image::tick(Ctx const &, Events const &, Fn<void(View &)>)
{
  state_.resolved.match(
    [&](None) {
      src_.match(
        [&](None) { state_.resolved = Option<ash::ImageInfo>{none}; },
        [&](ImageId id) { state_.resolved = Option{sys->image.get(id)}; },
        [&](Future<Result<ImageId, ImageLoadErr>> & f) {
          f.poll().match(
            [&](Result<ImageId, ImageLoadErr> & r) {
              r.match(
                [&](ImageId id) {
                  state_.resolved = Option{sys->image.get(id)};
                },
                [&](ImageLoadErr err) { state_.resolved = err; });
            },
            [&](Void) { state_.resolved = none; });
        });
    },
    [](auto &) {}, [](auto &) {});

  src_ = none;

  return ui::State{};
}

Layout Image::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  Vec2 const frame = style_.frame(allocated);

  if (style_.aspect_ratio.is_none())
  {
    return Layout{.extent = frame};
  }

  return Layout{.extent = with_aspect(frame, style_.aspect_ratio.v())};
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

  canvas.rrect({
    .area{region.center + center, extent},
    .corner_radii = style.radii,
    .tint         = style.tint,
    .sampler      = SamplerId::LinearClamped,
    .texture      = img.textures[0],
    .uv{uv0,                    uv1   }
  });
}

void Image::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                   CRect const &)
{
  state_.resolved.match([&](None) {},
                        [&](Option<ash::ImageInfo> & opt) {
                          opt.match(
                            [&](ash::ImageInfo & img) {
                              render_image(canvas, canvas_region, img, style_);
                            },
                            []() {});
                        },
                        [&](ImageLoadErr) {});
}

List::List(Generator generator, AllocatorRef allocator) :
  state_{.generator = generator, .items{allocator}},
  allocator_{allocator}
{
}

List & List::generator(Generator generator)
{
  state_.total_translation = 0;
  state_.view_extent       = 0;
  state_.first_item        = 0;
  state_.max_count         = USIZE_MAX;
  state_.num_loaded        = 0;
  state_.item_size         = none;
  state_.generator         = generator;
  state_.items.clear();
  return *this;
}

List & List::axis(Axis axis)
{
  style_.axis = axis;
  return *this;
}

List & List::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

List & List::item_frame(Frame frame)
{
  style_.item_frame = frame;
  return *this;
}

ui::State List::tick(Ctx const &, Events const & events, Fn<void(View &)> build)
{
  u32 const axis = style_.axis == Axis::X ? 0 : 1;

  if (events.scroll())
  {
    auto info                = events.scroll_info.unwrap();
    state_.total_translation = info.center[axis];
  }

  Slice visible = state_.visible().unwrap_or(Slice{0, 1})(state_.max_count);

  if (visible != state_.range())
  {
    auto old_range = state_.range();
    auto i         = visible.begin();

    for (; i < visible.end(); i++)
    {
      if (old_range.contains(i))
      {
        state_.items.push(std::move(state_.items[i])).unwrap();
      }
      else
      {
        if (auto item = state_.generator(allocator_, i); item.is_some())
        {
          state_.items.push(item.unwrap()).unwrap();
        }
        else
        {
          state_.max_count = i;
          break;
        }
      }
    }

    state_.items.erase(0, old_range.span);
    state_.first_item = visible.begin();
    state_.num_loaded = max(state_.range().end(), state_.num_loaded);
  }

  // [ ] ScrollBar: NEED TO GET SIZE INFO

  for (auto const & item : state_.items)
  {
    build(*item);
  }

  return ui::State{.scrollable = true, .viewport = true};
}

void List::size(Vec2 allocated, Span<Vec2> sizes)
{
  fill(sizes, style_.item_frame(style_.frame(allocated)));
}

Layout List::fit(Vec2 allocated, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  auto      frame      = style_.frame(allocated);
  Vec2      extent     = {};
  u32 const axis       = style_.axis == Axis::X ? 0 : 1;
  u32 const cross_axis = style_.axis == Axis::X ? 1 : 0;

  // Calculate total extent along main axis
  for (auto const size : sizes)
  {
    extent[cross_axis] = max(extent[cross_axis], size[cross_axis]);
    extent[axis] += size[axis];
  }

  // Position items along main axis with translation
  auto first_item_offset = state_.first_item * state_.item_size.unwrap_or();

  f32 cursor = -0.5F * extent[axis];
  cursor += state_.total_translation;
  cursor -= first_item_offset;

  for (auto [center, size] : zip(centers, sizes))
  {
    center[axis]       = cursor + size[axis] * 0.5F;
    center[cross_axis] = 0;
    cursor += size[axis];
  }

  if (!sizes.is_empty())
  {
    state_.item_size = sizes[0][axis];
  }

  state_.view_extent = frame[axis];

  return {
    .extent          = frame,
    .viewport_extent = extent,
    .viewport_center = {-state_.total_translation, 0}
  };
}

ui::State FocusView::tick(Ctx const & ctx, Events const &, Fn<void(View &)>)
{
  canvas_region =
    ctx.focused.map([](FocusRect r) { return r.area; }).unwrap_or();
  return ui::State{};
}

Layout FocusView::fit(Vec2, Span<Vec2 const>, Span<Vec2>)
{
  return Layout{.extent       = canvas_region.extent,
                .fixed_center = canvas_region.center};
}

void FocusView::render(Canvas &      canvas, CRect const &,
                       CRect const & canvas_region, CRect const &)
{
  // [ ] fix-up
  canvas.rrect(ShapeInfo{
    .area      = canvas_region,
    .stroke    = 1,
    .thickness = 0.5F,
    .tint      = ColorGradient{Vec4::splat(155)},
  });
}

}    // namespace ui
}    // namespace ash
