/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

using Scalar = Enum<f32, i32>;

/// @param start starting value, this is the value to be reset to when cancel is
/// requested
/// @param min minimum value of the scalar
/// @param max maximum value of the scalar
/// @param step step in either direction that should be taken. i.e. when `+` or
/// `-` is pressed.
/// @param current current value of the scalar, mutated by the GUI system
struct F32Info
{
  f32 base = 0;

  f32 min = 0;

  f32 max = 1;

  f32 step = 0.01F;

  constexpr f32 step_value(f32 current, f32 direction) const
  {
    return clamp(current + direction * step, min, max);
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
struct I32Info
{
  i32 base = 0;

  i32 min = 0;

  i32 max = 1'000;

  i32 step = 100;

  constexpr i32 step_value(i32 current, f32 direction) const
  {
    return clamp((i32) (current + direction * step), min, max);
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

using ScalarInfo = Enum<F32Info, I32Info>;

}    // namespace ui

inline void format(fmt::Sink sink, fmt::Spec spec, ui::Scalar const & value)
{
  return value.match([&](f32 f) { return format(sink, spec, f); },
                     [&](i32 i) { return format(sink, spec, i); });
}

namespace ui
{

struct Space : View
{
  struct Style
  {
    Frame frame{};
  } style;

  Space()                          = default;
  Space(Space const &)             = delete;
  Space(Space &&)                  = default;
  Space & operator=(Space const &) = delete;
  Space & operator=(Space &&)      = default;
  virtual ~Space() override        = default;

  Space & frame(Frame frame);

  Space & frame(Vec2 extent, bool constrain = true);

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;
};

/// @param axis flex axis to layout children along
/// @param main_align main-axis alignment. specifies how free space is used on
/// the main axis
/// @param cross_align cross-axis alignment. affects how free space is used on
/// the cross axis
struct Flex : View
{
  struct Style
  {
    Axis axis = Axis::X;

    bool wrap = true;

    MainAlign main_align = MainAlign::Start;

    f32 cross_align = 0;

    Frame frame = Frame{}.scale(1, 1);

    Frame item_frame = Frame{}.scale(1, 1);
  } style;

  Vec<ref<View>> items_;

  Flex(AllocatorRef allocator = default_allocator);
  Flex(Flex const &)             = delete;
  Flex(Flex &&)                  = default;
  Flex & operator=(Flex const &) = delete;
  Flex & operator=(Flex &&)      = default;
  virtual ~Flex() override       = default;

  Flex & axis(Axis axis);

  Flex & wrap(bool wrap);

  Flex & main_align(MainAlign align);

  Flex & cross_align(f32 align);

  Flex & frame(Vec2 extent, bool constrain = true);

  Flex & frame(Frame frame);

  Flex & item_frame(Frame frame);

  Flex & item_frame(Vec2 extent, bool constrain = true);

  Flex & items(std::initializer_list<ref<View>> list);

  Flex & items(Span<ref<View> const> list);

  virtual State tick(Ctx const & ctx, Events const & events,
                     Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;
};

struct Stack : View
{
  struct Style
  {
    bool reverse = false;

    Vec2 alignment = ALIGNMENT_CENTER_CENTER;

    Frame frame = Frame{}.scale({1, 1});
  } style;

  Vec<ref<View>> items_;

  Stack(AllocatorRef allocator = default_allocator);
  Stack(Stack const &)             = delete;
  Stack(Stack &&)                  = default;
  Stack & operator=(Stack const &) = delete;
  Stack & operator=(Stack &&)      = default;
  virtual ~Stack() override        = default;

  Stack & reverse(bool reverse);

  Stack & align(Vec2 alignment);

  Stack & frame(Vec2 extent, bool constrain = true);

  Stack & frame(Frame frame);

  Stack & items(std::initializer_list<ref<View>> list);

  Stack & items(Span<ref<View> const> list);

  virtual i32 stack_item(i32 base, u32 index, u32 num);

  virtual State tick(Ctx const & ctx, Events const & events,
                     Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers) override;

  virtual i32 z_index(i32 allocated, Span<i32> indices) override;
};

struct Text : View
{
  struct State
  {
    bool copyable = false;
  } state;

  struct Style
  {
    TextHighlightStyle highlight{.color        = theme.highlight,
                                 .corner_radii = Vec4::splat(2.5F)};
  } style;

  RenderText text_;

  TextCompositor compositor_;

  Text(Str32             text      = U""_str,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Text(Str8              text,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Text(Text const &)             = delete;
  Text(Text &&)                  = default;
  Text & operator=(Text const &) = delete;
  Text & operator=(Text &&)      = default;
  virtual ~Text() override       = default;

  Text & copyable(bool allow);

  Text & highlight_style(TextHighlightStyle highlight);

  Text & clear_highlights();

  Text & run(TextStyle const & style, FontStyle const & font, u32 first = 0,
             u32 count = U32_MAX);

  Text & text(Str32 text);

  Text & text(Str8 text);

  Str32 text() const;

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

// [ ] scroll and clip text if region isn't large enough
// [ ] wrapping to the next line if not large enough
// [ ] no wrap
// [ ] max-len/filter function
// [ ] secret text input
struct Input : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool editing : 1 = false;

    bool submit : 1 = false;

    bool multiline : 1 = false;

    bool enter_submits : 1 = false;

    bool tab_input : 1 = false;

    FocusState focus = {};
  } state;

  struct Style
  {
    TextHighlightStyle highlight = {.color        = theme.highlight,
                                    .corner_radii = Vec4::splat(2.5F)};
    CaretStyle         caret{.color = theme.caret, .thickness = 1.0F};
    usize              lines_per_page = 40;
    usize              tab_width      = 1;
  } style;

  struct Callbacks
  {
    Fn<void()> edit = noop;

    Fn<void()> submit = noop;

    Fn<void()> focus_in = noop;

    Fn<void()> focus_out = noop;
  } cb;

  AllocatorRef allocator_;

  RenderText content_;

  RenderText stub_;

  TextCompositor compositor_;

  Input(Str32             stub      = U""_str,
        TextStyle const & style     = TextStyle{.color = theme.on_surface},
        FontStyle const & font      = FontStyle{.font   = theme.body_font,
                                                .height = theme.body_font_height,
                                                .line_height = theme.line_height},
        AllocatorRef      allocator = default_allocator);

  Input(Str8              stub,
        TextStyle const & style     = TextStyle{.color = theme.on_surface},
        FontStyle const & font      = FontStyle{.font   = theme.body_font,
                                                .height = theme.body_font_height,
                                                .line_height = theme.line_height},
        AllocatorRef      allocator = default_allocator);

  Input(Input const &)             = delete;
  Input(Input &&)                  = default;
  Input & operator=(Input const &) = delete;
  Input & operator=(Input &&)      = default;
  virtual ~Input() override        = default;

  Input & disable(bool disable);

  Input & multiline(bool enable);

  Input & enter_submits(bool enable);

  Input & tab_input(bool enable);

  Input & on_edit(Fn<void()> fn);

  Input & on_submit(Fn<void()> fn);

  Input & on_focus_in(Fn<void()> fn);

  Input & on_focus_out(Fn<void()> fn);

  Input & content(Str8 text);

  Input & content(Str32 text);

  Input & content_run(TextStyle const & style, FontStyle const & font,
                      u32 first = 0, u32 count = U32_MAX);

  Input & stub(Str8 text);

  Input & stub(Str32 text);

  Input & stub_run(TextStyle const & style, FontStyle const & font,
                   u32 first = 0, u32 count = U32_MAX);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

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
    bool disabled = false;

    PressState press = {};
  } state;

  struct Style
  {
    Vec4U8 color = theme.primary;

    Vec4U8 hovered_color = theme.primary_variant;

    Vec4U8 disabled_color = theme.inactive;

    f32 stroke = 0.0F;

    f32 thickness = 1.0F;

    ButtonShape shape = ButtonShape::RRect;

    Vec2 padding = {};

    CornerRadii corner_radii = CornerRadii::all(2);

    Frame frame = Frame{}.scale(1, 1);
  } style;

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

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

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
                   u32 first = 0, u32 count = U32_MAX);

  TextButton & text(Str32 text);

  TextButton & text(Str8 text);

  TextButton & color(Vec4U8 color);

  TextButton & hovered_color(Vec4U8 color);

  TextButton & disabled_color(Vec4U8 color);

  TextButton & rrect(CornerRadii const & radii);

  TextButton & squircle(f32 degree = 5);

  TextButton & bevel(CornerRadii const & radii);

  TextButton & frame(Vec2 extent, bool constrain = true);

  TextButton & frame(Frame frame);

  TextButton & stroke(f32 stroke);

  TextButton & thickness(f32 thickness);

  TextButton & padding(Vec2 padding);

  TextButton & padding(f32 x, f32 y);

  TextButton & on_pressed(Fn<void()> fn);

  TextButton & on_hovered(Fn<void()> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;
};

struct Icon : View
{
  struct State
  {
    bool hidden = false;
  } state;

  RenderText text_;

  Icon(Str32             text      = U""_str,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.icon_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Icon(Str8              text,
       TextStyle const & style     = TextStyle{.color = theme.on_surface},
       FontStyle const & font      = FontStyle{.font        = theme.icon_font,
                                               .height      = theme.body_font_height,
                                               .line_height = theme.line_height},
       AllocatorRef      allocator = default_allocator);

  Icon(Icon const &)             = delete;
  Icon(Icon &&)                  = default;
  Icon & operator=(Icon const &) = delete;
  Icon & operator=(Icon &&)      = default;
  virtual ~Icon() override       = default;

  Icon & hide(bool hide);

  Icon & icon(Str8 text, TextStyle const & style, FontStyle const & font);

  Icon & icon(Str32 text, TextStyle const & style, FontStyle const & font);

  ui::State tick(Ctx const & ctx, Events const & events,
                 Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;
};

struct CheckBox : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool value : 1 = false;

    PressState press = {};
  } state;

  struct Style
  {
    Vec4U8 box_color = theme.inactive;

    Vec4U8 box_hovered_color = theme.active;

    f32 stroke = 1;

    f32 thickness = 0.5F;

    CornerRadii corner_radii = CornerRadii::all(2);

    f32 padding = 2.5F;
  } style;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  Icon icon_;

  CheckBox(Str32             text      = U""_str,
           TextStyle const & style     = TextStyle{.color = theme.on_surface},
           FontStyle const & font      = FontStyle{.font   = theme.icon_font,
                                                   .height = theme.body_font_height,
                                                   .line_height = theme.line_height},
           AllocatorRef      allocator = default_allocator);

  CheckBox(Str8              text,
           TextStyle const & style     = TextStyle{.color = theme.on_surface},
           FontStyle const & font      = FontStyle{.font   = theme.icon_font,
                                                   .height = theme.body_font_height,
                                                   .line_height = theme.line_height},
           AllocatorRef      allocator = default_allocator);

  CheckBox(CheckBox const &)             = delete;
  CheckBox(CheckBox &&)                  = default;
  CheckBox & operator=(CheckBox const &) = delete;
  CheckBox & operator=(CheckBox &&)      = default;
  virtual ~CheckBox() override           = default;

  Icon & icon();

  CheckBox & disable(bool disable);

  CheckBox & box_color(Vec4U8 color);

  CheckBox & box_hovered_color(Vec4U8 color);

  CheckBox & stroke(f32 stroke);

  CheckBox & thickness(f32 thickness);

  CheckBox & corner_radii(CornerRadii const & radii);

  CheckBox & padding(f32 padding);

  CheckBox & on_changed(Fn<void(bool)> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

/// @brief Multi-directional Slider
struct Slider : View
{
  struct State
  {
    bool disabled = false;

    DragState drag = {};

    f32 t = 0;

    f32 low = 0;

    f32 high = 1;
  } state;

  struct Style
  {
    Axis axis = Axis::X;

    Frame frame = Vec2{360, theme.body_font_height};

    f32 thumb_size = theme.body_font_height * 0.75F;

    f32 track_size = 4;

    Vec4U8 thumb_color = theme.primary;

    Vec4U8 thumb_hovered_color = theme.primary;

    Vec4U8 thumb_dragging_color = theme.primary_variant;

    CornerRadii thumb_corner_radii = CornerRadii::all(2.5);

    Vec4U8 track_color = theme.inactive;

    CornerRadii track_corner_radii = CornerRadii::all(2.5);

    f32 delta = 0.1F;
  } style;

  struct Callbacks
  {
    Fn<void(f32)> changed = noop;
  } cb;

  Slider()                           = default;
  Slider(Slider const &)             = delete;
  Slider(Slider &&)                  = default;
  Slider & operator=(Slider const &) = delete;
  Slider & operator=(Slider &&)      = default;
  virtual ~Slider() override         = default;

  Slider & disable(bool disable);

  Slider & range(f32 low, f32 high);

  Slider & interp(f32 t);

  Slider & axis(Axis axis);

  Slider & frame(Vec2 extent, bool constrain = true);

  Slider & frame(Frame frame);

  Slider & thumb_size(f32 size);

  Slider & track_size(f32 size);

  Slider & thumb_color(Vec4U8 color);

  Slider & thumb_hovered_color(Vec4U8 color);

  Slider & thumb_dragging_color(Vec4U8 color);

  Slider & thumb_corner_radii(CornerRadii const & color);

  Slider & track_color(Vec4U8 color);

  Slider & track_corner_radii(CornerRadii const & radii);

  Slider & on_changed(Fn<void(f32)> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

struct Switch : View
{
  struct State
  {
    bool disabled = false;

    PressState press = {};

    bool value = false;
  } state;

  struct Style
  {
    Vec4U8 on_color = theme.primary;

    Vec4U8 on_hovered_color = theme.primary_variant;

    Vec4U8 off_color = theme.active;

    Vec4U8 off_hovered_color = theme.inactive;

    Vec4U8 track_color = theme.surface_variant;

    f32 track_thickness = 1;

    f32 track_stroke = 0;

    CornerRadii corner_radii = CornerRadii::all(4);

    Frame frame = Vec2{40, 20};
  } style;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  Switch()                           = default;
  Switch(Switch const &)             = delete;
  Switch(Switch &&)                  = default;
  Switch & operator=(Switch const &) = delete;
  Switch & operator=(Switch &&)      = default;
  virtual ~Switch() override         = default;

  Switch & disable(bool disable);

  Switch & on();

  Switch & off();

  Switch & toggle();

  Switch & on_color(Vec4U8 color);

  Switch & on_hovered_color(Vec4U8 color);

  Switch & off_color(Vec4U8 color);

  Switch & off_hovered_color(Vec4U8 color);

  Switch & track_color(Vec4U8 color);

  Switch & corner_radii(CornerRadii const & radii);

  Switch & frame(Vec2 extent, bool constrain = true);

  Switch & frame(Frame frame);

  Switch & thumb_frame(Vec2 extent, bool constrain = true);

  Switch & thumb_frame(Frame frame);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

struct Radio : View
{
  struct State
  {
    bool       disabled : 1 = false;
    bool       value    : 1 = false;
    PressState press        = {};
  } state;

  struct Style
  {
    Frame frame = Vec2{20, 20};

    CornerRadii corner_radii = CornerRadii::all(0.5);

    f32 thickness = 0.5F;

    Vec4U8 color = theme.inactive;

    Vec4U8 inner_color = theme.primary;

    Vec4U8 inner_hovered_color = theme.primary_variant;
  } style;

  struct Callbacks
  {
    Fn<void(bool)> changed = noop;
  } cb;

  Radio()                          = default;
  Radio(Radio const &)             = delete;
  Radio(Radio &&)                  = default;
  Radio & operator=(Radio const &) = delete;
  Radio & operator=(Radio &&)      = default;
  virtual ~Radio() override        = default;

  Radio & disable(bool disable);

  Radio & corner_radii(CornerRadii const & radii);

  Radio & thickness(f32 thickness);

  Radio & color(Vec4U8 color);

  Radio & inner_color(Vec4U8 color);

  Radio & inner_hovered_color(Vec4U8 color);

  Radio & frame(Vec2 extent, bool constrain = true);

  Radio & frame(Frame frame);

  Radio & on_changed(Fn<void(bool)> fn);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

struct ScalarDragBox : View
{
  struct State
  {
    static constexpr hash64 HASH_DIRTY = 0;
    static constexpr hash64 HASH_CLEAN = 1;

    bool disabled : 1 = false;

    bool input_mode : 1 = false;

    bool dragging : 1 = false;

    FocusState focus = {};

    ScalarInfo spec = F32Info{};

    Scalar scalar = 0.0F;

    hash64 hash = HASH_DIRTY;
  } state;

  struct Style
  {
    Frame frame = Frame{}.min(200, theme.body_font_height);

    Vec2 padding = {2.5, 2.5};

    CornerRadii corner_radii = CornerRadii::all(2);

    Vec4U8 color = theme.inactive;

    Vec4U8 thumb_color = theme.inactive;

    f32 stroke = 1.0F;

    f32 thickness = 1.0F;

    Str format = "{}"_str;
  } style;

  Input input_;

  struct Callbacks
  {
    Fn<void(Scalar)> update = noop;
  } cb;

  ScalarDragBox(
    TextStyle const & style     = TextStyle{.color = theme.on_surface},
    FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                            .height      = theme.body_font_height,
                                            .line_height = theme.line_height},
    AllocatorRef      allocator = default_allocator) :
    input_{U""_str, style, font, allocator}
  {
    input_.multiline(false).tab_input(false).enter_submits(false);
  }

  ScalarDragBox(ScalarDragBox const &)             = delete;
  ScalarDragBox(ScalarDragBox &&)                  = default;
  ScalarDragBox & operator=(ScalarDragBox const &) = delete;
  ScalarDragBox & operator=(ScalarDragBox &&)      = default;
  virtual ~ScalarDragBox() override                = default;

  static void scalar_parse(Str32 text, ScalarInfo const & spec, Scalar &);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 offset) override;
};

struct ScalarBox : Flex
{
  struct Callbacks
  {
    Fn<void(Scalar)> update = noop;
  } cb;

  TextButton dec_;

  TextButton inc_;

  ScalarDragBox drag_;

  ScalarBox(
    Str32 decrease_text = U"remove"_str, Str32 increase_text = U"add"_str,
    TextStyle const & button_text_style =
      TextStyle{
        .shadow_scale = 1, .shadow_offset = {1, 1},
             .color = theme.on_primary
  },
    TextStyle const & drag_text_style = TextStyle{.shadow_scale  = 1,
                                                  .shadow_offset = {1, 1},
                                                  .color = theme.on_primary},
    FontStyle const & icon_font       = FontStyle{.font   = theme.icon_font,
                                                  .height = theme.body_font_height,
                                                  .line_height = theme.line_height},
    FontStyle const & text_font       = FontStyle{.font   = theme.body_font,
                                                  .height = theme.body_font_height,
                                                  .line_height = theme.line_height},
    AllocatorRef      allocator       = default_allocator);

  ScalarBox(ScalarBox const &)             = delete;
  ScalarBox(ScalarBox &&)                  = default;
  ScalarBox & operator=(ScalarBox const &) = delete;
  ScalarBox & operator=(ScalarBox &&)      = default;
  virtual ~ScalarBox() override            = default;

  ScalarBox & step(i32 direction);

  ScalarBox & stub(Str32 text);

  ScalarBox & stub(Str8 text);

  ScalarBox & format(Str format);

  ScalarBox & spec(f32 scalar, F32Info info);

  ScalarBox & spec(i32 scalar, I32Info info);

  ScalarBox & stroke(f32 stroke);

  ScalarBox & thickness(f32 thickness);

  ScalarBox & padding(Vec2 padding);

  ScalarBox & frame(Vec2 extent, bool constrain = true);

  ScalarBox & frame(Frame frame);

  ScalarBox & corner_radii(CornerRadii const & radii);

  ScalarBox & on_update(Fn<void(Scalar)> fn);

  ScalarBox & button_text_style(TextStyle const & style, FontStyle const & font,
                                u32 first = 0, u32 count = U32_MAX);

  ScalarBox & drag_text_style(TextStyle const & style, FontStyle const & font,
                              u32 first = 0, u32 count = U32_MAX);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;
};

struct ScrollBar : View
{
  struct State
  {
    bool      disabled : 1 = false;
    bool      hidden   : 1 = false;
    DragState drag         = {};
    f32       t            = 0;
  } state;

  struct Style
  {
    Axis axis = Axis::X;

    f32 thumb_size = 15;

    Vec4U8 thumb_color = theme.primary;

    Vec4U8 thumb_hovered_color = theme.primary_variant;

    Vec4U8 thumb_dragging_color = theme.primary;

    CornerRadii thumb_corner_radii = CornerRadii::all(2);

    Vec4U8 track_color = theme.inactive;

    CornerRadii track_corner_radii = CornerRadii::all(2);

    f32 delta = 0.1F;
  } style;

  ScrollBar()                              = default;
  ScrollBar(ScrollBar const &)             = delete;
  ScrollBar(ScrollBar &&)                  = default;
  ScrollBar & operator=(ScrollBar const &) = delete;
  ScrollBar & operator=(ScrollBar &&)      = default;
  virtual ~ScrollBar() override            = default;

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;
};

struct ScrollView : View
{
  struct State
  {
    bool disabled = false;

    Vec2 zoom = {1, 1};
  } state;

  struct Style
  {
    Frame frame = Vec2{200, 200};

    Frame inner_frame = Frame{}.rmax(F32_INF, F32_INF);

    f32 x_bar_size = 10;

    f32 y_bar_size = 10;

  } style;

  ScrollBar x_bar_{};

  ScrollBar y_bar_{};

  ref<View> child_;

  ScrollView(View & child);
  ScrollView(ScrollView const &)             = delete;
  ScrollView(ScrollView &&)                  = default;
  ScrollView & operator=(ScrollView const &) = delete;
  ScrollView & operator=(ScrollView &&)      = default;
  virtual ~ScrollView() override             = default;

  ScrollView & disable(bool d);

  ScrollView & item(View & view);

  ScrollView & thumb_size(f32 size);

  ScrollView & thumb_color(Vec4U8 color);

  ScrollView & thumb_hovered_color(Vec4U8 color);

  ScrollView & thumb_dragging_color(Vec4U8 color);

  ScrollView & thumb_corner_radii(CornerRadii const & c);

  ScrollView & track_color(Vec4U8 color);

  ScrollView & track_corner_radii(CornerRadii const & c);

  ScrollView & axes(Axes axes);

  ScrollView & frame(Vec2 extent, bool constrain);

  ScrollView & frame(Frame f);

  ScrollView & inner_frame(Frame f);

  ScrollView & inner_frame(Vec2 extent, bool constrain);

  ScrollView & bar_size(f32 x, f32 y);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual i32 layer(i32 allocated, Span<i32> children) override;
};

struct ComboItem : View
{
  struct State
  {
    bool disabled = false;

    bool selected = false;

    u32 id = 0;

    Fn<void(u32)> click_hook = noop;
  } state;

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

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;

  virtual Cursor cursor(Vec2 extent, Vec2 position) override;
};

struct TextComboItem : ComboItem
{
  struct State
  {
    PressState press;
  } state;

  struct Style
  {
    Frame frame = Frame{}.scale(1, 1);

    Vec2 padding = {5, 5};

    f32 alignment = ALIGNMENT_LEFT;

    Vec4U8 color = theme.surface_variant;

    Vec4U8 hover_color = theme.primary_variant;

    Vec4U8 selected_color = theme.primary;

    f32 stroke = 0;

    f32 thickness = 1;

    CornerRadii corner_radii = CornerRadii::all(2);
  } style;

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

  TextComboItem & frame(Vec2 extent, bool constrain = true);

  TextComboItem & frame(Frame frame);

  TextComboItem & padding(Vec2 padding);

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

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;
};

struct Combo : Flex
{
  struct State
  {
    bool disabled = false;

    Option<u32> selected = none;
  } state;

  struct Style
  {
    CornerRadii corner_radii = CornerRadii::all(2);

    Vec4U8 color = theme.surface;

    f32 stroke = 0;

    f32 thickness = 1;
  } style;

  struct Callbacks
  {
    Fn<void(Option<u32>)> selected = noop;
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

  Combo & frame(Vec2 extent, bool constrain = true);

  Combo & frame(Frame frame);

  Combo & item_frame(Frame frame);

  Combo & item_frame(Vec2 extent, bool constrain = true);

  Combo & disable(bool disable);

  Combo & color(Vec4U8 color);

  Combo & corner_radii(CornerRadii radii);

  Combo & on_selected(Fn<void(Option<u32>)> style);

  Combo & items(std::initializer_list<ref<ComboItem>> list);

  Combo & items(Span<ref<ComboItem> const> list);

  u32 num_items() const;

  Combo & select(Option<u32> item);

  Option<u32> get_selection() const;

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;
};

using ImageSrc = Enum<None, ImageId, Future<Result<ImageId, ImageLoadErr>>>;

enum class ImageFit : u8
{
  /// @brief try to contain the image within the frame
  /// without distorting it (preserving aspect ratio)
  Contain = 0,

  /// @brief crop the image to fit within the frame
  Crop = 1,

  /// @brief distort the image to fill the frame
  Fit = 2
};

struct Image : View
{
  struct State
  {
    Enum<None, Option<ash::ImageInfo>, ImageLoadErr> resolved = none;
  } state;

  struct Style
  {
    Option<f32> aspect_ratio = none;

    Frame frame = Frame{}.offset(250, 250);

    CornerRadii radii = CornerRadii::all(2);

    ColorGradient tint = colors::WHITE;

    ImageFit fit = ImageFit::Contain;

    Vec2 alignment = ALIGNMENT_CENTER_CENTER;
  } style;

  ImageSrc src_;

  Image(ImageSrc src = None{});

  Image & source(ImageSrc src);

  Image & aspect_ratio(f32 width, f32 height);

  Image & aspect_ratio(Option<f32> ratio);

  Image & frame(Frame frame);

  Image & frame(Vec2 extent, bool constrain = true);

  Image & corner_radii(CornerRadii const & radii);

  Image & tint(ColorGradient const & color);

  Image & fit(ImageFit fit);

  Image & align(Vec2 alignment);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, CRect const & region, Vec2 zoom,
                      CRect const & clip) override;
};

// [ ] size estimation?
struct List : View
{
  typedef Fn<Option<Dyn<View *>>(AllocatorRef, usize i)> Generator;

  static constexpr auto DEFAULT_GENERATOR =
    [](AllocatorRef, usize) -> Option<Dyn<View *>> { return none; };

  struct State
  {
    Slice range = {};

    /// @brief total translation of the entire list
    f32 translation = 0;

    /// @brief virtual view translation of the currently visible list items
    f32 virtual_translation = 0;

    Option<f32> item_size = none;

    Generator generator;
  } state_;

  struct Style
  {
    Axis axis = Axis::X;

    Frame frame = Frame{}.scale(1, 1);

    Frame item_frame = Frame{}.scale(1, 1);
  } style_;

  AllocatorRef allocator_;

  Vec<Dyn<View *>> items_;

  List(Generator    generator = DEFAULT_GENERATOR,
       AllocatorRef allocator = default_allocator);

  List & generator(Generator generator);

  List & axis(Axis axis);

  List & frame(Frame frame);

  List & item_frame(Frame frame);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;
};

// [ ] DEFAULT FOCUS VIEW
// [ ] set the global focus rect, focus view can move there
struct FocusView : View
{
};

// [ ] implement
// - coloring specific rows/columns/cells
// - large columns and rows
struct Table : View
{
};

/// REQUIREMENTS:
/// - Linear and Non-Linear Color Space Independence
/// - Rectangular Box with visualizations
/// - Text-based manual input
/// - RGB, SRGB, HSV, HEX, Linear, Hue, YUV
/// - color space, pixel info for color pickers
struct ColorPicker : View
{
};

/// REQUIREMENTS:
/// - plot modes: histogram, lines, scale, log
/// - plot from user buffer: can be at specific index and will plot rest from
/// head.
/// - buffer size
/// - line size, color, thickness
/// - background size color thickness
/// - show dims on hover (if enabled)
struct Plot : View
{
};

struct ProgressBar : View
{
};

}    // namespace ui
}    // namespace ash
