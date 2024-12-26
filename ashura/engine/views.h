/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/engine.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
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

// [ ] render hooks?
// [ ] debouncing
struct PressState
{
  bool       in      = false;
  bool       out     = false;
  bool       hovered = false;
  bool       down    = false;
  bool       up      = false;
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

    down = (events.mouse_down && ctx.mouse_down(MouseButton::Primary)) ||
           (events.key_down && ctx.key_down(KeyCode::Return));

    up = (events.mouse_up && ctx.mouse_up(MouseButton::Primary)) ||
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
    Frame     frame       = Frame{}.scale({1, 1});
  } style;

  Vec<ref<View>> items_{default_allocator};

  FlexView & axis(Axis a)
  {
    style.axis = a;
    return *this;
  }

  FlexView & wrap(bool w)
  {
    style.wrap = w;
    return *this;
  }

  FlexView & main_align(MainAlign align)
  {
    style.main_align = align;
    return *this;
  }

  FlexView & cross_align(f32 align)
  {
    style.cross_align = align;
    return *this;
  }

  FlexView & frame(Vec2 extent, bool constrain = true)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  FlexView & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  FlexView & items(std::initializer_list<ref<View>> list)
  {
    return items(span(list));
  }

  FlexView & items(Span<ref<View> const> list)
  {
    items_.extend(list).unwrap();
    return *this;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32,
                         ViewEvents const &, Fn<void(View &)> build) override
  {
    for (ref item : items_)
    {
      build(item);
    }

    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame = style.frame(allocated);
    fill(sizes, frame);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
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
};

struct StackView : View
{
  struct Style
  {
    bool  reverse   = false;
    Vec2  alignment = {0, 0};
    Frame frame     = Frame{}.scale({1, 1});
  } style;

  Vec<ref<View>> items_{default_allocator};

  StackView & reverse(bool r)
  {
    style.reverse = r;
    return *this;
  }

  StackView & align(Vec2 a)
  {
    style.alignment = a;
    return *this;
  }

  StackView & frame(Vec2 extent, bool constrain = true)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  StackView & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  StackView & items(std::initializer_list<ref<View>> list)
  {
    items(span(list));
    return *this;
  }

  StackView & items(Span<ref<View> const> list)
  {
    items_.extend(span(list)).unwrap();
    return *this;
  }

  virtual i32 stack_item(i32 base, u32 i, u32 num)
  {
    // sequential stacking
    i32 z = base;
    if (!style.reverse)
    {
      z += (i32) i;
    }
    else
    {
      z += (i32) (num - i);
    }
    return z;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32,
                         ViewEvents const &, Fn<void(View &)> build) override
  {
    for (ref item : items_)
    {
      build(item);
    }

    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, style.frame(allocated));
  }

  virtual ViewLayout fit(Vec2, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
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
  } style;

  RenderText     text_{};
  TextCompositor compositor_ = TextCompositor::make().unwrap();

  TextView()
  {
    text_.run(TextStyle{.color = DEFAULT_THEME.on_surface},
              FontStyle{.font        = FontId::Default,
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
    text_.highlight(highlight);
    return *this;
  }

  TextView & clear_highlights()
  {
    text_.clear_highlights();
    return *this;
  }

  TextView & cursor_highlight_style(TextHighlightStyle s)
  {
    style.cursor_highlight = s;
    return *this;
  }

  TextView & run(TextStyle const & style, FontStyle const & font, u32 first = 0,
                 u32 count = U32_MAX)
  {
    text_.run(style, font, first, count);
    return *this;
  }

  TextView & text(Span<c32 const> t)
  {
    text_.set_text(t);
    return *this;
  }

  TextView & text(Span<c8 const> t)
  {
    text_.set_text(t);
    return *this;
  }

  Span<c32 const> text()
  {
    return text_.get_text();
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region,
                         f32 zoom, ViewEvents const & events,
                         Fn<void(View &)>) override
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

    compositor_.command(text_.get_text(), text_.layout_, region.extent.x,
                        text_.alignment_, cmd, noop, noop, {},
                        *engine->clipboard, 1,
                        (ctx.mouse.position - region.center) * zoom);

    return ViewState{.draggable = state.copyable};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    text_.layout(allocated.x);
    return {.extent = text_.layout_.extent};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32 zoom,
                      Rect const & clip) override
  {
    highlight(TextHighlight{
      .slice = compositor_.get_cursor().as_slice(text_.get_text().size32()),
      .style = style.cursor_highlight});
    text_.render(canvas, region, clip.centered(), zoom);
    text_.highlights_.pop();
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
// [ ] managing large text input!!! i.e editors
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
  } style;

  struct Callbacks
  {
    Fn<void()> edit      = noop;
    Fn<void()> submit    = noop;
    Fn<void()> focus_in  = noop;
    Fn<void()> focus_out = noop;
  } cb;

  RenderText     content_{};
  RenderText     stub_{};
  TextCompositor compositor_ = TextCompositor::make().unwrap();

  TextInput()
  {
    content_.run(TextStyle{.color = DEFAULT_THEME.on_surface},
                 FontStyle{.font        = FontId::Default,
                           .font_height = DEFAULT_THEME.body_font_height,
                           .line_height = DEFAULT_THEME.line_height});
    stub_.run(TextStyle{.color = DEFAULT_THEME.on_surface},
              FontStyle{.font        = FontId::Default,
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
    content_.highlights_.push(highlight).unwrap();
    return *this;
  }

  TextInput & clear_highlights()
  {
    content_.highlights_.clear();
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
    content_.set_text(t);
    return *this;
  }

  TextInput & content(Span<c32 const> t)
  {
    content_.set_text(t);
    return *this;
  }

  TextInput & content_run(TextStyle const & style, FontStyle const & font,
                          u32 first = 0, u32 count = U32_MAX)
  {
    content_.run(style, font, first, count);
    return *this;
  }

  TextInput & stub(Span<c8 const> t)
  {
    stub_.set_text(t);
    return *this;
  }

  TextInput & stub(Span<c32 const> t)
  {
    stub_.set_text(t);
    return *this;
  }

  TextInput & stub_run(TextStyle const & style, FontStyle const & font,
                       u32 first = 0, u32 count = U32_MAX)
  {
    stub_.run(style, font, first, count);
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
        ctx.key_state(KeyCode::Left) && ctx.mouse_state(MouseButton::Primary))
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
                         f32 zoom, ViewEvents const & events,
                         Fn<void(View &)>) override
  {
    bool edited = false;
    auto erase  = [this, &edited](Slice32 style) {
      this->content_.text_.erase((Slice64) style);
      edited |= style.is_empty();
      this->content_.flush_text();
    };

    auto insert = [this, &edited](u32 pos, Span<c32 const> t) {
      CHECK(this->content_.text_.insert_span(pos, t));
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

    if (content_.layout_.lines.is_empty())
    {
      state.lines_per_page = 1;
    }
    else
    {
      state.lines_per_page =
        (u32) (region.extent.y /
               (content_.layout_.lines[0].metrics.height * zoom));
    }

    Vec<c32> text_input_utf32{default_allocator};

    utf8_decode(ctx.text, text_input_utf32).unwrap();

    compositor_.command(
      content_.text_, content_.layout_, region.extent.x, content_.alignment_,
      cmd, fn(insert), fn(erase), text_input_utf32, *engine->clipboard,
      state.lines_per_page, (ctx.mouse.position - region.center) * zoom);

    if (edited)
    {
      state.editing = true;
    }

    if (events.focus_out)
    {
      compositor_.unselect();
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

    return ViewState{
      .draggable  = !state.disabled,
      .focusable  = !state.disabled,
      .text       = TextInputInfo{.multiline = state.multiline,
                                  .tab_input = state.tab_input},
      .grab_focus = events.mouse_down
    };
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    if (content_.text_.is_empty())
    {
      stub_.layout(allocated.x);
      return {.extent = stub_.layout_.extent};
    }
    content_.layout(allocated.x);
    return {.extent = content_.layout_.extent};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32 zoom,
                      Rect const & clip) override
  {
    if (content_.text_.is_empty())
    {
      stub_.render(canvas, region, clip.centered(), zoom);
    }
    else
    {
      highlight(TextHighlight{
        .slice = compositor_.get_cursor().as_slice(content_.text_.size32()),
        .style = style.highlight});
      content_.render(canvas, region, clip.centered(), zoom);
      content_.highlights_.pop();
    }

    if (state.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
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
  } style;

  struct Callbacks
  {
    Fn<void()> pressed = noop;
    Fn<void()> hovered = noop;
  } cb;

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents const &  events, Fn<void(View &)>) override
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
    Vec2 size = allocated - 2 * style.padding(allocated);
    size.x    = max(size.x, 0.0F);
    size.y    = max(size.y, 0.0F);
    fill(sizes, size);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    fill(centers, Vec2{0, 0});
    return {.extent = (sizes.is_empty() ? Vec2{0, 0} : sizes[0]) +
                      2 * style.padding(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    ColorGradient tint = (state.press.hovered && !state.press.held) ?
                           style.hovered_color :
                           style.color;
    tint               = state.disabled ? style.disabled_color : tint;
    canvas.squircle({.center       = region.center,
                     .extent       = region.extent,
                     .corner_radii = style.corner_radii(region.extent),
                     .stroke       = style.stroke,
                     .thickness    = style.thickness,
                     .tint         = tint},
                    1, 256);

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
    }
  }
};

struct TextButton : Button
{
  TextView text_{};

  TextButton()
  {
    text_.state.copyable = false;
  }

  virtual ~TextButton() override = default;

  TextButton & disable(bool d)
  {
    state.disabled = d;
    return *this;
  }

  TextButton & run(TextStyle const & style, FontStyle const & font,
                   u32 first = 0, u32 count = U32_MAX)
  {
    text_.run(style, font, first, count);
    return *this;
  }

  TextButton & text(Span<c32 const> t)
  {
    text_.text(t);
    return *this;
  }

  TextButton & text(Span<c8 const> t)
  {
    text_.text(t);
    return *this;
  }

  TextButton & color(ColorGradient const & c)
  {
    style.color = c;
    return *this;
  }

  TextButton & hovered_color(ColorGradient const & c)
  {
    style.hovered_color = c;
    return *this;
  }

  TextButton & disabled_color(ColorGradient const & c)
  {
    style.disabled_color = c;
    return *this;
  }

  TextButton & corner_radii(CornerRadii const & c)
  {
    style.corner_radii = c;
    return *this;
  }

  TextButton & frame(Vec2 extent, bool constrain = true)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  TextButton & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  TextButton & padding(Vec2 p)
  {
    style.padding = Frame{p};
    return *this;
  }

  TextButton & padding(Frame p)
  {
    style.padding = p;
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
                         f32 zoom, ViewEvents const & events,
                         Fn<void(View &)> build) override
  {
    ViewState state = Button::tick(ctx, region, zoom, events, build);
    build(text_);
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
    Frame         frame{
              {20, 20}
    };
    FocusStyle focus = {};
  } style;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  CheckBox & disable(bool d)
  {
    state.disabled = d;
    return *this;
  }

  CheckBox & box_color(ColorGradient const & c)
  {
    style.box_color = c;
    return *this;
  }

  CheckBox & box_hovered_color(ColorGradient const & c)
  {
    style.box_hovered_color = c;
    return *this;
  }

  CheckBox & tick_color(ColorGradient const & c)
  {
    style.tick_color = c;
    return *this;
  }

  CheckBox & stroke(f32 s)
  {
    style.stroke = s;
    return *this;
  }

  CheckBox & thickness(f32 t)
  {
    style.thickness = t;
    return *this;
  }

  CheckBox & tick_thickness(f32 t)
  {
    style.tick_thickness = t;
    return *this;
  }

  CheckBox & corner_radii(CornerRadii const & r)
  {
    style.corner_radii = r;
    return *this;
  }

  CheckBox & frame(Vec2 extent, bool constrain = true)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  CheckBox & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  CheckBox & on_changed(Fn<void(bool)> f)
  {
    cb.changed = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents const &  events, Fn<void(View &)>) override
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
    Vec2 extent = style.frame(allocated);
    return {.extent = Vec2::splat(min(extent.x, extent.y))};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    ColorGradient tint =
      (state.press.hovered && !state.press.held && !state.disabled) ?
        style.box_hovered_color :
        style.box_color;
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent),
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
          .thickness = style.tick_thickness,
          .tint      = style.tick_color
      },
        span<Vec2>({{0.125F, 0.5F}, {0.374F, 0.75F}, {0.775F, 0.25F}}));
    }

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
    }
  }
};

/// @brief Multi-directional Slider
// [ ] make it easy so that we can just switch the direction without switching the axis values
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
    Axis  axis = Axis::X;
    Frame frame{
      {100, 20}
    };
    Frame thumb_frame{
      {20, 20}
    };
    Frame track_frame{
      {100, 20}
    };
    ColorGradient thumb_color          = DEFAULT_THEME.primary;
    ColorGradient thumb_hovered_color  = DEFAULT_THEME.primary;
    ColorGradient thumb_dragging_color = DEFAULT_THEME.primary;
    CornerRadii   thumb_corner_radii   = Size{.scale = 0.125F};
    ColorGradient track_color          = DEFAULT_THEME.inactive;
    CornerRadii   track_corner_radii   = Size{.scale = 0.125F};
    FocusStyle    focus                = {};
    f32           delta                = 0.1F;
  } style;

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
    style.axis = a;
    return *this;
  }

  Slider & frame(Vec2 extent, bool constrain = true)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  Slider & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  Slider & thumb_frame(Vec2 extent, bool constrain = true)
  {
    style.thumb_frame = Frame{extent, constrain};
    return *this;
  }

  Slider & thumb_frame(Frame f)
  {
    style.thumb_frame = f;
    return *this;
  }

  Slider & track_frame(Vec2 extent, bool constrain = true)
  {
    style.track_frame = Frame{extent, constrain};
    return *this;
  }

  Slider & track_frame(Frame f)
  {
    style.track_frame = f;
    return *this;
  }

  Slider & thumb_color(ColorGradient const & c)
  {
    style.thumb_color = c;
    return *this;
  }

  Slider & thumb_hovered_color(ColorGradient const & c)
  {
    style.thumb_hovered_color = c;
    return *this;
  }

  Slider & thumb_dragging_color(ColorGradient const & c)
  {
    style.thumb_dragging_color = c;
    return *this;
  }

  Slider & thumb_corner_radii(CornerRadii const & c)
  {
    style.thumb_corner_radii = c;
    return *this;
  }

  Slider & track_color(ColorGradient const & c)
  {
    style.track_color = c;
    return *this;
  }

  Slider & track_corner_radii(CornerRadii const & c)
  {
    style.track_corner_radii = c;
    return *this;
  }

  Slider & on_changed(Fn<void(f32)> f)
  {
    cb.changed = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents const & events, Fn<void(View &)>) override
  {
    u32 const main_axis = (style.axis == Axis::X) ? 0 : 1;

    state.drag.tick(events);

    if (state.drag.dragging)
    {
      Vec2 const track_extent = style.track_frame(region.extent);
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

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = style.frame(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    u32 const     main_axis   = (style.axis == Axis::X) ? 0 : 1;
    u32 const     cross_axis  = (style.axis == Axis::X) ? 1 : 0;
    Vec2 const    frame       = region.extent;
    Vec2 const    track_frame = style.track_frame(frame);
    ColorGradient thumb_color;

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

    Vec2 const track_begin = region.center - track_frame * 0.5F;

    // [ ] offset calculation is off

    Vec2 thumb_center;
    thumb_center[main_axis] =
      track_begin[main_axis] + track_frame[main_axis] * state.t;
    thumb_center[cross_axis] = region.center[cross_axis];

    Vec2 const thumb_extent = style.thumb_frame(region.extent) * dilation;

    canvas
      .rrect({.center       = region.center,
              .extent       = track_frame,
              .corner_radii = style.track_corner_radii(region.extent),
              .tint         = style.track_color})
      .rrect({.center       = thumb_center,
              .extent       = thumb_extent,
              .corner_radii = style.thumb_corner_radii(thumb_extent),
              .tint         = thumb_color});

    if (state.drag.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
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
    Frame         frame{
              {40, 20}
    };
    Frame thumb_frame{
      {17.5, 17.5}
    };
    FocusStyle focus = {};
  } style;

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

  Switch & on_color(ColorGradient const & c)
  {
    style.on_color = c;
    return *this;
  }

  Switch & on_hovered_color(ColorGradient const & c)
  {
    style.on_hovered_color = c;
    return *this;
  }

  Switch & off_color(ColorGradient const & c)
  {
    style.off_color = c;
    return *this;
  }

  Switch & off_hovered_color(ColorGradient const & c)
  {
    style.off_hovered_color = c;
    return *this;
  }

  Switch & track_color(ColorGradient const & c)
  {
    style.track_color = c;
    return *this;
  }

  Switch & corner_radii(CornerRadii const & r)
  {
    style.corner_radii = r;
    return *this;
  }

  Switch & frame(Vec2 extent, bool constrain = true)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  Switch & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  Switch & thumb_frame(Vec2 extent, bool constrain = true)
  {
    style.thumb_frame = Frame{extent, constrain};
    return *this;
  }

  Switch & thumb_frame(Frame f)
  {
    style.thumb_frame = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents const &  events, Fn<void(View &)>) override
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
    return {.extent = style.frame(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    Vec2 const thumb_extent = style.thumb_frame(region.extent);
    Vec2 const thumb_center =
      region.center + space_align(region.extent, thumb_extent,
                                  Vec2{state.value ? 1.0F : -1.0F, 0});

    ColorGradient thumb_color;
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
              .corner_radii = style.corner_radii(region.extent),
              .tint         = style.track_color})
      .rrect({.center       = thumb_center,
              .extent       = thumb_extent,
              .corner_radii = style.corner_radii(region.extent),
              .tint         = thumb_color});

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
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
    Frame frame{
      {20, 20}
    };
    CornerRadii   corner_radii        = Size{.scale = 0.125F};
    f32           thickness           = 1.0F;
    ColorGradient color               = DEFAULT_THEME.inactive;
    ColorGradient inner_color         = DEFAULT_THEME.primary;
    ColorGradient inner_hovered_color = DEFAULT_THEME.primary_variant;
    FocusStyle    focus               = {};
  } style;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  RadioBox & corner_radii(CornerRadii const & c)
  {
    style.corner_radii = c;
    return *this;
  }

  RadioBox & thickness(f32 t)
  {
    style.thickness = t;
    return *this;
  }

  RadioBox & color(ColorGradient const & c)
  {
    style.color = c;
    return *this;
  }

  RadioBox & inner_color(ColorGradient const & c)
  {
    style.inner_color = c;
    return *this;
  }

  RadioBox & inner_hovered_color(ColorGradient const & c)
  {
    style.inner_hovered_color = c;
    return *this;
  }

  RadioBox & frame(Vec2 extent, bool constrain = true)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  RadioBox & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  RadioBox & on_changed(Fn<void(bool)> f)
  {
    cb.changed = f;
    return *this;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents const &  events, Fn<void(View &)>) override
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
    return {.extent = style.frame(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent),
                  .stroke       = 1,
                  .thickness    = style.thickness,
                  .tint         = style.color});

    if (state.value)
    {
      Vec2 inner_extent = region.extent * (state.press.hovered ? 0.75F : 0.5F);
      ColorGradient inner_color =
        state.press.hovered ? style.inner_hovered_color : style.inner_color;

      canvas.circle(
        {.center = region.center, .extent = inner_extent, .tint = inner_color});
    }

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
    }
  }
};

using Scalar = Enum<f32, i32>;

namespace fmt
{

inline bool push(Context const & ctx, Spec const & spec, Scalar const & value)
{
  return value.match([&](f32 f) { return push(ctx, spec, f); },
                     [&](i32 i) { return push(ctx, spec, i); });
}

}    // namespace fmt

/// @param start starting value, this is the value to be reset to when cancel is
/// requested
/// @param min minimum value of the scalar
/// @param max maximum value of the scalar
/// @param step step in either direction that should be taken. i.e. when `+` or
/// `-` is pressed.
/// @param current current value of the scalar, mutated by the GUI system
struct F32InputSpec
{
  f32 base = 0;
  f32 min  = 0;
  f32 max  = 1;
  f32 step = 0.01;

  constexpr f32 step_value(f32 current, i32 direction) const
  {
    return clamp(current + ((direction > 0) ? step : -step), min, max);
  }

  constexpr f32 uninterp(f32 current) const
  {
    return clamp(unlerp(min, max, current), 0.0F, 1.0F);
  }

  constexpr f32 interp(f32 t) const
  {
    return clamp(lerp(min, max, t), min, max);
  }
};

/// @param start starting value, this is the value to be reset to when cancel is
/// requested
/// @param min minimum value of the scalar
/// @param max maximum value of the scalar
/// @param step step in either direction that should be taken. i.e. when `+` or
/// `-` is pressed.
/// @param current current value of the scalar, mutated by the GUI system
struct I32InputSpec
{
  i32 base = 0;
  i32 min  = 0;
  i32 max  = 1'000;
  i32 step = 100;

  constexpr i32 step_value(i32 current, i32 direction) const
  {
    return clamp(sat_add(current, (direction > 0) ? step : -step), min, max);
  }

  constexpr f32 uninterp(i32 current) const
  {
    return clamp(unlerp((f32) min, (f32) max, (f32) current), 0.0F, 1.0F);
  }

  constexpr i32 interp(f32 t) const
  {
    return clamp((i32) lerp((f32) min, (f32) max, t), min, max);
  }
};

using ScalarSpec = Enum<F32InputSpec, I32InputSpec>;

struct ScalarDragBox : View
{
  typedef Fn<void(fmt::Context const &, Scalar)>                  Fmt;
  typedef Fn<void(Span<c32 const>, ScalarSpec const &, Scalar &)> Parse;

  struct State
  {
    bool       disabled   = false;
    bool       input_mode = false;
    bool       dragging   = false;
    FocusState focus      = {};
    ScalarSpec spec       = F32InputSpec{};
    Scalar     scalar     = 0.0F;
  } state;

  // TODO: calculate minimum frame extent based on font size

  struct Style
  {
    Frame frame = Frame{}.min({DEFAULT_THEME.body_font_height * 10,
                               DEFAULT_THEME.body_font_height + 10});
    Frame padding{
      {5, 5}
    };
    Size          thumb_width  = {5};
    CornerRadii   corner_radii = Size{.scale = 0.125F};
    ColorGradient color        = DEFAULT_THEME.inactive;
    ColorGradient thumb_color  = DEFAULT_THEME.inactive;
    f32           stroke       = 1.0F;
    f32           thickness    = 1.0F;
    FocusStyle    focus        = {};
    Fmt           fmt          = fn(scalar_fmt);
    Parse         parse        = fn(scalar_parse);
  } style;

  TextInput input_{};

  struct Callbacks
  {
    Fn<void(Scalar)> update = noop;
  } cb;

  ScalarDragBox()
  {
    input_.multiline(false).tab_input(false).enter_submits(false);
  }

  virtual ~ScalarDragBox() override
  {
  }

  static void scalar_fmt(fmt::Context const & ctx, Scalar s)
  {
    fmt::format(ctx, fmt::Spec{.precision = 2}, s);
  }

  static void scalar_parse(Span<c32 const> text, ScalarSpec const & spec,
                           Scalar &);

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents const & events,
                         Fn<void(View &)>   build) override
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
      state.scalar = state.spec.match(
        [t](F32InputSpec & v) -> Scalar { return v.interp(t); },
        [t](I32InputSpec & v) -> Scalar { return v.interp(t); });
    }

    if (input_.state.editing)
    {
      style.parse(input_.content_.get_text(), state.spec, state.scalar);
    }
    else
    {
      char         scratch[128];
      c8           text[128];
      Buffer       buffer = ash::buffer(span(text).as_char());
      fmt::Context ctx    = fmt::buffer(buffer, scratch);
      style.fmt(ctx, state.scalar);
      input_.content_.set_text(span(text).slice(0, buffer.size()));
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

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 child = style.frame(allocated) - 2 * style.padding(allocated);
    child.x    = max(child.x, 0.0F);
    child.y    = max(child.y, 0.0F);
    fill(sizes, child);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    fill(centers, Vec2{0, 0});
    return {.extent = sizes[0] + 2 * style.padding(allocated)};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent),
                  .stroke       = style.stroke,
                  .thickness    = style.thickness,
                  .tint         = style.color});

    if (!state.input_mode)
    {
      f32 const t = state.spec.match(
        [this](F32InputSpec & v) { return v.uninterp(state.scalar[v0]); },
        [this](I32InputSpec & v) { return v.uninterp(state.scalar[v1]); });
      Vec2 const thumb_extent{style.thumb_width(region.extent.x),
                              region.extent.y};
      Vec2       thumb_center = region.center;
      thumb_center.x +=
        space_align(region.extent.x, thumb_extent.x, norm_to_axis(t));

      canvas.rrect({.center       = thumb_center,
                    .extent       = thumb_extent,
                    .corner_radii = Vec4::splat(region.extent.y * 0.125F),
                    .tint         = style.thumb_color});
    }

    if (state.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
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
    Fn<void(Scalar)> update = noop;
  } cb;

  TextButton    dec_{};
  TextButton    inc_{};
  ScalarDragBox drag_{};

  ScalarBox()
  {
    FlexView::axis(Axis::X)
      .wrap(false)
      .main_align(MainAlign::Start)
      .cross_align(0)
      .frame(Frame{}.scale({1, 1}));

    dec_.text(U"-"_str)
      .run(
        TextStyle{
          .shadow_scale  = 1,
          .shadow_offset = {1, 1},
          .color         = DEFAULT_THEME.on_primary,
          .shadow        = colors::BLACK
    },
        FontStyle{.font        = FontId::Default,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = 1})
      .on_pressed(fn(
        this,
        +[](ScalarBox * b) {
          auto & state = b->drag_.state;
          state.scalar = state.spec.match(
            [&state](F32InputSpec const & spec) -> Scalar {
              return spec.step_value(state.scalar[v0], -1);
            },
            [&state](I32InputSpec const & spec) -> Scalar {
              return spec.step_value(state.scalar[v1], -1);
            });
          b->cb.update(state.scalar);
        }))
      .padding({5, 5});

    inc_.text(U"+"_str)
      .run(
        TextStyle{
          .shadow_scale  = 1,
          .shadow_offset = {1, 1},
          .color         = DEFAULT_THEME.on_primary,
          .shadow        = colors::BLACK
    },
        FontStyle{.font        = FontId::Default,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = 1})
      .on_pressed(fn(
        this,
        +[](ScalarBox * b) {
          auto & state = b->drag_.state;
          state.scalar = state.spec.match(
            [&state](F32InputSpec const & spec) -> Scalar {
              return spec.step_value(state.scalar[v0], 1);
            },
            [&state](I32InputSpec const & spec) -> Scalar {
              return spec.step_value(state.scalar[v1], 1);
            });
          b->cb.update(state.scalar);
        }))
      .padding({5, 5});

    // [ ] color, stroke color, etc. the rectangles at small sizes seem to have
    // bad border radii.
    //
    // [ ] probably due to incorrect rrect rendering
    //
    // [ ] fix interpolation of slider
    //

    drag_.cb.update =
      fn(this, +[](ScalarBox * b, Scalar in) { b->cb.update(in); });

    // [ ] set drag box style: create similar methods for it
    // [ ] all views must have these methods
    // drag_.
  }

  virtual ~ScalarBox() override = default;

  // [ ] placeholder
  // [ ] spec

  ScalarBox & stroke(f32 s)
  {
    drag_.style.stroke = s;
    return *this;
  }

  ScalarBox & thickness(f32 t)
  {
    drag_.style.thickness = t;
    return *this;
  }

  ScalarBox & padding(Vec2 p)
  {
    dec_.padding(p);
    inc_.padding(p);
    drag_.style.padding = Frame{p};
    return *this;
  }

  ScalarBox & padding(Frame p)
  {
    dec_.padding(p);
    inc_.padding(p);
    drag_.style.padding = p;
    return *this;
  }

  ScalarBox & frame(Vec2 extent, bool constrain = true)
  {
    dec_.frame(extent, constrain);
    inc_.frame(extent, constrain);
    drag_.style.frame = Frame{extent, constrain};
    return *this;
  }

  ScalarBox & frame(Frame f)
  {
    dec_.frame(f);
    inc_.frame(f);
    drag_.style.frame = f;
    return *this;
  }

  ScalarBox & corner_radii(CornerRadii const & r)
  {
    dec_.corner_radii(r);
    inc_.corner_radii(r);
    drag_.style.corner_radii = r;
    return *this;
  }

  ScalarBox & on_update(Fn<void(Scalar)> f)
  {
    cb.update = f;
    return *this;
  }

  ScalarBox & text_style(TextStyle const & style, FontStyle const & font,
                         u32 first, u32 count)
  {
    dec_.run(style, font, first, count);
    inc_.run(style, font, first, count);
    drag_.input_.content_run(style, font, first, count)
      .stub_run(style, font, first, count);
    return *this;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32,
                         ViewEvents const &, Fn<void(View &)> build) override
  {
    build(dec_);
    build(drag_);
    build(inc_);
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
  } style;

  struct Callbacks
  {
    Fn<void(f32)> scrolled = noop;
  } cb;

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents const & events, Fn<void(View &)>) override
  {
    u32 const main_axis = (style.axis == Axis::X) ? 0 : 1;

    state.drag.tick(events);

    if (state.drag.dragging)
    {
      state.t = clamp(
        (ctx.mouse.position[main_axis] - region.extent[main_axis] * 0.5F) /
          region.extent[main_axis],
        0.0F, 1.0F);
      cb.scrolled(state.t);
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
                      Rect const &) override
  {
    u32 const  main_axis          = (style.axis == Axis::X) ? 0 : 1;
    u32 const  cross_axis         = (style.axis == Axis::X) ? 1 : 0;
    Vec4 const thumb_corner_radii = style.thumb_corner_radii(region.extent);
    Vec4 const track_corner_radii = style.track_corner_radii(region.extent);

    // calculate thumb main axis extent
    f32 const thumb_scale = region.extent[main_axis] / style.content_extent;
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
              .corner_radii = track_corner_radii,
              .stroke       = 0,
              .tint         = style.track_color})
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
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
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

  OptionRef<View> child_ = none;

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents const & events,
                         Fn<void(View &)>   build) override
  {
    child_.match(build);

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
    Frame frame{
      {200, 200}
    };
    Size x_bar_size = {.offset = 10};
    Size y_bar_size = {.offset = 10};
  } style;

  ScrollViewFrame view_frame_{};
  ScrollBar       x_bar_{};
  ScrollBar       y_bar_{};

  ScrollView()
  {
    x_bar_.style.axis = Axis::X;
    y_bar_.style.axis = Axis::Y;
  }

  virtual ~ScrollView() override = default;

  ScrollView & disable(bool d)
  {
    state.disabled        = d;
    x_bar_.state.disabled = d;
    y_bar_.state.disabled = d;
    return *this;
  }

  ScrollView & item(View & v)
  {
    view_frame_.child_ = v;
    return *this;
  }

  ScrollView & item(None)
  {
    view_frame_.child_ = none;
    return *this;
  }

  ScrollView & thumb_color(ColorGradient const & c)
  {
    x_bar_.style.thumb_color = c;
    y_bar_.style.thumb_color = c;
    return *this;
  }

  ScrollView & thumb_hovered_color(ColorGradient const & c)
  {
    x_bar_.style.thumb_hovered_color = c;
    y_bar_.style.thumb_hovered_color = c;
    return *this;
  }

  ScrollView & thumb_dragging_color(ColorGradient const & c)
  {
    x_bar_.style.thumb_dragging_color = c;
    y_bar_.style.thumb_dragging_color = c;
    return *this;
  }

  ScrollView & thumb_corner_radii(CornerRadii const & c)
  {
    x_bar_.style.thumb_corner_radii = c;
    y_bar_.style.thumb_corner_radii = c;
    return *this;
  }

  ScrollView & track_color(ColorGradient const & c)
  {
    x_bar_.style.track_color = c;
    y_bar_.style.track_color = c;
    return *this;
  }

  ScrollView & track_corner_radii(CornerRadii const & c)
  {
    x_bar_.style.track_corner_radii = c;
    y_bar_.style.track_corner_radii = c;
    return *this;
  }

  ScrollView & axes(Axes a)
  {
    style.axes          = a;
    x_bar_.state.hidden = has_bits(a, Axes::X);
    y_bar_.state.hidden = has_bits(a, Axes::Y);
    return *this;
  }

  ScrollView & frame(Vec2 extent, bool constrain)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  ScrollView & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  ScrollView & bar_size(f32 x, f32 y)
  {
    style.x_bar_size = Size{.offset = x};
    style.y_bar_size = Size{.offset = y};
    return *this;
  }

  ScrollView & bar_size(Size x, Size y)
  {
    style.x_bar_size = x;
    style.y_bar_size = y;
    return *this;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32,
                         ViewEvents const &, Fn<void(View &)> build) override
  {
    view_frame_.state.t = {x_bar_.state.t, y_bar_.state.t};
    build(view_frame_);
    build(x_bar_);
    build(y_bar_);
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame      = style.frame(allocated);
    f32 const  x_bar_size = style.x_bar_size(allocated.x);
    f32 const  y_bar_size = style.y_bar_size(allocated.y);

    sizes[0] = frame;
    sizes[1] = {frame.x, x_bar_size};

    if (has_bits(style.axes, Axes::X | Axes::Y))
    {
      sizes[1].x -= y_bar_size;
    }

    sizes[2] = {y_bar_size, frame.y};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2 const frame = style.frame(allocated);
    centers[0]       = {0, 0};
    centers[1]       = space_align(frame, sizes[1], ALIGNMENT_BOTTOM_LEFT);
    centers[2]       = space_align(frame, sizes[2], ALIGNMENT_TOP_RIGHT);
    x_bar_.style.content_extent = view_frame_.state.content_extent.x;
    y_bar_.style.content_extent = view_frame_.state.content_extent.y;

    return {.extent = frame};
  }
};

struct ComboBoxItem : View
{
  struct State
  {
    bool                         disabled = false;
    PressState                   press    = {};
    OptionRef<Option<u32> const> selected = none;
    u32                          index    = 0;
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

  OptionRef<Style const> style = none;

  struct Callbacks
  {
    Fn<void(Option<u32>)> selected = noop;
  } cb;

  OptionRef<View> child_ = none;

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents const &  events,
                         Fn<void(View &)>    build) override
  {
    state.press.tick(ctx, events);

    if (state.press.down && !state.selected)
    {
      cb.selected(state.index);
    }

    child_.match(build);

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
        space_align(allocated, sizes[i], Vec2{style.value().alignment, 0});
    }
    return {.extent = allocated};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    canvas.rrect({.center = region.center,
                  .extent = region.extent,
                  .tint   = state.press.hovered ?
                              style.value().hovered_background_color :
                              style.value().background_color});

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.value().focus.border_thickness,
                   .tint      = style.value().focus.border_color});
    }
  }
};

struct TextComboBoxItem : ComboBoxItem
{
  TextView text_{};

  TextComboBoxItem()
  {
    text_.state.copyable = false;
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const & region,
                         f32 zoom, ViewEvents const & events,
                         Fn<void(View &)> build) override
  {
    build(text_);

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
    Frame frame{
      {150, 450}
    };
    Size          item_height  = {25};
    CornerRadii   corner_radii = Size{.scale = 0.125F};
    f32           alignment    = 0;
    ColorGradient color        = DEFAULT_THEME.surface;
    f32           scroll_delta = 0;
  } style;

  Vec<ComboBoxItem *> items_{default_allocator};

  virtual ViewState tick(ViewContext const & ctx, CRect const & region, f32,
                         ViewEvents const & events,
                         Fn<void(View &)>   build) override
  {
    for (ComboBoxItem * item : items_)
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
    fill(sizes,
         Vec2{style.frame.x(allocated.x), style.item_height(allocated.y)});
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2 size = style.frame(allocated);

    f32 items_height = 0;
    for (Vec2 const & style : sizes)
    {
      items_height += style.y;
    }

    //[ ] centers

    size.y = min(size.y, items_height);

    Vec2 viewport_extent{size.x, items_height};

    return ViewLayout{
      .extent          = size,
      .viewport_extent = viewport_extent,
      .viewport_transform =
        scroll_transform(viewport_extent, size, Vec2{state.t, 0.5F},
          1),
      .fixed_position = Vec2{0,       0   }
    };
  }

  virtual i32 stack(i32 allocated) override
  {
    return allocated + 25;
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent),
                  .tint         = style.color});
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
    Option<u32> selected = none;
  } state;

  ComboBoxScrollView      scroll_view_{};
  OptionRef<ComboBoxItem> header_ = none;

  struct Style
  {
    CornerRadii   corner_radii  = Size{.scale = 0.125F};
    ColorGradient color         = DEFAULT_THEME.surface;
    ColorGradient hovered_color = DEFAULT_THEME.surface_variant;
    f32           alignment     = 0;
    Frame frame = Frame{}.scale({1, 0}).offset({0, 25}).max({200, F32_INF});
    FocusStyle          focus = {};
    ComboBoxItem::Style item  = {};
  } style;

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

  ComboBox & color(ColorGradient const & c)
  {
    style.color = c;
    return *this;
  }

  ComboBox & hovered_color(ColorGradient const & c)
  {
    style.hovered_color = c;
    return *this;
  }

  ComboBox & alignment(f32 a)
  {
    style.alignment = a;
    return *this;
  }

  ComboBox & frame(Vec2 extent, bool constrain)
  {
    style.frame = Frame{extent, constrain};
    return *this;
  }

  ComboBox & frame(Frame f)
  {
    style.frame = f;
    return *this;
  }

  ComboBox & align(f32 a)
  {
    style.alignment = a;
    return *this;
  }

  ComboBox & on_selected(Fn<void(Option<u32>)> style)
  {
    cb.selected = style;
    return *this;
  }

  ComboBox & add_item(ComboBoxItem & item)
  {
    u32 const index  = scroll_view_.items_.size32();
    item.state.index = index;
    item.style       = style.item;
    item.cb.selected =
      fn(this, +[](ComboBox * b, Option<u32> item) { b->set_selected(item); });
    item.state.selected = state.selected;
    scroll_view_.items_.push(&item).unwrap();
    return *this;
  }

  ComboBox & set_header(ComboBoxItem & item)
  {
    header_          = item;
    item.state.index = 0;
    item.cb.selected = fn(this, +[](ComboBox * b, Option<u32>) { b->open(); });
    item.style       = style.item;
    item.state.selected = none;
    return *this;
  }

  u32 num_items() const
  {
    return scroll_view_.items_.size32();
  }

  ComboBox & set_selected(Option<u32> item)
  {
    CHECK(!(item.is_some() && item.value_ > scroll_view_.items_.size32()));
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
    return scroll_view_.state.opened;
  }

  ComboBox & close()
  {
    scroll_view_.state.opened = false;
    return *this;
  }

  ComboBox & open()
  {
    scroll_view_.state.opened = true;
    return *this;
  }

  ComboBox & toggle()
  {
    if (scroll_view_.state.opened)
    {
      return open();
    }
    else
    {
      return close();
    }
  }

  virtual ViewState tick(ViewContext const & ctx, CRect const &, f32,
                         ViewEvents const &  events,
                         Fn<void(View &)>    build) override
  {
    state.press.tick(ctx, events);

    if (state.press.down)
    {
      toggle();
    }

    if (is_opened() && ctx.mouse_down(MouseButton::Primary) &&
        !contains(scroll_view_.View::region_, ctx.mouse.position))
    {
      close();
    }

    build(scroll_view_);

    header_.match(build);

    return ViewState{.clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, style.frame(allocated));
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> extents,
                         Span<Vec2> centers) override
  {
    Vec2 const frame = style.frame(allocated);
    for (u32 i = 0; i < extents.size32(); i++)
    {
      centers[i] = space_align(frame, extents[i], Vec2{style.alignment, 0});
    }
    return {.extent = frame};
  }

  virtual void render(Canvas & canvas, CRect const & region, f32,
                      Rect const &) override
  {
    canvas.rrect(
      {.center       = region.center,
       .extent       = region.extent,
       .corner_radii = style.corner_radii(region.extent),
       .tint = state.press.hovered ? style.hovered_color : style.color});

    if (state.press.focus.focused)
    {
      canvas.rect({.center    = region.center,
                   .extent    = region.extent,
                   .stroke    = 1,
                   .thickness = style.focus.border_thickness,
                   .tint      = style.focus.border_color});
    }
  }
};

struct TextComboBox : ComboBox
{
  TextComboBoxItem header_;

  TextComboBox()
  {
    set_header(header_);
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

}    // namespace ash
