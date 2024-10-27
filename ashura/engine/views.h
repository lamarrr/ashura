
/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/engine.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"
#include "ashura/std/unique.h"
#include <charconv>

namespace ash
{

struct FocusStyle
{
  ColorGradient border_color     = ColorGradient::all(DEFAULT_THEME.primary);
  f32           border_thickness = 1.0F;
};

struct FocusState
{
  bool in      = false;
  bool out     = false;
  bool focused = false;

  void tick(ViewEvents const &events)
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

  void tick(ViewContext const &ctx, ViewEvents const &events)
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

  void tick(ViewEvents const &events)
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
    Frame     frame       = {.width = {.scale = 1}, .height = {.scale = 1}};
  } style;

  virtual f32 align_item(u32 i, u32 n)
  {
    (void) i;
    (void) n;
    return style.cross_align;
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
    u8 const   main_axis    = (style.axis == Axis::X) ? 0 : 1;
    u8 const   cross_axis   = (style.axis == Axis::X) ? 1 : 0;
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
            space_align(cross_extent, sizes[b][cross_axis], align_item(b, n));
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
    for (Vec2 &center : centers)
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
    Frame frame     = {.width = {.scale = 1}, .height = {.scale = 1}};
  } style;

  virtual Vec2 align_item(u32 i, u32 num)
  {
    (void) i;
    (void) num;
    return style.alignment;
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

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, style.frame(allocated));
  }

  virtual ViewLayout fit(Vec2, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2      span;
    u32 const n = sizes.size32();

    for (Vec2 s : sizes)
    {
      span.x = max(span.x, s.x);
      span.y = max(span.y, s.y);
    }

    for (u32 i = 0; i < n; i++)
    {
      centers[i] = space_align(span, sizes[i], align_item(i, n));
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
    TextHighlightStyle highlight;
  } style;

  RenderText     text{};
  TextCompositor compositor{};

  TextView()
  {
    text.style(
        0, U32_MAX,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_surface)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});
  }

  virtual ~TextView() override
  {
    text.uninit();
    compositor.uninit();
  }

  void highlight(TextHighlight highlight)
  {
    text.highlight(highlight);
  }

  void clear_highlights()
  {
    text.clear_highlights();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)>) override
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

    compositor.command(text.get_text(), text.inner.layout, region.extent.x,
                       text.inner.alignment, cmd,
                       fn([](u32, Span<u32 const>) {}), fn([](Slice32) {}), {},
                       *ctx.clipboard, 1, ctx.mouse.position - region.center);

    return ViewState{.draggable = state.copyable};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    text.calculate_layout(allocated.x);
    return {.extent = text.inner.layout.extent};
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &clip) override
  {
    highlight(TextHighlight{
        .slice = compositor.get_cursor().as_slice(text.get_text().size32()),
        .style = style.highlight});
    text.render(canvas, region, clip, zoom);
    text.inner.highlights.pop();
  }

  virtual Cursor cursor(CRect const &, f32, Vec2) override
  {
    return state.copyable ? Cursor::Text : Cursor::Default;
  }
};

// [ ] scrollable
// [ ] viewport text with scrollable region, scroll direction
// [ ] text input while in view, i.e. page down
struct TextInput : View
{
  struct State
  {
    bool       disabled      = false;
    bool       editing       = false;
    bool       submit        = false;
    bool       is_multiline  = false;
    bool       enter_submits = false;
    bool       tab_input     = false;
    FocusState focus         = {};
  } state;

  struct Style
  {
    TextHighlightStyle highlight      = {};
    u32                lines_per_page = 1;
    FocusStyle         focus          = {};
  } style;

  RenderText     content{};
  RenderText     placeholder{};
  TextCompositor compositor{};

  Fn<void()> on_edit      = fn([] {});
  Fn<void()> on_submit    = fn([] {});
  Fn<void()> on_focus_in  = fn([] {});
  Fn<void()> on_focus_out = fn([] {});

  TextInput()
  {
    content.style(
        0, U32_MAX,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_surface)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});
    placeholder.style(
        0, U32_MAX,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_surface)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});
  }

  virtual ~TextInput() override
  {
    content.uninit();
    placeholder.uninit();
    compositor.uninit();
  }

  void highlight(TextHighlight const &highlight)
  {
    content.inner.highlights.push(highlight).unwrap();
  }

  void clear_highlights()
  {
    content.inner.highlights.clear();
  }

  constexpr TextCommand command(ViewContext const &ctx) const
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
    if (state.is_multiline && !state.enter_submits &&
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

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)>) override
  {
    bool edited = false;
    auto erase  = [this, &edited](Slice32 s) {
      this->content.inner.text.erase(s);
      edited |= s.is_empty();
      this->content.flush_text();
    };

    auto insert = [this, &edited](u32 pos, Span<u32 const> t) {
      CHECK(this->content.inner.text.insert_span_copy(pos, t));
      edited |= t.is_empty();
      this->content.flush_text();
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

    style.lines_per_page =
        content.inner.layout.lines.is_empty() ?
            0 :
            (u32) (region.extent.y /
                   content.inner.layout.lines[0].metrics.height);

    compositor.command(span(content.inner.text), content.inner.layout,
                       region.extent.x, content.inner.alignment, cmd,
                       fn(&insert), fn(&erase), ctx.text_input_utf32,
                       *ctx.clipboard, style.lines_per_page,
                       ctx.mouse.position - region.center);

    if (edited)
    {
      state.editing = true;
    }

    if (events.focus_out)
    {
      compositor.unselect();
    }

    if (events.key_down && ctx.key_state(KeyCode::Return) &&
        state.enter_submits)
    {
      state.submit = true;
    }

    if (state.focus.in)
    {
      on_focus_in();
    }

    if (state.focus.out)
    {
      on_focus_out();
    }

    if (state.submit)
    {
      on_submit();
    }

    if (edited)
    {
      on_edit();
    }

    return ViewState{.draggable  = !state.disabled,
                     .focusable  = !state.disabled,
                     .text_input = !state.disabled,
                     .tab_input  = state.tab_input,
                     .grab_focus = events.mouse_down};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    if (content.inner.text.is_empty())
    {
      placeholder.calculate_layout(allocated.x);
      return {.extent = placeholder.inner.layout.extent};
    }
    content.calculate_layout(allocated.x);
    return {.extent = content.inner.layout.extent};
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &clip) override
  {
    if (content.inner.text.is_empty())
    {
      placeholder.render(canvas, region, clip, zoom);
    }
    else
    {
      highlight(TextHighlight{.slice = compositor.get_cursor().as_slice(
                                  content.inner.text.size32()),
                              .style = style.highlight});
      content.render(canvas, region, clip, zoom);
      content.inner.highlights.pop();
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
    ColorGradient color = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
    ColorGradient disabled_color = ColorGradient::all(DEFAULT_THEME.inactive);
    CornerRadii   corner_radii   = CornerRadii::all({.scale = 0.125F});
    f32           stroke         = 0.0F;
    f32           thickness      = 1.0F;
    Frame         frame          = {};
    Frame         padding        = {};
    FocusStyle    focus          = {};
  } style;

  Fn<void()> on_pressed = fn([] {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    state.press.tick(ctx, events);

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

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &) override
  {
    ColorGradient tint = (state.press.hovered && !state.press.held) ?
                             style.hovered_color :
                             style.color;
    tint               = state.disabled ? style.disabled_color : tint;
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent.y),
                  .stroke       = style.stroke,
                  .thickness    = style.thickness,
                  .tint         = tint});

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
  TextView text{};

  TextButton()
  {
    text.state.copyable = false;
  }

  virtual ~TextButton() override = default;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    Button::tick(ctx, region, events, build);
    build(text);
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
    ColorGradient box_color = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient box_hovered_color = ColorGradient::all(DEFAULT_THEME.active);
    ColorGradient tick_color        = ColorGradient::all(DEFAULT_THEME.primary);
    f32           stroke            = 1;
    f32           thickness         = 1;
    f32           tick_thickness    = 1.5F;
    CornerRadii   corner_radii      = CornerRadii::all({.scale = 0.125F});
    Frame         frame             = Frame{.width = {20}, .height = {20}};
    FocusStyle    focus             = {};
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    state.press.tick(ctx, events);

    if (state.press.down)
    {
      state.value = !state.value;
      on_changed(state.value);
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

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &) override
  {
    ColorGradient tint =
        (state.press.hovered && !state.press.held && !state.disabled) ?
            style.box_hovered_color :
            style.box_color;
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent.y),
                  .stroke       = 1,
                  .thickness    = 2,
                  .tint         = tint});

    if (state.value)
    {
      canvas.line(
          {.center    = region.center,
           .extent    = region.extent,
           .stroke    = 0,
           .thickness = style.tick_thickness,
           .tint      = style.tick_color},
          span<Vec2>({{0.125f, 0.5f}, {0.374f, 0.75f}, {0.775f, 0.25f}}));
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
    Axis          axis        = Axis::X;
    Frame         frame       = {.width = {100}, .height = {30}};
    Frame         thumb_frame = {.width = {20}, .height = {20}};
    Frame         track_frame = {.width = {97.5F}, .height = {27.5F}};
    ColorGradient thumb_color =
        ColorGradient::all(DEFAULT_THEME.primary * opacity(0.5F));
    ColorGradient thumb_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary * opacity(0.75F));
    ColorGradient thumb_dragging_color =
        ColorGradient::all(DEFAULT_THEME.primary);
    CornerRadii   thumb_corner_radii = CornerRadii::all({.scale = 0.125F});
    ColorGradient track_color = ColorGradient::all(DEFAULT_THEME.inactive);
    CornerRadii   track_corner_radii = CornerRadii::all({.scale = 0.125F});
    FocusStyle    focus              = {};
    f32           delta              = 0.1F;
  } style;

  Fn<void(f32)> on_changed = fn([](f32) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)>) override
  {
    u8 const main_axis = (style.axis == Axis::X) ? 0 : 1;

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
      on_changed(value);
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

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &) override
  {
    u8 const   main_axis          = (style.axis == Axis::X) ? 0 : 1;
    u8 const   cross_axis         = (style.axis == Axis::X) ? 1 : 0;
    Vec2 const frame              = region.extent;
    Vec2 const track_frame        = style.track_frame(frame);
    Vec4 const thumb_corner_radii = style.thumb_corner_radii(region.extent.y);
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

    canvas.rrect({.center       = region.center,
                  .extent       = track_frame,
                  .corner_radii = style.track_corner_radii(region.extent.y),
                  .tint         = style.track_color});

    Vec2 const track_begin = region.center - track_frame * 0.5F;
    Vec2 const track_end   = region.center + track_frame * 0.5F;

    Vec2 thumb_center;
    thumb_center[main_axis] =
        track_begin[main_axis] + track_frame[main_axis] * state.t;
    thumb_center[cross_axis] = region.center[cross_axis];

    Vec2 const thumb_extent = style.thumb_frame(region.extent) * dilation;

    canvas.rrect({.center       = thumb_center,
                  .extent       = thumb_extent,
                  .corner_radii = style.thumb_corner_radii(thumb_extent.y),
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
    ColorGradient on_color = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient on_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
    ColorGradient off_color = ColorGradient::all(DEFAULT_THEME.active);
    ColorGradient off_hovered_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient track_color  = ColorGradient::all(DEFAULT_THEME.inactive);
    Frame         frame        = {.width = {40}, .height = {20}};
    Frame         thumb_frame  = {.width = {17.5}, .height = {17.5}};
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    FocusStyle    focus        = {};
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    state.press.tick(ctx, events);

    if (state.press.down)
    {
      state.value = !state.value;
      on_changed(state.value);
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = style.frame(allocated)};
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent.y),
                  .tint         = style.track_color});

    Vec2 const thumb_extent = style.thumb_frame(region.extent);
    Vec2 const thumb_center =
        region.center + space_align(region.extent, thumb_extent,
                                    Vec2{state.value ? -1.0F : 1.0F, 0});

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

    canvas.rrect({.center       = thumb_center,
                  .extent       = thumb_extent,
                  .corner_radii = style.corner_radii(region.extent.y),
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
    Frame         frame        = {.width = {50}, .height = {50}};
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    f32           thickness    = 1.0F;
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient inner_color  = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient inner_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
    FocusStyle focus = {};
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    state.press.tick(ctx, events);

    if (state.press.down)
    {
      state.value = !state.value;
      on_changed(state.value);
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = style.frame(allocated)};
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent.y),
                  .stroke       = 1,
                  .thickness    = style.thickness,
                  .tint         = style.color});

    if (state.value)
    {
      Vec2 inner_extent = region.extent * (state.press.hovered ? 0.75F : 0.5F);
      ColorGradient inner_color =
          state.press.hovered ? style.inner_hovered_color : style.inner_color;

      canvas.circle({.center = region.center,
                     .extent = inner_extent,
                     .tint   = inner_color});
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

struct ScalarDragBox : View
{
  typedef Fn<void(fmt::Context const &, ScalarInput)> Fmt;
  typedef Fn<void(Span<u32 const>, ScalarState &)>    Parse;

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
    Frame         frame        = {.width = {.min = 100},
                                  .height = {.min = DEFAULT_THEME.body_font_height + 10}};
    Frame         padding      = {.width = {5}, .height = {5}};
    Size          thumb_width  = {2.75F};
    CornerRadii   corner_radii = CornerRadii({.scale = 0.125F});
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient thumb_color  = ColorGradient::all(DEFAULT_THEME.inactive);
    f32           stroke       = 1.0F;
    f32           thickness    = 1.0F;
    FocusStyle    focus        = {};
  } style;

  TextInput input{};
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
    defer   utf8_{[&] { utf8.uninit(); }};
    utf8_encode(text, utf8).unwrap();

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
                         ViewEvents events, Fn<void(View &)>) override
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

    if (input.state.editing)
    {
      parse(input.content.get_text(), state.value);
    }
    else
    {
      char         scratch[128];
      char         text[128];
      Buffer       buffer = ash::buffer(span(text));
      fmt::Context ctx    = fmt::buffer(&buffer, span(scratch));
      fmt(ctx, state.value.current);
      input.content.set_text(span(buffer).as_u8());
    }

    input.state.disabled = !state.input_mode;

    if (input.state.editing || state.dragging)
    {
      on_update(state.value.current);
    }

    state.focus.tick(events);

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

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &) override
  {
    canvas.rrect({.center       = region.center,
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

  virtual Cursor cursor(CRect const &region, f32, Vec2 offset) override
  {
    (void) region;
    (void) offset;
    return state.disabled ? Cursor::Default : Cursor::EWResize;
  }
};

struct ScalarBox : FlexView
{
  Fn<void(ScalarInput)> on_update = fn([](ScalarInput) {});

  TextButton    dec{};
  TextButton    inc{};
  ScalarDragBox drag{};

  ScalarBox()
  {
    style.axis        = Axis::X;
    style.wrap        = false;
    style.main_align  = MainAlign::Start;
    style.cross_align = 0;
    style.frame       = Frame{.width = {.scale = 1}, .height = {.scale = 1}};

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

    inc.style.stroke       = drag.style.stroke;
    inc.style.thickness    = drag.style.thickness;
    inc.style.padding      = drag.style.padding;
    inc.style.frame.width  = drag.style.frame.height;
    inc.style.frame.height = drag.style.frame.height;
    inc.style.corner_radii = drag.style.corner_radii;
    dec.style.stroke       = drag.style.stroke;
    dec.style.thickness    = drag.style.thickness;
    dec.style.padding      = drag.style.padding;
    dec.style.frame.width  = drag.style.frame.height;
    dec.style.frame.height = drag.style.frame.height;
    dec.style.corner_radii = drag.style.corner_radii;

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
                         Fn<void(View &)> build) override
  {
    build(dec);
    build(drag);
    build(inc);
    return ViewState{};
  }
};

struct ScrollBar : View
{
  struct State
  {
    bool      disabled = false;
    DragState drag     = {};
    f32       t        = 0;
  } state;

  struct Style
  {
    Axis          axis           = Axis::X;
    Vec2          frame          = {};
    Vec2          content_extent = {};
    ColorGradient thumb_color =
        ColorGradient::all(DEFAULT_THEME.primary * opacity(0.5F));
    ColorGradient thumb_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary * opacity(0.75F));
    ColorGradient thumb_dragging_color =
        ColorGradient::all(DEFAULT_THEME.primary);
    CornerRadii   thumb_corner_radii = CornerRadii::all({{0}});
    ColorGradient track_color =
        ColorGradient::all(DEFAULT_THEME.inactive * opacity(0.75F));
    CornerRadii track_corner_radii = CornerRadii::all({{0}});
    FocusStyle  focus              = {};
    f32         delta              = 0.1F;
  } style;

  Fn<void(f32)> on_scrolled = fn([](f32) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)>) override
  {
    u8 const main_axis = (style.axis == Axis::X) ? 0 : 1;

    state.drag.tick(events);

    if (state.drag.dragging)
    {
      state.t =
          clamp((ctx.mouse.position[main_axis] - region.extent[main_axis] / 2) /
                    region.extent[main_axis],
                0.0f, 1.0f);
      on_scrolled(state.t);
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
    return {.extent = allocated};
  }

  virtual i32 stack(i32 allocated) override
  {
    // needs to be at a different stacking context since this will be placed
    // on top of the viewport
    return allocated + 1;
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &) override
  {
    u8 const   main_axis          = (style.axis == Axis::X) ? 0 : 1;
    u8 const   cross_axis         = (style.axis == Axis::X) ? 1 : 0;
    Vec4 const thumb_corner_radii = style.thumb_corner_radii(region.extent.y);
    Vec4 const track_corner_radii = style.track_corner_radii(region.extent.y);

    // calculate thumb main axis extent
    f32 const thumb_scale =
        style.frame[main_axis] / style.content_extent[main_axis];
    Vec2 thumb_extent;
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

    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = track_corner_radii,
                  .stroke       = 0,
                  .tint         = style.track_color});

    canvas.rrect({.center       = thumb_center,
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

struct ScrollViewViewport : View
{
};

// [ ] re-write this, since this is a viewport, it would contain and clip the
// children
// the scroll bars need to be outside the scroll view
struct ScrollView : View
{
  struct State
  {
    bool disabled = false;
  } state;

  struct Style
  {
    Axes  axes       = Axes::X | Axes::Y;
    Frame frame      = {.width = {200}, .height = {200}};
    Size  x_bar_size = {.offset = 10};
    Size  y_bar_size = {.offset = 10};
  } style;

  ScrollBar x_bar{};
  ScrollBar y_bar{};

  ScrollView()
  {
    x_bar.style.axis = Axis::X;
    y_bar.style.axis = Axis::Y;
  }

  virtual ViewState tick(ViewContext const &, CRect const &, ViewEvents,
                         Fn<void(View &)> build) override
  {
    x_bar.state.disabled = y_bar.state.disabled = state.disabled;
    build(x_bar);
    build(y_bar);
    return ViewState{.viewport = true};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame      = style.frame(allocated);
    f32 const  x_bar_size = style.x_bar_size(allocated.x);
    f32 const  y_bar_size = style.y_bar_size(allocated.y);

    sizes[0] = {frame.x, x_bar_size};

    if (has_bits(style.axes, Axes::Y))
    {
      sizes[0].x -= y_bar_size;
    }

    sizes[1] = {y_bar_size, frame.y};

    fill(sizes.slice(2), frame);
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2 const frame = style.frame(allocated);
    centers[0]       = space_align(frame, sizes[0], Vec2{1, 0});
    centers[1]       = space_align(frame, sizes[1], Vec2{1, 1});

    Vec2 content_size;
    for (Vec2 const &s : sizes.slice(2))
    {
      content_size.x = max(content_size.x, s.x);
      content_size.y = max(content_size.y, s.y);
    }

    Vec2 translation =
        (content_size - frame) * Vec2{x_bar.state.t, y_bar.state.t};
    translation = -0.5F * content_size + translation;
    fill(centers.slice(2), Vec2::splat(0));

    x_bar.style.content_extent = content_size;
    y_bar.style.content_extent = content_size;
    x_bar.style.frame          = frame;
    y_bar.style.frame          = frame;

    // [ ] on scroll update. update t of views?

    return {.extent             = frame,
            .viewport           = content_size,
            .viewport_transform = zoom_to(translation, 1)};
  }
};

// [ ] scrolling, selection, clipping
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
    ColorGradient hovered_text_color = ColorGradient::all(DEFAULT_THEME.active);
    ColorGradient hovered_background_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient disabled_text_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient disabled_background_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient selected_background_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient selected_text_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient text_color = ColorGradient::all(DEFAULT_THEME.on_surface);
    ColorGradient background_color = ColorGradient::all(DEFAULT_THEME.surface);
    CornerRadii   corner_radii     = CornerRadii::all({.scale = 0.125F});
    FocusStyle    focus            = {};
    Frame         frame            = {};
    f32           alignment        = 0;
  };

  Option<Style const *> style = None;

  Fn<void(Option<u32>)> on_selected = fn([](Option<u32>) {});

  RenderText text{};

  Option<View *> left  = None;
  Option<View *> right = None;

  void update_text(Span<u32 const> text);

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    state.press.tick(ctx, events);

    if (state.press.down && !state.selected)
    {
      on_selected(Some{state.index});
    }

    if (left)
    {
      build(*left.value());
    }

    if (right)
    {
      build(*right.value());
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    // allocate size to children
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    // [ ] assign positions to left and right
    return {};
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &clip) override
  {
    canvas.rrect({.center = region.center,
                  .extent = region.extent,
                  .tint   = state.press.hovered ?
                                style.value()->hovered_background_color :
                                style.value()->background_color});

    // [ ] render selected color
    text.render(canvas, region, clip, zoom);

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

/// [ ] z-index on expanded?
/// [ ] z-index effects on viewport
/// [ ] clip
struct ComboBoxScrollView : View
{
  struct State
  {
    bool        disabled = false;
    bool        opened   = false;
    Option<u32> selected = None;
  } state;

  struct Style
  {
    Frame               frame        = {.width = {150}, .height = {450}};
    Size                item_height  = {25};
    CornerRadii         corner_radii = CornerRadii::all({.scale = 0.125F});
    f32                 alignment    = 0;
    ColorGradient       color = ColorGradient::all(DEFAULT_THEME.surface);
    ComboBoxItem::Style item  = {};
  } style;

  struct Inner
  {
    Vec<Unique<ComboBoxItem *>> items = {};
  } inner;

  Fn<void(Option<u32>)> on_selected = fn([](Option<u32>) {});

  virtual ~ComboBoxScrollView() override
  {
    for (auto &item : inner.items)
    {
      item.uninit();
    }
    inner.items.uninit();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    // [ ] scrolling
    // on scroll increase offset of views
    for (auto &item : inner.items)
    {
      build(*item);
    }

    return ViewState{.scrollable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes,
         Vec2{style.frame.width(allocated.x), style.item_height(allocated.y)});
  }

  virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> centers) override
  {
    Vec2 size = style.frame(allocated);

    f32 items_height = 0;
    for (Vec2 const &s : sizes)
    {
      items_height += s.y;
    }

    size.y = min(size.y, items_height);

    // [ ] scrolling
    // [ ] absolute positioning to center of screen???

    return ViewLayout{.extent             = size,
                      .viewport           = {size.x, items_height},
                      .absolute_transform = {},
                      .viewport_transform = {}};
  }

  virtual i32 stack(i32 allocated)
  {
    return allocated + 255;
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &clip) override
  {
    canvas.rrect({.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent.y),
                  .tint         = style.color});
  }

  u64 add_item(Span<u32 const> text, Option<View *> left = None,
               Option<View *> right = None, bool disable = false)
  {
    Unique<ComboBoxItem *> item =
        unique_inplace<ComboBoxItem>(default_allocator).unwrap();
    item->state.disabled = disable;
    item->style          = Some<ComboBoxItem::Style const *>{&style.item};
    item->on_selected = fn(this, [](ComboBoxScrollView *v, Option<u32> item) {
      v->set_selected(item);
    });
    item->state.index = inner.items.size32();
    item->state.selected = Some<Option<u32> const *>{&state.selected};
    item->left           = left;
    item->right          = right;
    item->update_text(text);
    inner.items.push(item).unwrap();
  }

  u32 num_items() const
  {
    inner.items.size32();
  }

  void set_selected(Option<u32> item)
  {
    state.selected = item;
    on_selected(item);
  }

  Option<u32> get_selected()
  {
    return state.selected;
  }

  bool is_opened() const
  {
    return state.opened;
  }

  void close();

  void open();
};

struct ComboBox : View
{
  struct State
  {
    bool       disabled = false;
    PressState press    = {};
  } state;

  ComboBoxScrollView scroll_view;

  struct Style
  {
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);
    ColorGradient hovered_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    Frame frame =
        Frame{.width = {.scale = 1, .max = 200}, .height = {.offset = 25}};
    FocusStyle focus = {};
  } style;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    state.press.tick(ctx, events);

    if (state.press.down)
    {
      if (scroll_view.is_opened())
      {
        scroll_view.close();
      }
      else
      {
        scroll_view.open();
      }
    }

    // TODO:lamarrr: on click outside, lose focus?

    build(scroll_view);

    return ViewState{.clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void render(Canvas &canvas, CRect const &region, f32 zoom,
                      CRect const &clip) override
  {
    canvas.rrect(
        {.center       = region.center,
         .extent       = region.extent,
         .corner_radii = style.corner_radii(region.extent.y),
         .tint = state.press.hovered ? style.hovered_color : style.color});
    //[ ] draw selection text
    //[ ] draw button: scroll_view.opened?

    scroll_view.state.selected;
    scroll_view.inner.items[0];

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