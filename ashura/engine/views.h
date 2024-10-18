
/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/engine.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"
#include <charconv>

namespace ash
{

/// @param axis flex axis to layout children along
/// @param main_align main-axis alignment. specifies how free space is used on
/// the main axis
/// @param cross_align cross-axis alignment. affects how free space is used on
/// the cross axis
struct FlexView : View
{
  struct Style
  {
    Axis      axis : 3       = Axis::X;
    bool      wrap : 1       = true;
    bool      reverse : 1    = false;
    MainAlign main_align : 3 = MainAlign::Start;
    f32       cross_align    = 0;
    Frame     frame          = {.width = {.scale = 1}, .height = {.scale = 1}};
  } style;

  virtual f32 align_item(u32 i)
  {
    (void) i;
    return style.cross_align;
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame = style.frame(allocated);
    fill(sizes, frame);
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> offsets) override
  {
    u32 const  num_children = sizes.size32();
    Vec2 const frame        = style.frame(allocated);
    Vec2       span;
    f32        cross_cursor = 0;
    u8 const   main_axis    = (style.axis == Axis::X) ? 0 : 1;
    u8 const   cross_axis   = (style.axis == Axis::X) ? 1 : 0;

    for (u32 i = 0; i < num_children;)
    {
      u32 first        = i++;
      f32 main_extent  = sizes[first][main_axis];
      f32 cross_extent = sizes[first][cross_axis];
      f32 main_spacing = 0;

      while (i < num_children &&
             !(style.wrap &&
               (main_extent + sizes[i][main_axis]) > frame[main_axis]))
      {
        main_extent += sizes[i][main_axis];
        cross_extent = max(cross_extent, sizes[i][cross_axis]);
        i++;
      }

      u32 const count = i - first;

      if (style.main_align != MainAlign::Start)
      {
        main_spacing = max(frame[main_axis] - main_extent, 0.0f);
      }

      for (u32 b = first; b < first + count; b++)
      {
        offsets[b][cross_axis] =
            cross_cursor +
            space_align(frame[cross_axis], sizes[b][cross_axis], align_item(b));
      }

      switch (style.main_align)
      {
        case MainAlign::Start:
        {
          f32 main_spacing_cursor = 0;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis];
          }
        }
        break;

        case MainAlign::SpaceAround:
        {
          f32 spacing             = main_spacing / (count * 2);
          f32 main_spacing_cursor = 0;
          for (u32 b = first; b < first + count; b++)
          {
            main_spacing_cursor += spacing;
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis] + spacing;
          }
        }
        break;

        case MainAlign::SpaceBetween:
        {
          f32 spacing             = main_spacing / (count - 1);
          f32 main_spacing_cursor = 0;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis] + spacing;
          }
        }
        break;

        case MainAlign::SpaceEvenly:
        {
          f32 spacing             = main_spacing / (count + 1);
          f32 main_spacing_cursor = spacing;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis] + spacing;
          }
        }
        break;

        case MainAlign::End:
        {
          f32 main_spacing_cursor = main_spacing;
          for (u32 b = first; b < first + count; b++)
          {
            offsets[b][main_axis] = main_spacing_cursor;
            main_spacing_cursor += sizes[b][main_axis];
          }
        }
        break;

        default:
          break;
      }

      if (style.reverse)
      {
        for (u32 b0 = first, b1 = first + count - 1; b0 < b1; b0++, b1--)
        {
          swap(offsets[b0][main_axis], offsets[b1][main_axis]);
        }
      }

      cross_cursor += cross_extent;

      span[main_axis]  = max(span[main_axis], main_extent + main_spacing);
      span[cross_axis] = cross_cursor;
    }

    return {.extent = span};
  }
};

struct StackView : View
{
  struct Style
  {
    bool  reverse : 1 = false;
    Vec2  alignment   = {0, 0};
    Frame frame       = {.width = {.scale = 1}, .height = {.scale = 1}};
  } style;

  virtual Vec2 align_item(u32 i)
  {
    (void) i;
    return style.alignment;
  }

  virtual i32 stack_item(i32 base, u32 i, u32 num)
  {
    i32 z = base + 1;
    if (!style.reverse)
    {
      z += (i32) i;
    }
    else
    {
      z += (i32) (num - (i + 1));
    }
    return z;
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, style.frame(allocated));
  }

  virtual ViewExtent fit(Vec2, Span<Vec2 const> sizes,
                         Span<Vec2> offsets) override
  {
    Vec2      span;
    u32 const num_children = sizes.size32();

    for (Vec2 s : sizes)
    {
      span.x = max(span.x, s.x);
      span.y = max(span.y, s.y);
    }

    for (u32 i = 0; i < num_children; i++)
    {
      offsets[i] = space_align(span, sizes[i], align_item(i));
    }

    return {.extent = span};
  }

  virtual i32 z_index(i32 allocated, Span<i32> indices) override
  {
    for (u32 i = 0; i < indices.size32(); i++)
    {
      indices[i] = stack_item(allocated, i, indices.size32());
    }
    return allocated;
  }
};

// [ ] pre-determined height, scrolling of each item. bidirectional
struct ListView : View
{
};

struct TextView : View
{
  bool           copyable : 1           = false;
  ColorGradient  highlight_color        = {};
  CornerRadii    highlight_corner_radii = {};
  RenderText     text                   = {};
  TextCompositor compositor             = {};

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
    text.reset();
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
                       *ctx.clipboard, 1, ctx.mouse.position);
    text.set_highlight(
        compositor.get_cursor().as_slice(text.get_text().size32()));
    text.set_highlight_style(highlight_color,
                             highlight_corner_radii(region.extent.y));

    return ViewState{.draggable = copyable};
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    text.calculate_layout(allocated.x);
    return {.extent = text.inner.layout.extent};
  }

  virtual void render(CRect const &region, f32, CRect const &clip,
                      Canvas &canvas) override
  {
    text.render(region, clip, canvas);
  }

  virtual Cursor cursor(CRect const &, f32, Vec2) override
  {
    return copyable ? Cursor::Text : Cursor::Default;
  }
};

// [ ] ClickDetector with timing and debouncing
// [ ] input box focus request on click (i.e. grab focus)
struct TextInput : View
{
  struct State
  {
    bool disabled : 1      = false;
    bool editing : 1       = false;
    bool submit : 1        = false;
    bool focus_in : 1      = false;
    bool focus_out : 1     = false;
    bool focused : 1       = false;
    bool is_multiline : 1  = false;
    bool enter_submits : 1 = false;
    bool tab_input : 1     = false;
  } state;

  struct Style
  {
    ColorGradient highlight_color        = {};
    CornerRadii   highlight_corner_radii = {};
    u32           lines_per_page         = 1;
  } style;

  RenderText     content     = {};
  RenderText     placeholder = {};
  TextCompositor compositor  = {};

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
    content.reset();
    placeholder.reset();
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

    state.editing   = false;
    state.submit    = false;
    state.focus_in  = events.focus_in;
    state.focus_out = events.focus_out;

    if (events.focus_in)
    {
      state.focused = true;
    }

    if (events.focus_out)
    {
      state.focused = false;
    }

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
    else if (state.focused)
    {
      cmd = command(ctx);
    }

    compositor.command(span(content.inner.text), content.inner.layout,
                       region.extent.x, content.inner.alignment, cmd,
                       fn(&insert), fn(&erase), ctx.text_input_utf32,
                       *ctx.clipboard, style.lines_per_page,
                       ctx.mouse.position - region.begin());

    content.set_highlight(
        compositor.get_cursor().as_slice(content.inner.text.size32()));
    content.set_highlight_style(style.highlight_color,
                                style.highlight_corner_radii(region.extent.y));

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

    if (state.focus_in)
    {
      on_focus_in();
    }

    if (state.focus_out)
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
                     .tab_input  = state.tab_input};
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    placeholder.calculate_layout(allocated.x);
    content.calculate_layout(allocated.x);
    if (content.inner.text.is_empty())
    {
      return {.extent = placeholder.inner.layout.extent};
    }
    return {.extent = content.inner.layout.extent};
  }

  virtual void render(CRect const &region, f32 scale, CRect const &clip,
                      Canvas &canvas) override
  {
    if (content.inner.text.is_empty())
    {
      placeholder.render(region, clip, canvas);
    }
    else
    {
      content.render(region, clip, canvas);
    }
  }

  virtual Cursor cursor(CRect const &, f32, Vec2) override
  {
    return Cursor::Text;
  }
};

// [ ] calculate lines per page
// [ ] viewport text with scrollable region, scroll direction
// [ ] text input while in view, i.e. page down
struct ScrollableTextInput : View
{
  TextInput input;
  ScrollableTextInput()          = default;
  virtual ~ScrollableTextInput() = default;
};

struct Button : View
{
  struct State
  {
    bool disabled : 1     = false;
    bool pointer_down : 1 = false;
    bool pointer_up : 1   = false;
    bool pointer_in : 1   = false;
    bool pointer_out : 1  = false;
    bool focus_in : 1     = false;
    bool focus_out : 1    = false;
    bool focused : 1      = false;
    bool hovered : 1      = false;
    bool pressed : 1      = false;
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
  } style;

  Fn<void()> on_pressed = fn([] {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    state.pointer_down =
        events.mouse_down && ctx.mouse_down(MouseButtons::Primary);
    state.pointer_up  = events.mouse_up && ctx.mouse_up(MouseButtons::Primary);
    state.pointer_in  = events.mouse_in;
    state.pointer_out = events.mouse_out;
    state.focus_in    = events.focus_in;
    state.focus_out   = events.focus_out;

    if (events.focus_in)
    {
      state.focused = true;
    }

    if (events.focus_out)
    {
      state.focused = false;
    }

    if (state.pointer_in)
    {
      state.hovered = true;
    }

    if (state.pointer_out)
    {
      state.hovered = false;
    }

    if (state.pointer_down ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      on_pressed();
      state.pressed = true;
    }

    if (state.pointer_up || (events.key_up && ctx.key_up(KeyCode::Return)))
    {
      state.pressed = false;
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

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> offsets) override
  {
    fill(offsets, Vec2{0, 0});
    return {.extent = (sizes.is_empty() ? Vec2{0, 0} : sizes[0]) +
                      2 * style.padding(allocated)};
  }

  virtual void render(CRect const &region, f32 scale, CRect const &,
                      Canvas &canvas) override
  {
    ColorGradient tint =
        (state.hovered && !state.pressed) ? style.hovered_color : style.color;
    tint = state.disabled ? style.disabled_color : tint;
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .stroke       = style.stroke,
                           .thickness    = style.thickness,
                           .tint         = tint});
    if (state.focused)
    {
      // [ ] draw focus interaction for all widgets
    }
  }
};

struct TextButton : Button
{
  TextView text{};

  TextButton()
  {
    text.copyable = false;
  }

  virtual ~TextButton() override = default;
};

struct CheckBox : View
{
  struct State
  {
    bool disabled : 1 = false;
    bool hovered : 1  = false;
    bool pressed : 1  = false;
    bool value : 1    = false;
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
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if ((events.mouse_down && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      state.pressed = true;
      state.value   = !state.value;
      on_changed(state.value);
    }

    if ((events.mouse_up && ctx.mouse_up(MouseButtons::Primary)) ||
        (events.key_up && ctx.key_up(KeyCode::Return)))
    {
      state.pressed = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    Vec2 extent = style.frame(allocated);
    return {.extent = Vec2::splat(min(extent.x, extent.y))};
  }

  virtual void render(CRect const &region, f32 scale, CRect const &,
                      Canvas &canvas) override
  {
    ColorGradient tint = (state.hovered && !state.pressed && !state.disabled) ?
                             style.box_hovered_color :
                             style.box_color;
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .stroke       = 1,
                           .thickness    = 2,
                           .tint         = tint});

    if (state.value)
    {
      canvas.line(
          ShapeDesc{.center    = region.center,
                    .extent    = region.extent,
                    .stroke    = 0,
                    .thickness = style.tick_thickness,
                    .tint      = style.tick_color},
          span<Vec2>({{0.125f, 0.5f}, {0.374f, 0.75f}, {0.775f, 0.25f}}));
    }
  }
};

/// @brief Multi-directional Slider
struct Slider : View
{
  struct State
  {
    bool disabled : 1      = false;
    bool pointer_enter : 1 = false;
    bool pointer_leave : 1 = false;
    bool pointer_down : 1  = false;
    bool pointer_up : 1    = false;
    bool hovered : 1       = false;
    bool pressed : 1       = false;
  } state;

  struct Style
  {
    Axis          direction    = Axis::X;
    Frame         frame        = {.width = {100}, .height = {30}};
    Frame         thumb_frame  = {.width = {20}, .height = {20}};
    Size          track_height = {10};
    ColorGradient track_color  = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient thumb_color  = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient thumb_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
    CornerRadii track_corner_radii = CornerRadii::all({.scale = 0.125F});
    CornerRadii thumb_corner_radii = CornerRadii::all({.scale = 0.125F});
    u8          levels             = 0;
  } style;

  f32 t    = 0;
  f32 low  = 0;
  f32 high = 1;

  Fn<void(f32)> on_changed = fn([](f32) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)>) override
  {
    u8 const main_axis = (style.direction == Axis::X) ? 0 : 1;
    if (events.dragging)
    {
      t = unlerp(region.begin()[main_axis], region.end()[main_axis],
                 ctx.mouse.position[main_axis]);
      t = (style.levels == 0) ? t : grid_snap(t, 1.0F / style.levels);
      f32 const value = clamp(lerp(low, high, t), low, high);
      on_changed(value);
    }
    return ViewState{.pointable = !state.disabled,
                     .draggable = !state.disabled};
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = style.frame(allocated)};
  }

  virtual void render(CRect const &region, f32 scale, CRect const &,
                      Canvas &canvas) override
  {
    Vec2 const track_extent =
        Frame{style.frame.width, style.track_height}(region.extent);
    canvas.rrect(
        ShapeDesc{.center       = region.center,
                  .extent       = track_extent,
                  .corner_radii = style.track_corner_radii(region.extent.y),
                  .tint         = style.track_color});

    if (style.levels != 0)
    {
      f32 const unit = 1 / (f32) style.levels;
      for (i32 i = 1; i < style.levels; i++)
      {
        Vec2 center = region.begin();
        center.x += unit * i * region.extent.x;
        center.y += region.extent.y / 2;

        canvas.rrect(ShapeDesc{
            .center       = center,
            .extent       = Vec2::splat(min(track_extent.y, track_extent.x)),
            .corner_radii = Vec4::splat(1.0F),
            .tint         = style.thumb_color});
      }
    }

    Vec2 thumb_center =
        region.begin() + Vec2{region.extent.x * t, region.extent.y / 2};
    ColorGradient thumb_color =
        (state.hovered && !state.pressed && !state.disabled) ?
            style.thumb_hovered_color :
            style.thumb_color;
    f32 dilation =
        (state.hovered && !state.pressed && !state.disabled) ? 1.0F : 0.8F;
    Vec2 thumb_extent = style.thumb_frame(region.extent) * dilation;

    canvas.rrect(
        ShapeDesc{.center       = thumb_center,
                  .extent       = thumb_extent,
                  .corner_radii = style.thumb_corner_radii(thumb_extent.y),
                  .tint         = thumb_color});
  }
};

struct Switch : View
{
  struct State
  {
    bool disabled : 1 = false;
    bool value : 1    = false;
    bool hovered : 1  = false;
    bool pressed : 1  = false;
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
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    if (events.mouse_down && ctx.mouse_down(MouseButtons::Primary))
    {
      state.value = !state.value;
      on_changed(state.value);
      state.pressed = true;
    }

    if (events.mouse_up && ctx.mouse_up(MouseButtons::Primary))
    {
      state.pressed = false;
    }

    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = style.frame(allocated)};
  }

  virtual void render(CRect const &region, f32 scale, CRect const &,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .tint         = style.track_color});

    Vec2 const thumb_extent = style.thumb_frame(region.extent);
    Vec2 const thumb_center =
        region.begin() + space_align(region.extent, thumb_extent,
                                     Vec2{state.value ? -1.0F : 1.0F, 0});

    ColorGradient thumb_color;
    if (state.hovered)
    {
      thumb_color =
          state.value ? style.on_hovered_color : style.off_hovered_color;
    }
    else
    {
      thumb_color = state.value ? style.on_color : style.off_color;
    }

    canvas.rrect(ShapeDesc{.center       = thumb_center,
                           .extent       = thumb_extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .tint         = thumb_color});
  }
};

// [ ] implement
struct Collapsable : View
{
};

// [ ] scrolling
// [ ] selection
// [ ] clipping
// [ ] ios-style combo box
/// @param alignment alignment of the item, based on the text direction
struct ComboBoxItem : View
{
  struct State
  {
    bool disabled : 1 = false;
    bool hovered : 1  = false;
    bool selected : 1 = false;
    bool focused : 1  = false;
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
    Frame         frame            = {};
    f32           alignment        = -1;
  } style;

  Fn<void()> on_selected = fn([] {});

  RenderText text = {};

  ComboBoxItem()                                = default;
  ComboBoxItem(ComboBoxItem const &)            = delete;
  ComboBoxItem &operator=(ComboBoxItem const &) = delete;
  ComboBoxItem(ComboBoxItem &&);
  ComboBoxItem &operator=(ComboBoxItem &&);
  ~ComboBoxItem()
  {
    text.reset();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)>) override
  {
    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if (events.mouse_down && ctx.mouse_down(MouseButtons::Primary))
    {
      if (!state.selected)
      {
        state.selected = true;
        on_selected();
      }
    }

    if (events.key_down && ctx.key_down(KeyCode::Return))
    {
      if (!state.selected)
      {
        state.selected = true;
        on_selected();
      }
    }

    if (events.focus_in)
    {
      state.focused = true;
    }

    if (events.focus_out)
    {
      state.focused = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void render(CRect const &region, f32 scale, CRect const &clip,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center = region.center,
                           .extent = region.extent,
                           .tint   = state.hovered ?
                                         style.hovered_background_color :
                                         style.background_color});
    text.render(region, clip, canvas);
  }
};

/// [ ] z-index on expanded?
/// [ ] z-index effects on viewport
struct ComboBoxScrollView : View
{
  typedef Fn<void(u32, Span<u32 const>)> Selected;

  struct State
  {
    bool disabled : 1 = false;
    bool expanded : 1 = false;
  } state;

  struct Style
  {
    Frame         frame        = {};
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    f32           alignment    = 0;
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);
  } style;

  Vec<ComboBoxItem> items       = {};
  Selected          on_selected = fn([](u32, Span<u32 const>) {});

  ~ComboBoxScrollView()
  {
    items.reset();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    // [ ] scrolling
    // on scroll increase offset of views
    for (ComboBoxItem &item : items)
    {
      build(item);
    }
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> offsets) override
  {
    return {};
  }

  // virtual CRect clip(CRect const &region, CRect const &allocated) override
  // {
  //   return intersect(region, allocated);
  // }

  virtual void render(CRect const &region, f32 scale, CRect const &clip,
                      Canvas &canvas) override
  {
    // render rrect covering region
  }

  void clear_items();
  void add_item();
  bool is_opened();
  void close();
  void open();
  bool is_closed();
};

enum class ComboBoxMode : u8
{
  Inline = 0,
  Dialog = 1
};

struct ComboBox : View
{
  typedef ComboBoxScrollView::Selected Selected;

  struct State
  {
    bool disabled : 1 = false;
    bool hovered : 1  = false;
    bool pressed : 1  = false;
  } state;

  ComboBoxScrollView scroll_view;

  struct Style
  {
    ComboBoxMode  mode         = ComboBoxMode::Inline;
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.surface);
    ColorGradient hovered_color =
        ColorGradient::all(DEFAULT_THEME.surface_variant);
    ColorGradient text_color = ColorGradient::all(DEFAULT_THEME.on_surface);
    ColorGradient inactive_text_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    Frame frame =
        Frame{.width = {.scale = 1, .max = 200}, .height = {.offset = 25}};
  } style;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)> build) override
  {
    if (ctx.mouse_down(MouseButtons::Primary))
    {
      if (scroll_view.is_opened())
      {
        scroll_view.close();
      }
      else if (!state.disabled)
      {
        scroll_view.open();
      }
    }

    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if (events.focus_in)
    {
    }

    build(scroll_view);

    return ViewState{.clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual void render(CRect const &region, f32 scale, CRect const &clip,
                      Canvas &canvas) override
  {
    canvas.rrect(
        ShapeDesc{.center       = region.center,
                  .extent       = region.extent,
                  .corner_radii = style.corner_radii(region.extent.y),
                  .tint = state.hovered ? style.hovered_color : style.color});
    //[ ] draw selection text
    //[ ]  draw button: scroll_view.opened?
    // [ ] focus
  }
};

struct RadioBox : View
{
  struct State
  {
    bool disabled : 1      = false;
    bool pointer_enter : 1 = false;
    bool pointer_leave : 1 = false;
    bool pointer_down : 1  = false;
    bool pointer_up : 1    = false;
    bool hovered : 1       = false;
    bool focused : 1       = false;
    bool pressed : 1       = false;
    bool value : 1         = false;
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
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if (events.focus_in)
    {
      state.focused = true;
    }

    if (events.focus_out)
    {
      state.focused = false;
    }

    if ((events.mouse_down && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      state.pressed = true;
      state.value   = !state.value;
      on_changed(state.value);
    }

    if ((events.mouse_up && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_up && ctx.key_up(KeyCode::Return)))
    {
      state.pressed = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = style.frame(allocated)};
  }

  virtual void render(CRect const &region, f32 zoom, CRect const &,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .stroke       = 1,
                           .thickness    = style.thickness,
                           .tint         = style.color});

    if (state.value)
    {
      Vec2 inner_extent = region.extent * (state.hovered ? 0.75F : 0.5F);
      ColorGradient inner_color =
          state.hovered ? style.inner_hovered_color : style.inner_color;

      canvas.circle(ShapeDesc{.center = region.center,
                              .extent = inner_extent,
                              .tint   = inner_color});
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
    i32 i32;
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

struct ScalarDragBox : View, Pin<>
{
  typedef Fn<void(fmt::Context const &, ScalarInput)> Fmt;
  typedef Fn<void(Span<u32 const>, ScalarState &)>    Parse;

  struct State
  {
    bool        disabled : 1   = false;
    bool        input_mode : 1 = false;
    bool        dragging : 1   = false;
    ScalarState value          = {};
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
  } style;

  TextInput input = {};
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
    defer   utf8_{[&] { utf8.reset(); }};
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

    return ViewState{.pointable = !state.disabled,
                     .draggable = !state.disabled};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 child = style.frame(allocated) - 2 * style.padding(allocated);
    child.x    = max(child.x, 0.0F);
    child.y    = max(child.y, 0.0F);
    fill(sizes, child);
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> offsets) override
  {
    fill(offsets, Vec2{0, 0});
    return {.extent = sizes[0] + 2 * style.padding(allocated)};
  }

  virtual void render(CRect const &region, f32 zoom, CRect const &,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
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
      Vec2       thumb_center{0, region.center.y};

      thumb_center.x =
          space_align(region.extent.x, thumb_extent.x, norm_to_axis(t));

      canvas.rrect(
          ShapeDesc{.center       = thumb_center,
                    .extent       = thumb_extent,
                    .corner_radii = Vec4::splat(region.extent.y * 0.125F),
                    .tint         = style.thumb_color});
    }
  }

  virtual Cursor cursor(CRect const &region, f32, Vec2 offset) override
  {
    (void) region;
    (void) offset;
    return state.disabled ? Cursor::Default : Cursor::EWResize;
  }
};

struct ScalarBox : FlexView, Pin<>
{
  struct State
  {
    bool disabled : 1  = false;
    bool steppable : 1 = true;
    bool draggable : 1 = true;
  } state;

  Fn<void(ScalarInput)> on_update = fn([](ScalarInput) {});

  TextButton    dec  = {};
  TextButton    inc  = {};
  ScalarDragBox drag = {};

  ScalarBox()
  {
    FlexView::style.axis        = Axis::X;
    FlexView::style.wrap        = false;
    FlexView::style.reverse     = false;
    FlexView::style.main_align  = MainAlign::Start;
    FlexView::style.cross_align = 0;
    FlexView::style.frame =
        Frame{.width = {.scale = 1}, .height = {.scale = 1}};

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

    inc.style.stroke = dec.style.stroke = drag.style.stroke;
    inc.style.thickness = dec.style.thickness = drag.style.thickness;
    inc.style.padding = dec.style.padding = drag.style.padding;
    inc.style.frame.width = dec.style.frame.width = drag.style.frame.height;
    inc.style.frame.height = dec.style.frame.height = drag.style.frame.height;
    inc.style.corner_radii = dec.style.corner_radii = drag.style.corner_radii;

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
    dec.state.disabled  = state.disabled || !state.steppable;
    inc.state.disabled  = state.disabled || !state.steppable;
    drag.state.disabled = state.disabled || !state.draggable;
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
    bool disabled : 1 = false;
    bool hovered : 1  = false;
    bool dragging : 1 = false;
    f32  t            = 0;
  } state;

  struct Style
  {
    Axis          direction = Axis::X;
    ColorGradient thumb_color =
        ColorGradient::all(DEFAULT_THEME.inactive * opacity(0.75F));
    ColorGradient thumb_dragging_color =
        ColorGradient::all(DEFAULT_THEME.active * opacity(0.75F));
    ColorGradient track_color =
        ColorGradient::all(DEFAULT_THEME.inactive * opacity(0.75F));
    Vec2 frame_extent   = {};
    Vec2 content_extent = {};
  } style;

  Fn<void(f32)> on_scrolled = fn([](f32) {});

  explicit ScrollBar(Axis direction) : style{.direction = direction}
  {
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View &)>) override
  {
    u8 const main_axis = (style.direction == Axis::X) ? 0 : 1;

    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if (events.dragging)
    {
      state.dragging = true;
      state.t =
          clamp((ctx.mouse.position[main_axis] - region.extent[main_axis] / 2) /
                    region.extent[main_axis],
                0.0f, 1.0f);
      on_scrolled(state.t);
    }

    if (events.drag_end)
    {
      state.dragging = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .draggable = !state.disabled};
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return {.extent = allocated};
  }

  virtual void render(CRect const &region, f32 zoom, CRect const &,
                      Canvas &canvas) override
  {
    u8 const   main_axis    = (style.direction == Axis::X) ? 0 : 1;
    u8 const   cross_axis   = (style.direction == Axis::Y) ? 1 : 0;
    Vec4 const corner_radii = Vec4::splat(region.extent.y * 0.09F);

    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii,
                           .stroke       = 0,
                           .tint         = style.track_color});

    // calculate thumb main axis extent
    f32 const thumb_scale =
        style.frame_extent[main_axis] / style.content_extent[main_axis];
    Vec2 thumb_extent        = {0, 0};
    thumb_extent[cross_axis] = region.extent[cross_axis];
    thumb_extent[main_axis]  = thumb_scale * region.extent[main_axis];

    // align thumb to remaining space based on size of visible region
    Vec2 const bar_offset  = region.begin();
    f32 const main_spacing = thumb_extent[main_axis] - region.extent[main_axis];
    Vec2      thumb_center;
    thumb_center[main_axis] = bar_offset[main_axis] + main_spacing * state.t +
                              thumb_extent[main_axis] / 2;
    thumb_center[cross_axis] = region.center[cross_axis];

    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii,
                           .stroke       = 1,
                           .thickness    = 1,
                           .tint         = style.track_color});

    canvas.rrect(ShapeDesc{.center       = thumb_center,
                           .extent       = thumb_extent,
                           .corner_radii = corner_radii,
                           .stroke       = 0,
                           .tint = state.dragging ? style.thumb_dragging_color :
                                                    style.thumb_color});
  }
};

struct ScrollView : View
{
  struct State
  {
    bool disabled : 1 = false;
  } state;

  struct Style
  {
    Axes  axes       = Axes::X | Axes::Y;
    Frame frame      = {.width = {200}, .height = {200}};
    Size  x_bar_size = {.offset = 10};
    Size  y_bar_size = {.offset = 10};
  } style;

  ScrollBar x_bar = ScrollBar{Axis::X};
  ScrollBar y_bar = ScrollBar{Axis::Y};

  virtual ViewState tick(ViewContext const &, CRect const &, ViewEvents,
                         Fn<void(View &)> build) override
  {
    x_bar.state.disabled = y_bar.state.disabled = state.disabled;
    build(x_bar);
    build(y_bar);
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame        = style.frame(allocated);
    f32 const  x_bar_size_r = style.x_bar_size(allocated.x);
    f32 const  y_bar_size_r = style.y_bar_size(allocated.y);

    sizes[0] = {frame.x, x_bar_size_r};

    if (has_bits(style.axes, Axes::Y))
    {
      sizes[0].x -= y_bar_size_r;
    }

    sizes[1] = {y_bar_size_r, frame.y};

    fill(sizes.slice(2), frame);
  }

  virtual ViewExtent fit(Vec2 allocated, Span<Vec2 const> sizes,
                         Span<Vec2> offsets) override
  {
    Vec2 const frame = style.frame(allocated);
    offsets[0]       = space_align(frame, sizes[0], Vec2{1, 0});
    offsets[1]       = space_align(frame, sizes[1], Vec2{-1, 1});

    Vec2 content_size;
    for (Vec2 const &sz : sizes.slice(2))
    {
      content_size.x = max(content_size.x, sz.x);
      content_size.y = max(content_size.y, sz.y);
    }

    Vec2 const displacement =
        -1 * (content_size - frame) * Vec2{x_bar.state.t, y_bar.state.t};

    fill(offsets.slice(2), displacement);

    x_bar.style.content_extent = y_bar.style.content_extent = content_size;
    x_bar.style.frame_extent = y_bar.style.frame_extent = frame;

    return {.extent = frame};
  }

  virtual i32 z_index(i32 z_index, Span<i32> indices) override
  {
    static constexpr i32 ELEVATION = 128;
    indices[0]                     = z_index + ELEVATION;
    indices[1]                     = z_index + ELEVATION;
    fill(indices.slice(2), z_index + 1);
    return z_index;
  }

  // virtual CRect clip(CRect const &region, CRect const &allocated) override
  // {
  //   return intersect(region.offseted(), allocated.offseted()).centered();
  // }
};

// [ ] implement
struct ContextMenu : View
{
};

// [ ] implement
/// REQUIREMENTS
/// - Linear and Non-Linear Color Space Independence
/// - Rectangular Box with visualizations
/// - Text-based manual input
/// - RGB, SRGB, HSV, HEX, Linear, Hue, YUV
// - [ ] color space, pixel info for color pickers
struct ColorPicker : View
{
};

/// [ ] REQUIREMENTS:
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

// [ ] header selectors.
// [ ] navigation model, with keyboard and gamepad
struct Tab : View
{
};

}        // namespace ash