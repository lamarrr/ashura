/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/engine.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/dyn.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"

namespace ash
{

struct FocusStyle
{
  ColorGradient border_color     = DEFAULT_THEME.primary;
  f32           border_thickness = 1.0F;
};

struct FocusState
{
  bool in      = false;
  bool out     = false;
  bool focused = false;

  void tick(ViewEvents const & events)
  {
    in  = events.focus_in;
    out = events.focus_out;

    if (events.focus_in)
    {
      focused = true;
    }

    if (events.focus_out)
    {
      focused = false;
    }
  }
};

struct PressState
{
  bool       in      = false;
  bool       out     = false;
  bool       hovered = false;
  bool       down    = false;
  bool       held    = false;
  FocusState focus   = {};

  void tick(ViewContext const & ctx, ViewEvents const & events)
  {
    focus.tick(events);

    in  = events.mouse_in;
    out = events.mouse_out;

    if (in)
    {
      hovered = true;
    }

    if (out)
    {
      hovered = false;
      held    = false;
    }

    if (events.focus_out)
    {
      held = false;
    }

    down = (events.mouse_down && ctx.mouse_down(MouseButtons::Primary)) ||
           (events.key_down && ctx.key_down(KeyCode::Return));

    bool up = (events.mouse_up && ctx.mouse_up(MouseButtons::Primary)) ||
              (events.key_up && ctx.key_up(KeyCode::Return));

    if (down)
    {
      held = true;
    }

    if (up)
    {
      held = false;
    }
  }
};

struct DragState
{
  bool       in       = false;
  bool       out      = false;
  bool       hovered  = false;
  bool       start    = false;
  bool       dragging = false;
  bool       end      = false;
  FocusState focus    = {};

  void tick(ViewEvents const & events)
  {
    focus.tick(events);
    in  = events.mouse_in;
    out = events.mouse_out;

    if (in)
    {
      hovered = true;
    }

    if (out)
    {
      hovered = false;
    }

    start    = events.drag_start;
    dragging = events.dragging;
    end      = events.drag_end;
  }
};

// [ ] implement
struct Spacer : View
{
};

/// @param axis flex axis to layout children along
/// @param main_align main-axis alignment. specifies how free space is used on
/// the main axis
/// @param cross_align cross-axis alignment. affects how free space is used on
/// the cross axis
struct FlexView : View
{
  struct Style
  {
    Axis      axis        = Axis::X;
    bool      wrap        = true;
    MainAlign main_align  = MainAlign::Start;
    f32       cross_align = 0;
    Frame     frame       = Frame{}.scale(1, 1);
  } styling;

  struct Inner
  {
    Vec<View *> items{default_allocator};
  } inner;

  FlexView & axis(Axis a)
  {
    styling.axis = a;
    return *this;
  }

  FlexView & wrap(bool w)
  {
    styling.wrap = w;
    return *this;
  }

  FlexView & main_align(MainAlign align)
  {
    styling.main_align = align;
    return *this;
  }

  FlexView & cross_align(f32 align)
  {
    styling.cross_align = align;
    return *this;
  }

  FlexView & frame(f32 width, f32 height, bool constrain = true)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  FlexView & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  FlexView & items(std::initializer_list<View *> list)
  {
    inner.items.extend_copy(span(list)).unwrap();
    return *this;
  }

  FlexView & items(Span<View * const> list)
  {
    inner.items.extend_copy(list).unwrap();
    return *this;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32, ViewEvents,
                         Fn<void(View &)> build) override
  {
    for (View * item : inner.items)
    {
      build(*item);
    }

    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame = styling.frame(allocated);
    fill(sizes, frame);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    u32 const  n            = sizes.size32();
    Vec2 const frame        = styling.frame(allocated);
    u8 const   main_axis    = (styling.axis == Axis::X) ? 0 : 1;
    u8 const   cross_axis   = (styling.axis == Axis::X) ? 1 : 0;
    Vec2       span         = {};
    f32        cross_cursor = 0;

    for (u32 i = 0; i < n;)
    {
      u32 first        = i++;
      f32 main_extent  = sizes[first][main_axis];
      f32 cross_extent = sizes[first][cross_axis];
      f32 main_spacing = 0;

      while (i < n && !(styling.wrap &&
                        (main_extent + sizes[i][main_axis]) > frame[main_axis]))
      {
        main_extent += sizes[i][main_axis];
        cross_extent = max(cross_extent, sizes[i][cross_axis]);
        i++;
      }

      u32 const count = i - first;

      if (styling.main_align != MainAlign::Start)
      {
        main_spacing = max(frame[main_axis] - main_extent, 0.0F);
      }

      for (u32 b = first; b < first + count; b++)
      {
        f32 const pos          = space_align(cross_extent, sizes[b][cross_axis],
                                             styling.cross_align);
        centers[b][cross_axis] = cross_cursor + cross_extent * 0.5F + pos;
      }

      switch (styling.main_align)
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
};

struct StackView : View
{
  struct Style
  {
    bool  reverse   = false;
    Vec2  alignment = {0, 0};
    Frame frame     = Frame{}.scale(1, 1);
  } styling;

  struct Inner
  {
    Vec<View *> items{default_allocator};
  } inner;

  StackView & reverse(bool r)
  {
    styling.reverse = r;
    return *this;
  }

  StackView & align(f32 x, f32 y)
  {
    styling.alignment = Vec2{x, y};
    return *this;
  }

  StackView & align(Vec2 a)
  {
    styling.alignment = a;
    return *this;
  }

  StackView & frame(f32 width, f32 height, bool constrain = true)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  StackView & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  StackView & items(std::initializer_list<View *> list)
  {
    inner.items.extend_copy(span(list)).unwrap();
    return *this;
  }

  StackView & items(Span<View * const> list)
  {
    inner.items.extend_copy(list).unwrap();
    return *this;
  }

  virtual i32 stack_item(i32 base, u32 i, u32 num)
  {
    // sequential stacking
    i32 z = base;
    if (!styling.reverse)
    {
      z += (i32) i;
    }
    else
    {
      z += (i32) (num - i);
    }
    return z;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32, ViewEvents,
                         Fn<void(View &)> build) override
  {
    for (View * item : inner.items)
    {
      build(*item);
    }

    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, styling.frame(allocated));
  }

  virtual ViewLayout fit(Vec2, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2      span;
    u32 const n = sizes.size32();

    for (Vec2 styling : sizes)
    {
      span.x = max(span.x, styling.x);
      span.y = max(span.y, styling.y);
    }

    for (u32 i = 0; i < n; i++)
    {
      centers[i] = space_align(span, sizes[i], styling.alignment);
    }

    return {.extent = span};
  }

  virtual i32 z_index(i32 allocated, Span<i32> indices) override
  {
    u32 const n = indices.size32();
    for (u32 i = 0; i < n; i++)
    {
      indices[i] = stack_item(allocated, i, n);
    }
    return allocated;
  }
};

struct TextView : View
{
  struct State
  {
    bool copyable = false;
  } state;

  struct Style
  {
    TextHighlightStyle cursor_highlight;
  } styling;

  struct Inner
  {
    RenderText     text{};
    TextCompositor compositor{};
  } inner;

  TextView()
  {
    inner.text.style(TextStyle{.foreground = DEFAULT_THEME.on_surface},
                     FontStyle{.font        = engine->default_font,
                               .font_height = DEFAULT_THEME.body_font_height,
                               .line_height = DEFAULT_THEME.line_height});
  }

  virtual ~TextView() override = default;

  TextView & copyable(bool allow)
  {
    state.copyable = allow;
    return *this;
  }

  TextView & highlight(TextHighlight highlight)
  {
    inner.text.highlight(highlight);
    return *this;
  }

  TextView & clear_highlights()
  {
    inner.text.clear_highlights();
    return *this;
  }

  TextView & cursor_highlight_style(TextHighlightStyle s)
  {
    styling.cursor_highlight = s;
    return *this;
  }

  TextView & style(TextStyle const & style, FontStyle const & font,
                   u32 first = 0, u32 count = U32_MAX)
  {
    inner.text.style(style, font, first, count);
    return *this;
  }

  TextView & text(Span<c32 const> t)
  {
    inner.text.set_text(t);
    return *this;
  }

  TextView & text(Span<c8 const> t)
  {
    inner.text.set_text(t);
    return *this;
  }

  Span<c32 const> text()
  {
    return inner.text.get_text();
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region,
                         f32 zoom, ViewEvents events, Fn<void(View &)>) override
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

    inner.compositor.command(inner.text.get_text(), inner.text.inner.layout,
                             region.extent.x, inner.text.inner.alignment, cmd,
                             noop, noop, {}, *ctx.clipboard, 1,
                             (ctx.mouse.position - region.center) * zoom);

    return ViewState{.draggable = state.copyable};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    inner.text.layout(allocated.x);
    return {.extent = inner.text.inner.layout.extent};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32 zoom,
                      CRect const & clip) override
  {
    highlight(TextHighlight{.slice = inner.compositor.get_cursor().as_slice(
                                inner.text.get_text().size32()),
                            .style = styling.cursor_highlight});
    inner.text.render(canvas, region, clip, zoom);
    inner.text.inner.highlights.pop();
  }

  virtual Cursor cursor(CRect const &, f32, Vec2) override
  {
    return state.copyable ? Cursor::Text : Cursor::Default;
  }
};

// [ ] scrollable
// [ ] viewport text with scrollable region, scroll direction
// [ ] scroll on pagedown and cursor change
// [ ] text input while in view, i.e. page down
struct TextInput : View
{
  struct State
  {
    bool       disabled       = false;
    bool       editing        = false;
    bool       submit         = false;
    bool       multiline      = false;
    bool       enter_submits  = false;
    bool       tab_input      = false;
    u32        lines_per_page = 1;
    FocusState focus          = {};
  } state;

  struct Style
  {
    TextHighlightStyle highlight = {};
    FocusStyle         focus     = {};
  } styling;

  struct Callbacks
  {
    Fn<void()> edit      = noop;
    Fn<void()> submit    = noop;
    Fn<void()> focus_in  = noop;
    Fn<void()> focus_out = noop;
  } cb;

  struct Inner
  {
    RenderText     content{};
    RenderText     stub{};
    TextCompositor compositor{};
  } inner;

  TextInput()
  {
    inner.content.style(TextStyle{.foreground = DEFAULT_THEME.on_surface},
                        FontStyle{.font        = engine->default_font,
                                  .font_height = DEFAULT_THEME.body_font_height,
                                  .line_height = DEFAULT_THEME.line_height});
    inner.stub.style(TextStyle{.foreground = DEFAULT_THEME.on_surface},
                     FontStyle{.font        = engine->default_font,
                               .font_height = DEFAULT_THEME.body_font_height,
                               .line_height = DEFAULT_THEME.line_height});
  }

  virtual ~TextInput() override = default;

  TextInput & multiline(bool e)
  {
    state.multiline = e;
    return *this;
  }

  TextInput & enter_submits(bool e)
  {
    state.enter_submits = e;

    return *this;
  }

  TextInput & tab_input(bool e)
  {
    state.tab_input = e;
    return *this;
  }

  TextInput & highlight(TextHighlight const & highlight)
  {
    inner.content.inner.highlights.push(highlight).unwrap();
    return *this;
  }

  TextInput & clear_highlights()
  {
    inner.content.inner.highlights.clear();
    return *this;
  }

  TextInput & on_edit(Fn<void()> f)
  {
    cb.edit = f;
    return *this;
  }

  TextInput & on_submit(Fn<void()> f)
  {
    cb.submit = f;
    return *this;
  }

  TextInput & on_focus_in(Fn<void()> f)
  {
    cb.focus_in = f;
    return *this;
  }

  TextInput & on_focus_out(Fn<void()> f)
  {
    cb.focus_out = f;
    return *this;
  }

  TextInput & content(Span<c8 const> t)
  {
    inner.content.set_text(t);
    return *this;
  }

  TextInput & content(Span<c32 const> t)
  {
    inner.content.set_text(t);
    return *this;
  }

  TextInput & content_style(TextStyle const & style, FontStyle const & font,
                            u32 first = 0, u32 count = U32_MAX)
  {
    inner.content.style(style, font, first, count);
    return *this;
  }

  TextInput & stub(Span<c8 const> t)
  {
    inner.stub.set_text(t);
    return *this;
  }

  TextInput & stub(Span<c32 const> t)
  {
    inner.stub.set_text(t);
    return *this;
  }

  TextInput & stub_style(TextStyle const & style, FontStyle const & font,
                         u32 first = 0, u32 count = U32_MAX)
  {
    inner.stub.style(style, font, first, count);
    return *this;
  }

  constexpr TextCommand command(ViewContext const & ctx) const
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
        ctx.key_state(KeyCode::Left) && ctx.mouse_state(MouseButtons::Primary))
    {
      return TextCommand::HitSelect;
    }
    if (state.multiline && !state.enter_submits &&
        ctx.key_state(KeyCode::Return))
    {
      return TextCommand::NewLine;
    }
    if (state.tab_input && ctx.key_state(KeyCode::Tab))
    {
      return TextCommand::Tab;
    }
    return TextCommand::None;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region,
                         f32 zoom, ViewEvents events, Fn<void(View &)>) override
  {
    bool edited = false;
    auto erase  = [this, &edited](Slice32 styling) {
      this->inner.content.inner.text.erase(styling);
      edited |= styling.is_empty();
      this->inner.content.flush_text();
    };

    auto insert = [this, &edited](u32 pos, Span<c32 const> t) {
      CHECK(this->inner.content.inner.text.insert_span_copy(pos, t));
      edited |= t.is_empty();
      this->inner.content.flush_text();
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

    if (inner.content.inner.layout.lines.is_empty())
    {
      state.lines_per_page = 1;
    }
    else
    {
      state.lines_per_page =
          (u32) (region.extent.y /
                 (inner.content.inner.layout.lines[0].metrics.height * zoom));
    }

    Vec<c32> text_input_utf32{default_allocator};

    utf8_decode(ctx.text_input, text_input_utf32).unwrap();

    inner.compositor.command(
        inner.content.inner.text, inner.content.inner.layout, region.extent.x,
        inner.content.inner.alignment, cmd, fn(insert), fn(erase),
        text_input_utf32, *ctx.clipboard, state.lines_per_page,
        (ctx.mouse.position - region.center) * zoom);

    if (edited)
    {
      state.editing = true;
    }

    if (events.focus_out)
    {
      inner.compositor.unselect();
    }

    if (events.key_down && ctx.key_state(KeyCode::Return) &&
        state.enter_submits)
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

    return ViewState{.draggable  = !state.disabled,
                     .focusable  = !state.disabled,
                     .text_input = !state.disabled,
                     .tab_input  = state.tab_input,
                     .grab_focus = events.mouse_down};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    if (inner.content.inner.text.is_empty())
    {
      inner.stub.layout(allocated.x);
      return {.extent = inner.stub.inner.layout.extent};
    }
    inner.content.layout(allocated.x);
    return {.extent = inner.content.inner.layout.extent};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32 zoom,
                      CRect const & clip) override
  {
    if (inner.content.inner.text.is_empty())
    {
      inner.stub.render(canvas, region, clip, zoom);
    }
    else
    {
      highlight(TextHighlight{.slice = inner.compositor.get_cursor().as_slice(
                                  inner.content.inner.text.size32()),
                              .style = styling.highlight});
      inner.content.render(canvas, region, clip, zoom);
      inner.content.inner.highlights.pop();
    }

    if (state.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }

  virtual Cursor cursor(CRect const &, f32, Vec2) override
  {
    return Cursor::Text;
  }
};

struct Button : View
{
  struct State
  {
    bool       disabled = false;
    PressState press    = {};
  } state;

  struct Style
  {
    ColorGradient color          = DEFAULT_THEME.primary;
    ColorGradient hovered_color  = DEFAULT_THEME.primary_variant;
    ColorGradient disabled_color = DEFAULT_THEME.inactive;
    CornerRadii   corner_radii   = Size{.scale = 0.25};
    f32           stroke         = 0.0F;
    f32           thickness      = 1.0F;
    Frame         frame          = {};
    Frame         padding        = {};
    FocusStyle    focus          = {};
  } styling;

  struct Callbacks
  {
    Fn<void()> pressed = noop;
    Fn<void()> hovered = noop;
  } cb;

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents          events, Fn<void(View &)>) override
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

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 size = allocated - 2 * styling.padding(allocated);
    size.x    = max(size.x, 0.0F);
    size.y    = max(size.y, 0.0F);
    fill(sizes, size);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    fill(centers, Vec2{0, 0});
    return {.extent = (sizes.is_empty() ? Vec2{0, 0} : sizes[0]) +
                      2 * styling.padding(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    ColorGradient tint = (state.press.hovered && !state.press.held) ?
                             styling.hovered_color :
                             styling.color;
    tint               = state.disabled ? styling.disabled_color : tint;
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = styling.corner_radii(region.extent.y),
                  .stroke       = styling.stroke,
                  .thickness    = styling.thickness,
                  .tint         = tint});

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }
};

struct TextButton : Button
{
  struct Inner
  {
    TextView text{};
  } inner;

  TextButton()
  {
    inner.text.state.copyable = false;
  }

  virtual ~TextButton() override = default;

  TextButton & disable(bool d)
  {
    state.disabled = d;
    return *this;
  }

  TextButton & style(TextStyle const & style, FontStyle const & font,
                     u32 first = 0, u32 count = U32_MAX)
  {
    inner.text.style(style, font, first, count);
    return *this;
  }

  TextButton & text(Span<c32 const> t)
  {
    inner.text.text(t);
    return *this;
  }

  TextButton & text(Span<c8 const> t)
  {
    inner.text.text(t);
    return *this;
  }

  TextButton & color(ColorGradient c)
  {
    styling.color = c;
    return *this;
  }

  TextButton & hovered_color(ColorGradient c)
  {
    styling.hovered_color = c;
    return *this;
  }

  TextButton & disabled_color(ColorGradient c)
  {
    styling.disabled_color = c;
    return *this;
  }

  TextButton & corner_radii(CornerRadii c)
  {
    styling.corner_radii = c;
    return *this;
  }

  TextButton & frame(f32 width, f32 height, bool constrain = true)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  TextButton & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  TextButton & padding(f32 x, f32 y)
  {
    styling.padding = Frame{x, y};
    return *this;
  }

  TextButton & padding(Frame p)
  {
    styling.padding = p;
    return *this;
  }

  TextButton & on_pressed(Fn<void()> f)
  {
    cb.pressed = f;
    return *this;
  }

  TextButton & on_hovered(Fn<void()> f)
  {
    cb.hovered = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region,
                         f32 zoom, ViewEvents events,
                         Fn<void(View &)> build) override
  {
    ViewState state = Button::tick(ctx, region, zoom, events, build);
    build(inner.text);
    return state;
  }
};

struct CheckBox : View
{
  struct State
  {
    bool       disabled = false;
    PressState press    = {};
    bool       value    = false;
  } state;

  struct Style
  {
    ColorGradient box_color         = DEFAULT_THEME.inactive;
    ColorGradient box_hovered_color = DEFAULT_THEME.active;
    ColorGradient tick_color        = DEFAULT_THEME.primary;
    f32           stroke            = 1;
    f32           thickness         = 1;
    f32           tick_thickness    = 1.5F;
    CornerRadii   corner_radii      = Size{.scale = 0.125F};
    Frame         frame{20, 20};
    FocusStyle    focus = {};
  } styling;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  CheckBox & disable(bool d)
  {
    state.disabled = d;
    return *this;
  }

  CheckBox & box_color(ColorGradient c)
  {
    styling.box_color = c;
    return *this;
  }

  CheckBox & box_hovered_color(ColorGradient c)
  {
    styling.box_hovered_color = c;
    return *this;
  }

  CheckBox & tick_color(ColorGradient c)
  {
    styling.tick_color = c;
    return *this;
  }

  CheckBox & stroke(f32 s)
  {
    styling.stroke = s;
    return *this;
  }

  CheckBox & thickness(f32 t)
  {
    styling.thickness = t;
    return *this;
  }

  CheckBox & tick_thickness(f32 t)
  {
    styling.tick_thickness = t;
    return *this;
  }

  CheckBox & corner_radii(CornerRadii r)
  {
    styling.corner_radii = r;
    return *this;
  }

  CheckBox & frame(f32 width, f32 height, bool constrain = true)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  CheckBox & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  CheckBox & on_changed(Fn<void(bool)> f)
  {
    cb.changed = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents          events, Fn<void(View &)>) override
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

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    Vec2 extent = styling.frame(allocated);
    return {.extent = Vec2::splat(min(extent.x, extent.y))};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    ColorGradient tint =
        (state.press.hovered && !state.press.held && !state.disabled) ?
            styling.box_hovered_color :
            styling.box_color;
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = styling.corner_radii(region.extent.y),
                  .stroke       = 1,
                  .thickness    = 2,
                  .tint         = tint});

    if (state.value)
    {
      canvas.line(
          {
              .center    = region.center,
              .extent    = region.extent,
              .stroke    = 0,
              .thickness = styling.tick_thickness,
              .tint      = styling.tick_color
      },
          span<Vec2>({{0.125F, 0.5F}, {0.374F, 0.75F}, {0.775F, 0.25F}}));
    }

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }
};

/// @brief Multi-directional Slider
struct Slider : View
{
  struct State
  {
    bool      disabled = false;
    DragState drag     = {};
    f32       t        = 0;
    f32       low      = 0;
    f32       high     = 1;
  } state;

  struct Style
  {
    Axis          axis = Axis::X;
    Frame         frame{100, 20};
    Frame         thumb_frame{20, 20};
    Frame         track_frame{100, 20};
    ColorGradient thumb_color          = DEFAULT_THEME.primary;
    ColorGradient thumb_hovered_color  = DEFAULT_THEME.primary;
    ColorGradient thumb_dragging_color = DEFAULT_THEME.primary;
    CornerRadii   thumb_corner_radii   = Size{.scale = 0.125F};
    ColorGradient track_color          = DEFAULT_THEME.inactive;
    CornerRadii   track_corner_radii   = Size{.scale = 0.125F};
    FocusStyle    focus                = {};
    f32           delta                = 0.1F;
  } styling;

  struct Callbacks
  {
    Fn<void(f32)> changed = noop;
  } cb;

  Slider & range(f32 low, f32 high)
  {
    state.low  = low;
    state.high = high;
    return *this;
  }

  Slider & interp(f32 t)
  {
    state.t = t;
    return *this;
  }

  Slider & axis(Axis a)
  {
    styling.axis = a;
    return *this;
  }

  Slider & frame(f32 width, f32 height, bool constrain = true)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  Slider & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  Slider & thumb_frame(f32 width, f32 height, bool constrain = true)
  {
    styling.thumb_frame = Frame{width, height, constrain};
    return *this;
  }

  Slider & thumb_frame(Frame f)
  {
    styling.thumb_frame = f;
    return *this;
  }

  Slider & track_frame(f32 width, f32 height, bool constrain = true)
  {
    styling.track_frame = Frame{width, height, constrain};
    return *this;
  }

  Slider & track_frame(Frame f)
  {
    styling.track_frame = f;
    return *this;
  }

  Slider & thumb_color(ColorGradient c)
  {
    styling.thumb_color = c;
    return *this;
  }

  Slider & thumb_hovered_color(ColorGradient c)
  {
    styling.thumb_hovered_color = c;
    return *this;
  }

  Slider & thumb_dragging_color(ColorGradient c)
  {
    styling.thumb_dragging_color = c;
    return *this;
  }

  Slider & thumb_corner_radii(CornerRadii c)
  {
    styling.thumb_corner_radii = c;
    return *this;
  }

  Slider & track_color(ColorGradient c)
  {
    styling.track_color = c;
    return *this;
  }

  Slider & track_corner_radii(CornerRadii c)
  {
    styling.track_corner_radii = c;
    return *this;
  }

  Slider & on_changed(Fn<void(f32)> f)
  {
    cb.changed = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents events, Fn<void(View &)>) override
  {
    u8 const main_axis = (styling.axis == Axis::X) ? 0 : 1;

    state.drag.tick(events);

    if (state.drag.dragging)
    {
      Vec2 const track_extent = styling.track_frame(region.extent);
      Vec2 const track_begin  = region.center - track_extent * 0.5F;
      Vec2 const track_end    = region.center + track_extent * 0.5F;
      state.t = clamp(unlerp(track_begin[main_axis], track_end[main_axis],
                             ctx.mouse.position[main_axis]),
                      0.0F, 1.0F);
      f32 const value =
          clamp(lerp(state.low, state.high, state.t), state.low, state.high);
      cb.changed(value);
    }

    if (state.drag.focus.focused)
    {
      if ((styling.axis == Axis::X && ctx.key_state(KeyCode::Left)) ||
          (styling.axis == Axis::Y && ctx.key_state(KeyCode::Up)))
      {
        state.t = max(state.t - styling.delta, 0.0F);
      }
      else if ((styling.axis == Axis::X && ctx.key_state(KeyCode::Right)) ||
               (styling.axis == Axis::Y && ctx.key_state(KeyCode::Down)))
      {
        state.t = min(state.t + styling.delta, 1.0F);
      }
    }

    return ViewState{.pointable = !state.disabled,
                     .draggable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = styling.frame(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    u8 const      main_axis   = (styling.axis == Axis::X) ? 0 : 1;
    u8 const      cross_axis  = (styling.axis == Axis::X) ? 1 : 0;
    Vec2 const    frame       = region.extent;
    Vec2 const    track_frame = styling.track_frame(frame);
    ColorGradient thumb_color;

    if (state.drag.dragging)
    {
      thumb_color = styling.thumb_dragging_color;
    }
    else if (state.drag.hovered)
    {
      thumb_color = styling.thumb_hovered_color;
    }
    else
    {
      thumb_color = styling.thumb_color;
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

    Vec2 const track_begin = region.center - track_frame * 0.5F;

    Vec2 thumb_center;
    thumb_center[main_axis] =
        track_begin[main_axis] + track_frame[main_axis] * state.t;
    thumb_center[cross_axis] = region.center[cross_axis];

    Vec2 const thumb_extent = styling.thumb_frame(region.extent) * dilation;

    canvas
        .rrect({.center       = region.center,
                .extent       = track_frame,
                .corner_radii = styling.track_corner_radii(region.extent.y),
                .tint         = styling.track_color})
        .rrect({.center       = thumb_center,
                .extent       = thumb_extent,
                .corner_radii = styling.thumb_corner_radii(thumb_extent.y),
                .tint         = thumb_color});

    if (state.drag.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }
};

struct Switch : View
{
  struct State
  {
    bool       disabled = false;
    PressState press    = {};
    bool       value    = false;
  } state;

  struct Style
  {
    ColorGradient on_color          = DEFAULT_THEME.primary;
    ColorGradient on_hovered_color  = DEFAULT_THEME.primary_variant;
    ColorGradient off_color         = DEFAULT_THEME.active;
    ColorGradient off_hovered_color = DEFAULT_THEME.inactive;
    ColorGradient track_color       = DEFAULT_THEME.inactive;
    CornerRadii   corner_radii      = Size{.scale = 0.125F};
    Frame         frame{40, 20};
    Frame         thumb_frame{17.5, 17.5};
    FocusStyle    focus = {};
  } styling;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  Switch & on()
  {
    state.value = true;
    cb.changed(true);
    return *this;
  }

  Switch & off()
  {
    state.value = false;
    cb.changed(false);
    return *this;
  }

  Switch & toggle()
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

  Switch & on_color(ColorGradient c)
  {
    styling.on_color = c;
    return *this;
  }

  Switch & on_hovered_color(ColorGradient c)
  {
    styling.on_hovered_color = c;
    return *this;
  }

  Switch & off_color(ColorGradient c)
  {
    styling.off_color = c;
    return *this;
  }

  Switch & off_hovered_color(ColorGradient c)
  {
    styling.off_hovered_color = c;
    return *this;
  }

  Switch & track_color(ColorGradient c)
  {
    styling.track_color = c;
    return *this;
  }

  Switch & corner_radii(CornerRadii r)
  {
    styling.corner_radii = r;
    return *this;
  }

  Switch & frame(f32 width, f32 height, bool constrain = true)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  Switch & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  Switch & thumb_frame(f32 width, f32 height, bool constrain = true)
  {
    styling.thumb_frame = Frame{width, height, constrain};
    return *this;
  }

  Switch & thumb_frame(Frame f)
  {
    styling.thumb_frame = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents          events, Fn<void(View &)>) override
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

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = styling.frame(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    Vec2 const thumb_extent = styling.thumb_frame(region.extent);
    Vec2 const thumb_center =
        region.center + space_align(region.extent, thumb_extent,
                                    Vec2{state.value ? 1.0F : -1.0F, 0});

    ColorGradient thumb_color;
    if (state.press.hovered)
    {
      thumb_color =
          state.value ? styling.on_hovered_color : styling.off_hovered_color;
    }
    else
    {
      thumb_color = state.value ? styling.on_color : styling.off_color;
    }

    canvas
        .rrect({.center       = region.center,
                .extent       = region.extent,
                .corner_radii = styling.corner_radii(region.extent.y),
                .tint         = styling.track_color})
        .rrect({.center       = thumb_center,
                .extent       = thumb_extent,
                .corner_radii = styling.corner_radii(region.extent.y),
                .tint         = thumb_color});

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }
};

struct RadioBox : View
{
  struct State
  {
    bool       disabled = false;
    PressState press    = {};
    bool       value    = false;
  } state;

  struct Style
  {
    Frame         frame{20, 20};
    CornerRadii   corner_radii        = Size{.scale = 0.125F};
    f32           thickness           = 1.0F;
    ColorGradient color               = DEFAULT_THEME.inactive;
    ColorGradient inner_color         = DEFAULT_THEME.primary;
    ColorGradient inner_hovered_color = DEFAULT_THEME.primary_variant;
    FocusStyle    focus               = {};
  } styling;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  RadioBox & corner_radii(CornerRadii c)
  {
    styling.corner_radii = c;
    return *this;
  }

  RadioBox & thickness(f32 t)
  {
    styling.thickness = t;
    return *this;
  }

  RadioBox & color(ColorGradient c)
  {
    styling.color = c;
    return *this;
  }

  RadioBox & inner_color(ColorGradient c)
  {
    styling.inner_color = c;
    return *this;
  }

  RadioBox & inner_hovered_color(ColorGradient c)
  {
    styling.inner_hovered_color = c;
    return *this;
  }

  RadioBox & frame(f32 width, f32 height, bool constrain = true)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  RadioBox & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  RadioBox & on_changed(Fn<void(bool)> f)
  {
    cb.changed = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents          events, Fn<void(View &)>) override
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

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = styling.frame(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = styling.corner_radii(region.extent.y),
                  .stroke       = 1,
                  .thickness    = styling.thickness,
                  .tint         = styling.color});

    if (state.value)
    {
      Vec2 inner_extent = region.extent * (state.press.hovered ? 0.75F : 0.5F);
      ColorGradient inner_color = state.press.hovered ?
                                      styling.inner_hovered_color :
                                      styling.inner_color;

      canvas.circle({.center = region.center,
                     .extent = inner_extent,
                     .tint   = inner_color});
    }

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }
};

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
    i32 i32 = 0;
    f32 f32;
  };

  ScalarInputType type = ScalarInputType::i32;
};

namespace fmt
{

inline bool push(Context const & ctx, Spec const & spec,
                 ScalarInput const & value)
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
  ScalarInput max     = {.i32 = 200};
  ScalarInput step    = {};
  ScalarInput current = {.i32 = 100};

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
        return clamp(unlerp((f32) min.i32, (f32) max.i32, (f32) current.i32),
                     0.0F, 1.0F);
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
  return ScalarState{
      .base = {.f32 = base, .type = ScalarInputType::i32},
      .min  = {.f32 = min,  .type = ScalarInputType::i32},
      .max  = {.f32 = max,  .type = ScalarInputType::i32},
      .step = {.f32 = step, .type = ScalarInputType::i32}
  };
}

constexpr ScalarState scalar(i32 base, i32 min, i32 max, i32 step)
{
  return ScalarState{
      .base = {.i32 = base, .type = ScalarInputType::i32},
      .min  = {.i32 = min,  .type = ScalarInputType::i32},
      .max  = {.i32 = max,  .type = ScalarInputType::i32},
      .step = {.i32 = step, .type = ScalarInputType::i32}
  };
}

struct ScalarDragBox : View
{
  typedef Fn<void(fmt::Context const &, ScalarInput)> Fmt;
  typedef Fn<void(Span<c32 const>, ScalarState &)>    Parse;

  struct State
  {
    bool        disabled   = false;
    bool        input_mode = false;
    bool        dragging   = false;
    FocusState  focus      = {};
    ScalarState value      = {};
  } state;

  struct Style
  {
    Frame         frame = Frame{}.min(100, DEFAULT_THEME.body_font_height + 10);
    Frame         padding{5, 5};
    Size          thumb_width  = {5};
    CornerRadii   corner_radii = Size{.scale = 0.125F};
    ColorGradient color        = DEFAULT_THEME.inactive;
    ColorGradient thumb_color  = DEFAULT_THEME.inactive;
    f32           stroke       = 1.0F;
    f32           thickness    = 1.0F;
    FocusStyle    focus        = {};
    Fmt           fmt          = fn(scalar_fmt);
    Parse         parse        = fn(scalar_parse);
  } styling;

  struct Inner
  {
    TextInput input{};
  } inner;

  struct Callbacks
  {
    Fn<void(ScalarInput)> update = noop;
  } cb;

  ScalarDragBox()
  {
    inner.input.multiline(false).tab_input(false).enter_submits(false);
  }

  virtual ~ScalarDragBox() override
  {
  }

  static void scalar_fmt(fmt::Context const & ctx, ScalarInput v)
  {
    fmt::format(ctx, v);
  }

  static void scalar_parse(Span<c32 const> text, ScalarState & styling);

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents events, Fn<void(View &)> build) override
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
      state.value.interp(t);
    }

    if (inner.input.state.editing)
    {
      styling.parse(inner.input.inner.content.get_text(), state.value);
    }
    else
    {
      char         scratch[128];
      c8           text[128];
      Buffer       buffer = ash::buffer(span(text).as_char());
      fmt::Context ctx    = fmt::buffer(buffer, scratch);
      styling.fmt(ctx, state.value.current);
      inner.input.inner.content.set_text(span(text).slice(0, buffer.size()));
    }

    inner.input.state.disabled = !state.input_mode;

    if (inner.input.state.editing || state.dragging)
    {
      cb.update(state.value.current);
    }

    state.focus.tick(events);

    build(inner.input);

    return ViewState{.pointable = !state.disabled,
                     .draggable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 child = styling.frame(allocated) - 2 * styling.padding(allocated);
    child.x    = max(child.x, 0.0F);
    child.y    = max(child.y, 0.0F);
    fill(sizes, child);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    fill(centers, Vec2{0, 0});
    return {.extent = sizes[0] + 2 * styling.padding(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = styling.corner_radii(region.extent.y),
                  .stroke       = styling.stroke,
                  .thickness    = styling.thickness,
                  .tint         = styling.color});

    if (!state.input_mode)
    {
      f32 const  t = state.value.uninterp();
      Vec2 const thumb_extent{styling.thumb_width(region.extent.x),
                              region.extent.y};
      Vec2       thumb_center = region.center;
      thumb_center.x +=
          space_align(region.extent.x, thumb_extent.x, norm_to_axis(t));

      canvas.rrect({.center       = thumb_center,
                    .extent       = thumb_extent,
                    .corner_radii = Vec4::splat(region.extent.y * 0.125F),
                    .tint         = styling.thumb_color});
    }

    if (state.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }

  virtual Cursor cursor(CRect const & region, f32, Vec2 offset) override
  {
    (void) region;
    (void) offset;
    return state.disabled ? Cursor::Default : Cursor::EWResize;
  }
};

struct ScalarBox : FlexView
{
  struct Callbacks
  {
    Fn<void(ScalarInput)> update = noop;
  } cb;

  struct Inner
  {
    TextButton    dec{};
    TextButton    inc{};
    ScalarDragBox drag{};
  } inner;

  ScalarBox()
  {
    FlexView::axis(Axis::X)
        .wrap(false)
        .main_align(MainAlign::Start)
        .cross_align(0)
        .frame(Frame{}.scale(1, 1));

    inner.dec.text(U"-"_str)
        .style(
            TextStyle{
                .shadow_scale  = 1,
                .shadow_offset = {1, 1},
                .foreground    = DEFAULT_THEME.on_primary,
                .shadow        = colors::BLACK
    },
            FontStyle{.font        = engine->default_font,
                      .font_height = DEFAULT_THEME.body_font_height,
                      .line_height = 1})
        .on_pressed(fn(
            this,
            +[](ScalarBox * b) {
              b->inner.drag.state.value.step_value(-1);
              b->cb.update(b->inner.drag.state.value.current);
            }))
        .padding(5, 5);

    inner.inc.text(U"+"_str)
        .style(
            TextStyle{
                .shadow_scale  = 1,
                .shadow_offset = {1, 1},
                .foreground    = DEFAULT_THEME.on_primary,
                .shadow        = colors::BLACK
    },
            FontStyle{.font        = engine->default_font,
                      .font_height = DEFAULT_THEME.body_font_height,
                      .line_height = 1})
        .on_pressed(fn(
            this,
            +[](ScalarBox * b) {
              b->inner.drag.state.value.step_value(1);
              b->cb.update(b->inner.drag.state.value.current);
            }))
        .padding(5, 5);

    // [ ] color, stroke color, etc. the rectangles at small sizes seem to have
    // bad border radii.
    //
    // [ ] probably due to incorrect rrect rendering
    //
    // [ ] fix interpolation of slider
    //

    inner.drag.cb.update =
        fn(this, +[](ScalarBox * b, ScalarInput in) { b->cb.update(in); });

    // [ ] set drag box style: create similar methods for it
    // [ ] all views must have these methods
    // inner.drag.
  }

  virtual ~ScalarBox() override = default;

  // [ ] placeholder

  ScalarBox & stroke(f32 s)
  {
    inner.drag.styling.stroke = s;
    return *this;
  }

  ScalarBox & thickness(f32 t)
  {
    inner.drag.styling.thickness = t;
    return *this;
  }

  ScalarBox & padding(f32 x, f32 y)
  {
    inner.dec.padding(x, y);
    inner.inc.padding(x, y);
    inner.drag.styling.padding = Frame{x, y};
    return *this;
  }

  ScalarBox & padding(Frame p)
  {
    inner.dec.padding(p);
    inner.inc.padding(p);
    inner.drag.styling.padding = p;
    return *this;
  }

  ScalarBox & frame(f32 width, f32 height, bool constrain = true)
  {
    inner.dec.frame(width, height, constrain);
    inner.inc.frame(width, height, constrain);
    inner.drag.styling.frame = Frame{width, height, constrain};
    return *this;
  }

  ScalarBox & frame(Frame f)
  {
    inner.dec.frame(f);
    inner.inc.frame(f);
    inner.drag.styling.frame = f;
    return *this;
  }

  ScalarBox & corner_radii(CornerRadii r)
  {
    inner.dec.corner_radii(r);
    inner.inc.corner_radii(r);
    inner.drag.styling.corner_radii = r;
    return *this;
  }

  ScalarBox & on_update(Fn<void(ScalarInput)> f)
  {
    cb.update = f;
    return *this;
  }

  ScalarBox & text_style(TextStyle const & style, FontStyle const & font,
                         u32 first, u32 count)
  {
    inner.dec.style(style, font, first, count);
    inner.inc.style(style, font, first, count);
    inner.drag.inner.input.content_style(style, font, first, count)
        .stub_style(style, font, first, count);
    return *this;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32, ViewEvents,
                         Fn<void(View &)> build) override
  {
    build(inner.dec);
    build(inner.drag);
    build(inner.inc);
    return ViewState{};
  }
};

struct ScrollBar : View
{
  struct State
  {
    bool      disabled = false;
    bool      hidden   = false;
    DragState drag     = {};
    f32       t        = 0;
  } state;

  struct Style
  {
    Axis          axis                 = Axis::X;
    f32           content_extent       = 1;
    ColorGradient thumb_color          = DEFAULT_THEME.primary * opacity(0.5F);
    ColorGradient thumb_hovered_color  = DEFAULT_THEME.primary * opacity(0.75F);
    ColorGradient thumb_dragging_color = DEFAULT_THEME.primary;
    CornerRadii   thumb_corner_radii   = Size{};
    ColorGradient track_color        = DEFAULT_THEME.inactive * opacity(0.75F);
    CornerRadii   track_corner_radii = Size{};
    FocusStyle    focus              = {};
    f32           delta              = 0.1F;
  } styling;

  struct Callbacks
  {
    Fn<void(f32)> scrolled = noop;
  } cb;

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents events, Fn<void(View &)>) override
  {
    u8 const main_axis = (styling.axis == Axis::X) ? 0 : 1;

    state.drag.tick(events);

    if (state.drag.dragging)
    {
      state.t =
          clamp((ctx.mouse.position[main_axis] - region.extent[main_axis] / 2) /
                    region.extent[main_axis],
                0.0F, 1.0F);
      cb.scrolled(state.t);
    }

    if (state.drag.focus.focused)
    {
      if ((styling.axis == Axis::X && ctx.key_state(KeyCode::Left)) ||
          (styling.axis == Axis::Y && ctx.key_state(KeyCode::Up)))
      {
        state.t = max(state.t - styling.delta, 0.0F);
      }
      else if ((styling.axis == Axis::X && ctx.key_state(KeyCode::Right)) ||
               (styling.axis == Axis::Y && ctx.key_state(KeyCode::Down)))
      {
        state.t = min(state.t + styling.delta, 1.0F);
      }
    }

    return ViewState{.hidden    = state.hidden,
                     .pointable = !state.disabled,
                     .draggable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = allocated};
  }

  virtual i32 stack(i32 allocated) override
  {
    // needs to be at a different stacking context since this will be placed
    // on top of the viewport
    return allocated + 1;
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    u8 const   main_axis          = (styling.axis == Axis::X) ? 0 : 1;
    u8 const   cross_axis         = (styling.axis == Axis::X) ? 1 : 0;
    Vec4 const thumb_corner_radii = styling.thumb_corner_radii(region.extent.y);
    Vec4 const track_corner_radii = styling.track_corner_radii(region.extent.y);

    // calculate thumb main axis extent
    f32 const thumb_scale = region.extent[main_axis] / styling.content_extent;
    Vec2      thumb_extent;
    thumb_extent[cross_axis] = region.extent[cross_axis];
    thumb_extent[main_axis]  = thumb_scale * region.extent[main_axis];

    // align thumb to remaining space based on size of visible region
    Vec2 const bar_offset  = region.begin();
    f32 const main_spacing = thumb_extent[main_axis] - region.extent[main_axis];
    Vec2      thumb_center;
    thumb_center[main_axis] = bar_offset[main_axis] + main_spacing * state.t +
                              thumb_extent[main_axis] * 0.5F;
    thumb_center[cross_axis] = region.center[cross_axis];

    ColorGradient thumb_color;
    if (state.drag.dragging)
    {
      thumb_color = styling.thumb_dragging_color;
    }
    else if (state.drag.hovered)
    {
      thumb_color = styling.thumb_hovered_color;
    }
    else
    {
      thumb_color = styling.thumb_color;
    }

    canvas
        .rrect({.center       = region.center,
                .extent       = region.extent,
                .corner_radii = track_corner_radii,
                .stroke       = 0,
                .tint         = styling.track_color})
        .rrect({.center       = thumb_center,
                .extent       = thumb_extent,
                .corner_radii = thumb_corner_radii,
                .stroke       = 0,
                .tint         = thumb_color});

    if (state.drag.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }
};

struct ScrollViewFrame : View
{
  struct State
  {
    Vec2 t              = {0, 0};
    f32  zoom           = 1;
    Vec2 content_extent = {};
  } state;

  struct Inner
  {
    Option<View *> child = None;
  } inner;

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    if (inner.child)
    {
      build(*inner.child.value());
    }

    if (events.mouse_scroll)
    {
      state.t += (ctx.mouse.wheel_translation / region.extent);
      state.t.x = clamp(state.t.x, 0.0F, 1.0F);
      state.t.x = clamp(state.t.y, 0.0F, 1.0F);
    }

    return ViewState{.viewport = true};
  }

  virtual void size(Vec2, Span<Vec2> sizes) override
  {
    if (!sizes.is_empty())
    {
      sizes[0] = {F32_MAX, F32_MAX};
    }
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2 content_size;
    if (!sizes.is_empty())
    {
      content_size = sizes[0];
    }
    fill(centers, Vec2{0, 0});

    return {.extent          = allocated,
            .viewport_extent = state.content_extent,
            .viewport_transform =
                scroll_transform(content_size, allocated, state.t, state.zoom)};
  }
};

struct ScrollView : View
{
  struct State
  {
    bool disabled = false;
  } state;

  struct Style
  {
    Axes  axes = Axes::X | Axes::Y;
    Frame frame{200, 200};
    Size  x_bar_size = {.offset = 10};
    Size  y_bar_size = {.offset = 10};
  } styling;

  struct Inner
  {
    ScrollViewFrame view_frame{};
    ScrollBar       x_bar{};
    ScrollBar       y_bar{};
  } inner;

  ScrollView()
  {
    inner.x_bar.styling.axis = Axis::X;
    inner.y_bar.styling.axis = Axis::Y;
  }

  virtual ~ScrollView() override = default;

  ScrollView & disable(bool d)
  {
    state.disabled             = d;
    inner.x_bar.state.disabled = d;
    inner.y_bar.state.disabled = d;
    return *this;
  }

  ScrollView & item(View & v)
  {
    inner.view_frame.inner.child = Some{&v};
    return *this;
  }

  ScrollView & item(NoneType)
  {
    inner.view_frame.inner.child = None;
    return *this;
  }

  ScrollView & thumb_color(ColorGradient c)
  {
    inner.x_bar.styling.thumb_color = c;
    inner.y_bar.styling.thumb_color = c;
    return *this;
  }

  ScrollView & thumb_hovered_color(ColorGradient c)
  {
    inner.x_bar.styling.thumb_hovered_color = c;
    inner.y_bar.styling.thumb_hovered_color = c;
    return *this;
  }

  ScrollView & thumb_dragging_color(ColorGradient c)
  {
    inner.x_bar.styling.thumb_dragging_color = c;
    inner.y_bar.styling.thumb_dragging_color = c;
    return *this;
  }

  ScrollView & thumb_corner_radii(CornerRadii c)
  {
    inner.x_bar.styling.thumb_corner_radii = c;
    inner.y_bar.styling.thumb_corner_radii = c;
    return *this;
  }

  ScrollView & track_color(ColorGradient c)
  {
    inner.x_bar.styling.track_color = c;
    inner.y_bar.styling.track_color = c;
    return *this;
  }

  ScrollView & track_corner_radii(CornerRadii c)
  {
    inner.x_bar.styling.track_corner_radii = c;
    inner.y_bar.styling.track_corner_radii = c;
    return *this;
  }

  ScrollView & axes(Axes a)
  {
    styling.axes             = a;
    inner.x_bar.state.hidden = has_bits(a, Axes::X);
    inner.y_bar.state.hidden = has_bits(a, Axes::Y);
    return *this;
  }

  ScrollView & frame(f32 width, f32 height, bool constrain)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  ScrollView & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  ScrollView & bar_size(f32 x, f32 y)
  {
    styling.x_bar_size = Size{.offset = x};
    styling.y_bar_size = Size{.offset = y};
    return *this;
  }

  ScrollView & bar_size(Size x, Size y)
  {
    styling.x_bar_size = x;
    styling.y_bar_size = y;
    return *this;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32, ViewEvents,
                         Fn<void(View &)> build) override
  {
    inner.view_frame.state.t = {inner.x_bar.state.t, inner.y_bar.state.t};
    build(inner.view_frame);
    build(inner.x_bar);
    build(inner.y_bar);
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame      = styling.frame(allocated);
    f32 const  x_bar_size = styling.x_bar_size(allocated.x);
    f32 const  y_bar_size = styling.y_bar_size(allocated.y);

    sizes[0] = frame;
    sizes[1] = {frame.x, x_bar_size};

    if (has_bits(styling.axes, Axes::X | Axes::Y))
    {
      sizes[1].x -= y_bar_size;
    }

    sizes[2] = {y_bar_size, frame.y};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2 const frame = styling.frame(allocated);
    centers[0]       = {0, 0};
    centers[1]       = space_align(frame, sizes[1], ALIGNMENT_BOTTOM_LEFT);
    centers[2]       = space_align(frame, sizes[2], ALIGNMENT_TOP_RIGHT);
    inner.x_bar.styling.content_extent =
        inner.view_frame.state.content_extent.x;
    inner.y_bar.styling.content_extent =
        inner.view_frame.state.content_extent.y;

    return {.extent = frame};
  }
};

struct ComboBoxItem : View
{
  struct State
  {
    bool                        disabled = false;
    PressState                  press    = {};
    Option<Option<u32> const *> selected = None;
    u32                         index    = 0;
  } state;

  struct Style
  {
    ColorGradient hovered_text_color        = DEFAULT_THEME.active;
    ColorGradient hovered_background_color  = DEFAULT_THEME.surface_variant;
    ColorGradient disabled_text_color       = DEFAULT_THEME.inactive;
    ColorGradient disabled_background_color = DEFAULT_THEME.inactive;
    ColorGradient selected_background_color = DEFAULT_THEME.surface_variant;
    ColorGradient selected_text_color       = DEFAULT_THEME.surface_variant;
    ColorGradient text_color                = DEFAULT_THEME.on_surface;
    ColorGradient background_color          = DEFAULT_THEME.surface;
    CornerRadii   corner_radii              = Size{.scale = 0.125F};
    FocusStyle    focus                     = {};
    Frame         frame                     = {};
    f32           alignment                 = 0;
  };

  Option<Style const *> style = None;

  struct Callbacks
  {
    Fn<void(Option<u32>)> selected = noop;
  } cb;

  struct Inner
  {
    Option<View *> child = None;
  } inner;

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    state.press.tick(ctx, events);

    if (state.press.down && !state.selected)
    {
      cb.selected(Some{state.index});
    }

    if (inner.child)
    {
      build(*inner.child.value());
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, allocated);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    for (u32 i = 0; i < centers.size32(); i++)
    {
      centers[i] =
          space_align(allocated, sizes[i], Vec2{style.value()->alignment, 0});
    }
    return {.extent = allocated};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    canvas.rrect({.center = region.center,
                  .extent = region.extent,
                  .tint   = state.press.hovered ?
                                style.value()->hovered_background_color :
                                style.value()->background_color});

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.value()->focus.border_thickness,
                   .tint      = style.value()->focus.border_color});
    }
  }
};

struct TextComboBoxItem : ComboBoxItem
{
  struct Inner
  {
    TextView text{};
  } inner;

  TextComboBoxItem()
  {
    inner.text.state.copyable = false;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region,
                         f32 zoom, ViewEvents events,
                         Fn<void(View &)> build) override
  {
    build(inner.text);

    return ComboBoxItem::tick(ctx, region, zoom, events, build);
  }
};

struct ComboBoxScrollView : View
{
  struct State
  {
    bool disabled = false;
    bool opened   = false;
    f32  t        = 0;
  } state;

  struct Style
  {
    Frame         frame{150, 450};
    Size          item_height  = {25};
    CornerRadii   corner_radii = Size{.scale = 0.125F};
    f32           alignment    = 0;
    ColorGradient color        = DEFAULT_THEME.surface;
    f32           scroll_delta = 0;
  } styling;

  struct Inner
  {
    Vec<ComboBoxItem *> items{default_allocator};
  } inner;

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    for (ComboBoxItem * item : inner.items)
    {
      build(*item);
    }

    if (events.mouse_scroll)
    {
      state.t += (ctx.mouse.wheel_translation.y / region.extent.y);
      state.t = clamp(state.t, 0.0F, 1.0F);
    }

    return ViewState{.hidden = !state.opened, .scrollable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, Vec2{styling.frame.width(allocated.x),
                     styling.item_height(allocated.y)});
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2 size = styling.frame(allocated);

    f32 items_height = 0;
    for (Vec2 const & styling : sizes)
    {
      items_height += styling.y;
    }

    //[ ] centers

    size.y = min(size.y, items_height);

    Vec2 viewport_extent{size.x, items_height};

    return ViewLayout{.extent             = size,
                      .viewport_extent    = viewport_extent,
                      .viewport_transform = scroll_transform(
                          viewport_extent, size, Vec2{state.t, 0.5F}, 1),
                      .fixed_position = Some{Vec2{0, 0}}};
  }

  virtual i32 stack(i32 allocated) override
  {
    return allocated + 25;
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = styling.corner_radii(region.extent.y),
                  .tint         = styling.color});
  }
};

// [ ] re-work combobox
// [ ] add list view
struct ComboBox : View
{
  struct State
  {
    bool        disabled = false;
    PressState  press    = {};
    Option<u32> selected = None;
  } state;

  struct Inner
  {
    ComboBoxScrollView     scroll_view{};
    Option<ComboBoxItem *> header = None;
  } inner;

  struct Style
  {
    CornerRadii   corner_radii  = Size{.scale = 0.125F};
    ColorGradient color         = DEFAULT_THEME.surface;
    ColorGradient hovered_color = DEFAULT_THEME.surface_variant;
    f32           alignment     = 0;
    Frame         frame = Frame{}.scale(1, 0).offset(0, 25).max(200, F32_INF);
    FocusStyle    focus = {};
    ComboBoxItem::Style item = {};
  } styling;

  struct Callbacks
  {
    Fn<void(Option<u32>)> selected = noop;
  } cb;

  virtual ~ComboBox() override = default;

  ComboBox & disable(bool d)
  {
    state.disabled = d;
    return *this;
  }

  ComboBox & color(ColorGradient c)
  {
    styling.color = c;
    return *this;
  }

  ComboBox & hovered_color(ColorGradient c)
  {
    styling.hovered_color = c;
    return *this;
  }

  ComboBox & alignment(f32 a)
  {
    styling.alignment = a;
    return *this;
  }

  ComboBox & frame(f32 width, f32 height, bool constrain)
  {
    styling.frame = Frame{width, height, constrain};
    return *this;
  }

  ComboBox & frame(Frame f)
  {
    styling.frame = f;
    return *this;
  }

  ComboBox & align(f32 a)
  {
    styling.alignment = a;
    return *this;
  }

  ComboBox & on_selected(Fn<void(Option<u32>)> styling)
  {
    cb.selected = styling;
    return *this;
  }

  ComboBox & add_item(ComboBoxItem & item)
  {
    u32 const index  = inner.scroll_view.inner.items.size32();
    item.state.index = index;
    item.style       = Some<ComboBoxItem::Style const *>{&styling.item};
    item.cb.selected = fn(
        this, +[](ComboBox * b, Option<u32> item) { b->set_selected(item); });
    item.state.selected = Some<Option<u32> const *>{&state.selected};
    inner.scroll_view.inner.items.push(&item).unwrap();
    return *this;
  }

  ComboBox & set_header(ComboBoxItem & item)
  {
    inner.header     = Some<ComboBoxItem *>{&item};
    item.state.index = 0;
    item.cb.selected = fn(this, +[](ComboBox * b, Option<u32>) { b->open(); });
    item.style       = Some<ComboBoxItem::Style const *>{&styling.item};
    item.state.selected = None;
    return *this;
  }

  u32 num_items() const
  {
    return inner.scroll_view.inner.items.size32();
  }

  ComboBox & set_selected(Option<u32> item)
  {
    CHECK(!(item.is_some() &&
            item.value_ > inner.scroll_view.inner.items.size32()));
    state.selected = item;
    cb.selected(item);
    return *this;
  }

  Option<u32> get_selected() const
  {
    return state.selected;
  }

  bool is_opened() const
  {
    return inner.scroll_view.state.opened;
  }

  ComboBox & close()
  {
    inner.scroll_view.state.opened = false;
    return *this;
  }

  ComboBox & open()
  {
    inner.scroll_view.state.opened = true;
    return *this;
  }

  ComboBox & toggle()
  {
    if (inner.scroll_view.state.opened)
    {
      return open();
    }
    else
    {
      return close();
    }
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    state.press.tick(ctx, events);

    if (state.press.down)
    {
      toggle();
    }

    if (is_opened() && ctx.mouse_down(MouseButtons::All) &&
        !contains(inner.scroll_view.View::inner.region, ctx.mouse.position))
    {
      close();
    }

    build(inner.scroll_view);

    if (inner.header.is_some())
    {
      build(*inner.header.value());
    }

    return ViewState{.clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, styling.frame(allocated));
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> extents,
                         Span<Vec2> centers) override
  {
    Vec2 const frame = styling.frame(allocated);
    for (u32 i = 0; i < extents.size32(); i++)
    {
      centers[i] = space_align(frame, extents[i], Vec2{styling.alignment, 0});
    }
    return {.extent = frame};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      CRect const &) override
  {
    canvas.rrect(
        {.center       = region.center,
         .extent       = region.extent,
         .corner_radii = styling.corner_radii(region.extent.y),
         .tint = state.press.hovered ? styling.hovered_color : styling.color});

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = styling.focus.border_thickness,
                   .tint      = styling.focus.border_color});
    }
  }
};

struct TextComboBox : ComboBox
{
  struct Inner
  {
    TextComboBoxItem header;
  } inner;

  TextComboBox()
  {
    set_header(inner.header);
  }

  virtual ~TextComboBox() override = default;
};

// [ ] implement
/// REQUIREMENTS
/// - Linear and Non-Linear Color Space Independence
/// - Rectangular Box with visualizations
/// - Text-based manual input
/// - RGB, SRGB, HSV, HEX, Linear, Hue, YUV
/// - color space, pixel info for color pickers
struct ColorPicker : View
{
};

/// [ ] implement
/// REQUIREMENTS:
/// - plot modes: histogram, lines, scale, log
/// - plot from user buffer: can be at specific index and will plot rest from
/// head.
struct Plot : View
{
};

// [ ] implement
struct ProgressBar : View
{
};

}        // namespace ash
