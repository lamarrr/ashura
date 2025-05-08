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
  style.frame = frame;
  return *this;
}

Space & Space::frame(Vec2 extent, bool constrain)
{
  style.frame = Frame{extent, constrain};
  return *this;
}

Layout Space::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return Layout{.extent = style.frame(allocated)};
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
  Vec2 const frame = style.frame(allocated);
  fill(sizes, style.item_frame(frame));
}

Layout Flex::fit(Vec2 allocated, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  auto const n            = sizes.size();
  Vec2 const frame        = style.frame(allocated);
  u32 const  main_axis    = (style.axis == Axis::X) ? 0 : 1;
  u32 const  cross_axis   = (style.axis == Axis::X) ? 1 : 0;
  Vec2       span         = {};
  f32        cross_cursor = 0;

  for (usize i = 0; i < n;)
  {
    auto first        = i++;
    f32  main_extent  = sizes[first][main_axis];
    f32  cross_extent = sizes[first][cross_axis];
    f32  main_spacing = 0;

    while (i < n && !(style.wrap &&
                      (main_extent + sizes[i][main_axis]) > frame[main_axis]))
    {
      main_extent += sizes[i][main_axis];
      cross_extent = max(cross_extent, sizes[i][cross_axis]);
      i++;
    }

    auto const count = i - first;

    if (style.main_align != MainAlign::Start)
    {
      main_spacing = max(frame[main_axis] - main_extent, 0.0F);
    }

    for (auto [center, size] :
         zip(centers.slice(first, count), sizes.slice(first, count)))
    {
      f32 const pos =
        space_align(cross_extent, size[cross_axis], style.cross_align);
      center[cross_axis] = cross_cursor + cross_extent * 0.5F + pos;
    }

    switch (style.main_align)
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
  fill(sizes, style.frame(allocated));
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
    center = space_align(span, size, style.alignment);
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

TextCommand text_command(Ctx const & ctx, Events const & events, bool multiline,
                         bool enter_submits, bool tab_input)
{
  if (events.focus_out())
  {
    return TextCommand::Escape;
  }

  if (events.text_input())
  {
    return TextCommand::InputText;
  }

  auto const shift = ctx.key.held(KeyModifiers::LeftShift) ||
                     ctx.key.held(KeyModifiers::RightShift);
  auto const ctrl = ctx.key.held(KeyModifiers::LeftCtrl) ||
                    ctx.key.held(KeyModifiers::RightCtrl);

  if (events.key_down())
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

    if (ctrl && ctx.key.down(KeyCode::X))
    {
      return TextCommand::Cut;
    }

    if (ctrl && ctx.key.down(KeyCode::C))
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

    if (multiline && !enter_submits && ctx.key.down(KeyCode::Return))
    {
      return TextCommand::NewLine;
    }

    if (tab_input && ctx.key.down(KeyCode::Tab))
    {
      return TextCommand::Tab;
    }

    if (ctx.key.down(KeyCode::Escape))
    {
      return TextCommand::Unselect;
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

    if (ctx.key.down(KeyCode::Return) && enter_submits)
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

    return TextCommand::HitSelect;
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
  state.copyable = allow;
  return *this;
}

Text & Text::highlight_style(TextHighlightStyle highlight)
{
  style.highlight = highlight;
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
  TextCommand cmd = TextCommand::None;
  if (events.drag_start())
  {
    cmd = TextCommand::Hit;
  }
  else if (ctx.mouse.down(MouseButton::Primary) &&
           (ctx.mouse.clicks(MouseButton::Primary) == 2))
  {
    cmd = TextCommand::SelectWord;
  }
  else if (ctx.mouse.down(MouseButton::Primary) &&
           (ctx.mouse.clicks(MouseButton::Primary) == 3))
  {
    cmd = TextCommand::SelectAll;
  }
  else if (events.drag_update())
  {
    cmd = TextCommand::HitSelect;
  }
  else if (ctx.mouse.any_down)
  {
    cmd = TextCommand::Unselect;
  }

  auto hit_info =
    events.hit_info.map([](auto s) { return s; }).unwrap_or(HitInfo{});

  bool modified = compositor_.command(
    text_, cmd, {}, engine->clipboard, 1, 1, hit_info.canvas_region.center,
    hit_info.viewport_region.extent.x, hit_info.viewport_hit, hit_info.zoom(),
    default_allocator);
  CHECK(!modified, "");

  text_.add_highlight(compositor_.cursor().selection())
    .highlight_style(style.highlight);

  return ui::State{.draggable = state.copyable};
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
  return state.copyable ? Cursor::Text : Cursor::Default;
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

  state.editing = false;
  state.submit  = false;

  u8 buffer[512];

  FallbackAllocator allocator{Arena::from(buffer), allocator_};

  Vec<c32> input_u32{allocator};

  if (events.text_input())
  {
    utf8_decode(ctx.key.text, input_u32).unwrap();
  }

  TextCommand cmd = TextCommand::None;

  if (events.focus_over() || events.drag_update())
  {
    cmd = text_command(ctx, events, state.multiline, state.enter_submits,
                       state.tab_input);
  }

  auto hit_info =
    events.hit_info.map([](auto s) { return s; }).unwrap_or(HitInfo{});

  bool modified = compositor_.command(
    content_, cmd, input_u32, engine->clipboard, style.lines_per_page,
    style.tab_width, hit_info.canvas_region.center,
    hit_info.viewport_region.extent.x, hit_info.viewport_hit, hit_info.zoom(),
    allocator);

  auto cursor = compositor_.cursor();

  content_.clear_highlights()
    .clear_carets()
    .add_highlight(cursor.selection())
    .add_caret(cursor.caret());

  if (events.focus_over())
  {
    content_.highlight_style(style.highlight).caret_style(style.caret);
  }
  else
  {
    content_.highlight_style(none).caret_style(none);
  }

  if (modified)
  {
    content_.flush_text();
  }

  if (edited)
  {
    state.editing = true;
  }

  if (cmd == TextCommand::Submit)
  {
    state.submit = true;
  }

  if (events.focus_in())
  {
    cb.focus_in();
  }

  if (events.focus_out())
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

  return ui::State{
    .text =
      TextInputInfo{.multiline = state.multiline, .tab_input = state.tab_input},
    .draggable  = !state.disabled,
    .focusable  = !state.disabled,
    .grab_focus = events.pointer_down()
  };
}

Layout Input::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  if (content_.text_.is_empty())
  {
    stub_.layout(allocated.x);
    return {.extent = stub_.layout_.extent};
  }
  content_.layout(allocated.x);
  return {.extent = content_.layout_.extent};
}

void Input::render(Canvas & canvas, CRect const & viewport_region,
                   CRect const & canvas_region, CRect const & clip)
{
  if (content_.text_.is_empty())
  {
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

ui::State Button::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_over())
  {
    cb.hovered();
  }

  if (events.pointer_down())
  {
    cb.pressed();
  }

  state.held    = events.pointer_over() && ctx.mouse.held(MouseButton::Primary);
  state.hovered = events.pointer_over();

  return ui::State{.pointable = !state.disabled,
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

Layout Button::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  fill(centers, Vec2{0, 0});
  Vec2 size = sizes.is_empty() ? Vec2{0, 0} : sizes[0];
  return {.extent = size + 2 * style.padding};
}

void Button::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                    CRect const &)
{
  Vec4U8 tint;

  if (state.disabled)
  {
    tint = style.disabled_color;
  }
  else if (state.hovered && !state.held)
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
      canvas.rrect({.area         = canvas_region,
                    .corner_radii = style.corner_radii,
                    .stroke       = style.stroke,
                    .thickness    = style.thickness,
                    .tint         = tint});
      break;
    case ButtonShape::Squircle:
      canvas.squircle({.area         = canvas_region,
                       .corner_radii = style.corner_radii,
                       .stroke       = style.stroke,
                       .thickness    = style.thickness,
                       .tint         = tint});
      break;
    case ButtonShape::Bevel:
      canvas.brect({.area         = canvas_region,
                    .corner_radii = style.corner_radii,
                    .stroke       = style.stroke,
                    .thickness    = style.thickness,
                    .tint         = tint});
      break;
    default:
      break;
  }
}

Cursor Button::cursor(Vec2, Vec2)
{
  return state.disabled ? Cursor::Default : Cursor::Pointer;
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
  state.disabled = d;
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

TextButton & TextButton::squircle(f32 degree)
{
  style.corner_radii = CornerRadii{degree, degree, degree, degree};
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

ui::State TextButton::tick(Ctx const & ctx, Events const & events,
                           Fn<void(View &)> build)
{
  ui::State state = Button::tick(ctx, events, build);
  build(text_);
  return state;
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
  state.hidden = hide;
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
  return ui::State{.hidden = state.hidden};
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

CheckBox & CheckBox::corner_radii(CornerRadii const & r)
{
  style.corner_radii = r;
  return *this;
}

CheckBox & CheckBox::padding(f32 p)
{
  style.padding = p;
  return *this;
}

CheckBox & CheckBox::on_changed(Fn<void(bool)> f)
{
  cb.changed = f;
  return *this;
}

ui::State CheckBox::tick(Ctx const &, Events const & events,
                         Fn<void(View &)> build)
{
  if (events.pointer_down())
  {
    state.value = !state.value;
    cb.changed(state.value);
  }

  icon_.hide(!state.value);

  build(icon_);

  return ui::State{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

void CheckBox::size(Vec2 allocated, Span<Vec2> sizes)
{
  fill(sizes, allocated - 2 * style.padding);
}

Layout CheckBox::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  fill(centers, Vec2{});
  return {.extent = style.padding + sizes[0]};
}

void CheckBox::render(Canvas &      canvas, CRect const &,
                      CRect const & canvas_region, CRect const &)
{
  Vec4U8 tint;
  if (state.hovered && !state.held && !state.disabled)
  {
    tint = style.box_hovered_color;
  }
  else
  {
    tint = style.box_color;
  }

  canvas.rrect({.area         = canvas_region,
                .corner_radii = style.corner_radii,
                .stroke       = 1,
                .thickness    = 2,
                .tint         = tint});
}

Cursor CheckBox::cursor(Vec2, Vec2)
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

ui::State Slider::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  u32 const main_axis = (style.axis == Axis::X) ? 0 : 1;

  if (events.drag_update())
  {
    auto      h = events.hit_info.unwrap_or(HitInfo{});
    f32 const thumb_begin =
      h.viewport_region.begin()[main_axis] + style.thumb_size * 0.5F;
    f32 const thumb_end =
      h.viewport_region.end()[main_axis] - style.thumb_size * 0.5F;
    state.t = clamp(unlerp(thumb_begin, thumb_end, h.viewport_hit[main_axis]),
                    0.0F, 1.0F);
    f32 const value =
      clamp(lerp(state.low, state.high, state.t), state.low, state.high);
    cb.changed(value);
  }

  if (events.focus_over())
  {
    if ((style.axis == Axis::X && ctx.key.down(KeyCode::Left)) ||
        (style.axis == Axis::Y && ctx.key.down(KeyCode::Up)))
    {
      state.t = max(state.t - style.delta, 0.0F);
    }
    else if ((style.axis == Axis::X && ctx.key.down(KeyCode::Right)) ||
             (style.axis == Axis::Y && ctx.key.down(KeyCode::Down)))
    {
      state.t = min(state.t + style.delta, 1.0F);
    }
  }

  state.dragging = events.drag_update();
  state.hovered  = events.focus_over();

  return ui::State{.pointable = !state.disabled,
                   .draggable = !state.disabled,
                   .focusable = !state.disabled};
}

Layout Slider::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style.frame(allocated)};
}

void Slider::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                    CRect const &)
{
  u32 const main_axis  = (style.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style.axis == Axis::Y) ? 0 : 1;

  Vec4U8 thumb_color;

  if (state.dragging)
  {
    thumb_color = style.thumb_dragging_color;
  }
  else if (state.hovered)
  {
    thumb_color = style.thumb_hovered_color;
  }
  else
  {
    thumb_color = style.thumb_color;
  }

  f32 dilation = 1.0F;

  if (state.dragging || state.hovered)
  {
    dilation = 1.0F;
  }
  else
  {
    dilation = 0.8F;
  }

  f32 const thumb_begin =
    canvas_region.begin()[main_axis] + style.thumb_size * 0.5F;
  f32 const thumb_end =
    canvas_region.end()[main_axis] - style.thumb_size * 0.5F;
  f32 const thumb_center = lerp(thumb_begin, thumb_end, state.t);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = canvas_region.center[cross_axis];
  thumb_rect.extent             = Vec2::splat(style.thumb_size);

  CRect track_rect;

  track_rect.center             = canvas_region.center;
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
    .rrect({
      .area         = track_rect,
      .corner_radii = style.track_corner_radii,
      .tint         = style.track_color
  })
    .rrect({.area         = coverage_rect,
            .corner_radii = style.track_corner_radii,
            .tint         = thumb_color})
    .rrect({.area{thumb_rect.center, thumb_rect.extent * dilation},
            .corner_radii = style.thumb_corner_radii * dilation,
            .tint         = thumb_color});
}

Cursor Slider::cursor(Vec2, Vec2)
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

ui::State Switch::tick(Ctx const &, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_down())
  {
    state.value = !state.value;
    cb.changed(state.value);
  }

  state.hovered = events.pointer_over();

  return ui::State{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

Layout Switch::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style.frame(allocated)};
}

void Switch::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                    CRect const &)
{
  Vec2 thumb_extent = canvas_region.extent;
  thumb_extent.x *= 0.5F;
  Vec2 const alignment{state.value ? ALIGNMENT_RIGHT : ALIGNMENT_LEFT,
                       ALIGNMENT_CENTER};
  Vec2 const thumb_center =
    canvas_region.center +
    space_align(canvas_region.extent, thumb_extent, alignment);

  Vec4U8 thumb_color;
  if (state.hovered)
  {
    thumb_color =
      state.value ? style.on_hovered_color : style.off_hovered_color;
  }
  else
  {
    thumb_color = state.value ? style.on_color : style.off_color;
  }

  canvas
    .rrect({
      .area         = canvas_region,
      .corner_radii = style.corner_radii,
      .tint         = style.track_color
  })
    .rrect({.area{thumb_center, thumb_extent},
            .corner_radii = style.corner_radii,
            .tint         = thumb_color});
}

Cursor Switch::cursor(Vec2, Vec2)
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

ui::State Radio::tick(Ctx const &, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_down())
  {
    state.value = !state.value;
    cb.changed(state.value);
  }

  state.hovered = events.pointer_over();

  return ui::State{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
}

Layout Radio::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style.frame(allocated)};
}

void Radio::render(Canvas & canvas, CRect const &, CRect const & canvas_region,
                   CRect const &)
{
  canvas.rrect({.area         = canvas_region,
                .corner_radii = style.corner_radii,
                .stroke       = 1,
                .thickness    = style.thickness,
                .tint         = style.color});

  if (state.value)
  {
    Vec2   inner_extent = canvas_region.extent * (state.hovered ? 0.75F : 0.5F);
    Vec4U8 inner_color =
      state.hovered ? style.inner_hovered_color : style.inner_color;

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
  sformat(allocator, style.format, state.scalar)
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
  state.dragging = events.drag_update();

  if (events.drag_start() &&
      (ctx.key.down(KeyCode::LeftCtrl) || ctx.key.down(KeyCode::RightCtrl)))
  {
    state.input_mode = !state.input_mode;
  }

  if (state.dragging && !state.input_mode)
  {
    auto      h = events.hit_info.unwrap_or(HitInfo{});
    f32 const t = clamp(unlerp(h.viewport_region.begin().x,
                               h.viewport_region.end().x, h.viewport_hit.x),
                        0.0F, 1.0F);
    state.scalar =
      state.spec.match([t](F32Info & v) -> Scalar { return v.interp(t); },
                       [t](I32Info & v) -> Scalar { return v.interp(t); });

    format_();
    cb.update(state.scalar);
  }
  else if (input_.state.editing)
  {
    scalar_parse(input_.content_.get_text(), state.spec, state.scalar);
    cb.update(state.scalar);
  }

  input_.state.disabled = !state.input_mode;

  build(input_);

  return ui::State{.pointable = !state.disabled,
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

Layout ScalarDragBox::fit(Vec2 allocated, Span<Vec2 const> sizes,
                          Span<Vec2> centers)
{
  Vec2 frame         = style.frame(allocated);
  Vec2 padded_extent = sizes[0] + 2 * style.padding;
  frame.x            = max(frame.x, padded_extent.x);
  frame.y            = max(frame.y, padded_extent.y);
  fill(centers, Vec2{0, 0});

  return {.extent = frame};
}

void ScalarDragBox::render(Canvas &      canvas, CRect const &,
                           CRect const & canvas_region, CRect const &)
{
  canvas.rrect({.area         = canvas_region,
                .corner_radii = style.corner_radii,
                .stroke       = style.stroke,
                .thickness    = style.thickness,
                .tint         = style.color});

  if (!state.input_mode)
  {
    f32 const t = state.spec.match(
      [&](F32Info & v) { return v.uninterp(state.scalar[v0]); },
      [&](I32Info & v) { return v.uninterp(state.scalar[v1]); });

    CRect const thumb_rect = CRect::from_offset(
      canvas_region.begin(), canvas_region.extent * Vec2{t, 1});

    canvas.rrect({.area         = thumb_rect,
                  .corner_radii = style.corner_radii,
                  .tint         = style.thumb_color});
  }
}

Cursor ScalarDragBox::cursor(Vec2, Vec2)
{
  return state.disabled ? Cursor::Default : Cursor::EWResize;
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
  auto & state = drag_.state;
  state.scalar = state.spec.match(
    [&](F32Info const & spec) -> Scalar {
      return spec.step_value(state.scalar[v0], direction);
    },
    [&](I32Info const & spec) -> Scalar {
      return spec.step_value(state.scalar[v1], direction);
    });
  drag_.format_();
  cb.update(state.scalar);
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
  drag_.style.format = format;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::spec(f32 scalar, F32Info info)
{
  drag_.state.scalar = scalar;
  drag_.state.spec   = info;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::spec(i32 scalar, I32Info info)
{
  drag_.state.scalar = scalar;
  drag_.state.spec   = info;
  drag_.format_();
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

ui::State ScrollBar::tick(Ctx const & ctx, Events const & events,
                          Fn<void(View &)>)
{
  u32 const main_axis = (style.axis == Axis::X) ? 0 : 1;

  if (events.drag_update())
  {
    auto      h = events.hit_info.unwrap_or(HitInfo{});
    f32 const thumb_begin =
      h.viewport_region.begin()[main_axis] + style.thumb_size * 0.5F;
    f32 const thumb_end =
      h.viewport_region.end()[main_axis] - style.thumb_size * 0.5F;
    state.t = clamp(unlerp(thumb_begin, thumb_end, h.viewport_hit[main_axis]),
                    0.0F, 1.0F);
  }

  if (events.focus_over())
  {
    if ((style.axis == Axis::X && ctx.key.down(KeyCode::Left)) ||
        (style.axis == Axis::Y && ctx.key.down(KeyCode::Up)))
    {
      state.t = max(state.t - style.delta, 0.0F);
    }
    else if ((style.axis == Axis::X && ctx.key.down(KeyCode::Right)) ||
             (style.axis == Axis::Y && ctx.key.down(KeyCode::Down)))
    {
      state.t = min(state.t + style.delta, 1.0F);
    }
  }

  state.dragging = events.drag_update();
  state.hovered  = events.pointer_over();
  state.focused  = events.focus_over();

  return ui::State{.hidden    = state.hidden,
                   .pointable = !state.disabled,
                   .draggable = !state.disabled,
                   .focusable = !state.disabled};
}

Layout ScrollBar::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = allocated};
}

void ScrollBar::render(Canvas &      canvas, CRect const &,
                       CRect const & canvas_region, CRect const &)
{
  u32 const main_axis  = (style.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style.axis == Axis::X) ? 1 : 0;

  f32 const thumb_begin =
    canvas_region.begin()[main_axis] + style.thumb_size * 0.5F;
  f32 const thumb_end =
    canvas_region.end()[main_axis] - style.thumb_size * 0.5F;
  f32 const thumb_center = lerp(thumb_begin, thumb_end, state.t);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = canvas_region.center[cross_axis];
  thumb_rect.extent[main_axis]  = style.thumb_size;
  thumb_rect.extent[cross_axis] = canvas_region.extent[cross_axis];

  Vec4U8 thumb_color;
  if (state.dragging)
  {
    thumb_color = style.thumb_dragging_color;
  }
  else if (state.hovered)
  {
    thumb_color = style.thumb_hovered_color;
  }
  else
  {
    thumb_color = style.thumb_color;
  }

  canvas
    .rrect({.area         = canvas_region,
            .corner_radii = style.track_corner_radii,
            .stroke       = 0,
            .tint         = style.track_color})
    .rrect({.area         = thumb_rect,
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

ui::State ScrollView::tick(Ctx const &, Events const & events,
                           Fn<void(View &)> build)
{
  if (events.scroll())
  {
    auto scroll = events.scroll_info.unwrap_or(ScrollInfo{});

    if (!x_bar_.state.disabled)
    {
      x_bar_.state.pos = scroll.center.x;
    }

    if (!y_bar_.state.disabled)
    {
      y_bar_.state.pos = scroll.center.y;
    }
  }

  build(child_);
  build(x_bar_);
  build(y_bar_);

  return ui::State{.viewport = true};
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

Layout ScrollView::fit(Vec2 allocated, Span<Vec2 const> sizes,
                       Span<Vec2> centers)
{
  Vec2 const frame = style.frame(allocated);

  centers[0] = {0, 0};
  centers[1] = space_align(frame, sizes[1], ALIGNMENT_BOTTOM_LEFT);
  centers[2] = space_align(frame, sizes[2], ALIGNMENT_TOP_RIGHT);

  Vec2 const context_extent = sizes[0];

  return {
    .extent          = frame,
    .viewport_extent = context_extent,
    .viewport_center{x_bar_.state.pos, y_bar_.state.pos}
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
  return ui::State{.pointable = !state.disabled,
                   .clickable = !state.disabled,
                   .focusable = !state.disabled};
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

ui::State TextComboItem::tick(Ctx const & ctx, Events const & events,
                              Fn<void(View &)> build)
{
  if (events.pointer_over() && ctx.mouse.down(MouseButton::Primary) &&
      !ComboItem::state.selected)
  {
    ComboItem::state.click_hook(ComboItem::state.id);
  }

  state.hovered = events.pointer_over();
  state.pressed = events.pointer_over() && ctx.mouse.held(MouseButton::Primary);

  build(text_);

  return ui::State{.pointable = !ComboItem::state.disabled,
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

Layout TextComboItem::fit(Vec2 allocated, Span<Vec2 const> sizes,
                          Span<Vec2> centers)
{
  Vec2 frame = style.frame(allocated);
  frame.x    = max(frame.x, sizes[0].x + 2 * style.padding.x);
  frame.y    = max(frame.y, sizes[0].y + 2 * style.padding.y);

  centers[0] = space_align(frame, sizes[0], Vec2{style.alignment, 0});

  return {.extent = frame};
}

void TextComboItem::render(Canvas &      canvas, CRect const &,
                           CRect const & canvas_region, CRect const &)
{
  Vec4U8 color;
  if (ComboItem::state.selected)
  {
    color = style.selected_color;
  }
  else if (state.hovered && !state.pressed)
  {
    color = style.color;
  }
  else if (state.hovered)
  {
    color = style.hover_color;
  }
  else
  {
    color = style.color;
  }

  canvas.rrect({.area         = canvas_region,
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
    item->state.disabled   = state.disabled;
    item->state.selected   = false;
    item->state.click_hook = {this, [](Combo * c, usize id) { c->select(id); }};
    item->state.id         = i;
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

  state.selected = i;

  for (ComboItem & it : items_)
  {
    it.state.selected = false;
  }

  if (i.is_some())
  {
    ComboItem & item    = items_[i.v()];
    item.state.selected = true;
  }

  cb.selected(i);
  return *this;
}

Option<usize> Combo::get_selection() const
{
  return state.selected;
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

ui::State Image::tick(Ctx const &, Events const &, Fn<void(View &)>)
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

  return ui::State{};
}

Layout Image::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  Vec2 const frame = style.frame(allocated);

  if (style.aspect_ratio.is_none())
  {
    return Layout{.extent = frame};
  }

  return Layout{.extent = with_aspect(frame, style.aspect_ratio.v())};
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
  state.resolved.match([&](None) {},
                       [&](Option<ash::ImageInfo> & opt) {
                         opt.match(
                           [&](ash::ImageInfo & img) {
                             render_image(canvas, canvas_region, img, style);
                           },
                           []() {});
                       },
                       [&](ImageLoadErr) {});
}

List::List(Generator generator, AllocatorRef allocator) :
  state_{.generator = generator},
  allocator_{allocator},
  items_{allocator},
  scrolled_offsets_{allocator}
{
}

List & List::generator(Generator generator)
{
  state_.total_translation = 0;
  state_.view_extent       = 0;
  state_.actives_range     = {};
  state_.max_size          = USIZE_MAX;
  state_.generator         = generator;
  items_.clear();
  scrolled_offsets_.clear();
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

ui::State List::tick(Ctx const & ctx, Events const & events,
                     Fn<void(View &)> build)
{
  u32 const axis = style_.axis == Axis::X ? 0 : 1;

  if (events.scroll())
  {
    auto info                = events.scroll_info.unwrap();
    state_.total_translation = info.center[axis];
  }

  Slice next = state_.actives_range;

  if ((statex_.subset_translation + statex_.subset_extent) <
      statex_.view_extent)
  {
    next.span += statex_.batch_size;
  }

  if (statex_.subset_translation > 0)
  {
    auto begin = sat_sub(next.begin(), statex_.batch_size);
    auto end   = next.end();
    next       = Slice::from_range(begin, end);
  }

  next = next(statex_.max_size);

  if (next != statex_.range)
  {
    Vec<Dyn<View *>> new_items{allocator_};

    auto i = next.begin();

    for (; i < next.end(); i++)
    {
      if (statex_.range.contains(i))
      {
        new_items.push(std::move(items_[i])).unwrap();
      }
      else
      {
        if (auto item = statex_.generator(allocator_, i); item.is_some())
        {
          new_items.push(item.unwrap()).unwrap();
        }
        else
        {
          statex_.max_size = i;
          break;
        }
      }
    }

    new_items     = std::move(items_);
    statex_.range = Slice::from_range(next.begin(), i);
  }

  // [ ] NEED TO GET SIZE INFO FOR SCROLL BAR

  for (auto const & item : items_)
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
  f32 cursor = -extent[axis] * 0.5F + statex_.total_translation;
  for (auto [center, size] : zip(centers, sizes))
  {
    center[axis]       = cursor + size[axis] * 0.5F;
    center[cross_axis] = 0;
    cursor += size[axis];
  }

  // Update state
  statex_.view_extent   = allocated[axis];
  statex_.subset_extent = extent[axis];

  return {
    .extent          = allocated,
    .viewport_extent = extent,
    .viewport_center = {statex_.total_translation, 0}
  };
}

void List::render(Canvas & canvas, CRect const & viewport_region,
                  CRect const & canvas_region, CRect const & clip)
{
  // Render each visible item
  for (auto const & item : items_)
  {
    item->render(canvas, viewport_region, canvas_region, clip);
  }
}

}    // namespace ui
}    // namespace ash
