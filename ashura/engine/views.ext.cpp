/*

ui::State ComboItem::tick(Scope const &, Events const &, Fn<void(View &)>)
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

TextComboItem::TextComboItem(Str8 text, TextStyle const & style, FontStyle const & font,
                             Allocator allocator) :
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

ui::State TextComboItem::tick(Scope const & scope, Events const & events,
                              Fn<void(View &)> build)
{
    if (events.pointer_over() && ctx.mouse.down(MouseButton::Primary) &&
        !ComboItem::state_.selected)
    {
        ComboItem::state_.click_hook(ComboItem::state_.id);
    }

    state_.hovered = events.pointer_over();
    state_.pressed = events.pointer_over() && ctx.mouse.held(MouseButton::Primary);

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

Layout TextComboItem::fit(f32x2 allocated, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    auto frame  = style_.frame(allocated);
    auto padded = sizes[0] + style_.padding.axes();
    frame.x     = max(frame.x, padded.x);
    frame.y     = max(frame.y, padded.y);

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
        item->state_.click_hook = {this, [](Combo * c, usize id) { c->select(id); }};
        item->state_.id         = i;
    }

    items_.append(list).unwrap();
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
        ASH_CHECK(i.v() < size32(items_), "");
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

ui::State Combo::tick(Scope const &, Events const &, Fn<void(View &)> build)
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


Input::Input(Str32 s, TextStyle const & style, FontStyle const & font,
             Allocator allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
    content(U""_s).content_run(style, font).stub(s).stub_run(style, font);
}

Input::Input(Str8 s, TextStyle const & style, FontStyle const & font,
             Allocator allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
    content(U""_s).content_run(style, font).stub(s).stub_run(style, font);
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

Input & Input::content_run(TextStyle const & style, FontStyle const & font, usize first,
                           usize count)
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

Input & Input::stub_run(TextStyle const & style, FontStyle const & font, usize first,
                        usize count)
{
    stub_.run(style, font, first, count);
    return *this;
}

ui::State Input::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    bool edited = false;

    state_.editing = false;
    state_.submit  = false;

    u8 buffer[512];

    Vec<c32> input_u32{allocator};

    if (events.text_input())
    {
        utf8_decode(ctx.key.text, input_u32).unwrap();
    }

    TextCommand cmd = text_command(ctx, events,
                                   TextCfg{.multiline_input = state_.multiline,
                                           .enter_submits   = state_.enter_submits,
                                           .tab_input       = state_.tab_input,
                                           .copyable        = true,
                                           .editable        = true,
                                           .highlightable   = true});

    auto hit_info = events.hit_info.map([](auto s) { return s; }).unwrap_or();

    compositor_.command(content_, cmd, input_u32, engine->clipboard,
                        style_.lines_per_page, style_.tab_width,
                        hit_info.viewport_region.center,
                        hit_info.viewport_region.extent.x, hit_info.canvas_hit,
                        transform2d_to_3d(hit_info.canvas_transform), allocator);

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
      .text =
        TextInputInfo{.multiline = state_.multiline, .tab_input = state_.tab_input},
      .draggable  = !state_.disabled,
      .focusable  = !state_.disabled,
      .grab_focus = events.drag_start()
    };
}

Layout Input::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    content_.layout(allocated.x);
    stub_.layout(allocated.x);
    if (content_.text_.is_empty())
    {
        return {.extent = stub_.layout_.extent};
    }
    return {.extent = content_.layout_.extent};
}

void Input::render(Canvas & canvas, RenderInfo const & info)
{
    if (content_.text_.is_empty())
    {
        // TODO: placeholder overlay when empty. use child view instead
        stub_.render(canvas.text_renderer(), info.viewport_region.center,
                     info.viewport_region.extent.x,
                     transform2d_to_3d(info.canvas_transform), info.clip);
    }
    else
    {
        // TODO: need to draw caret even if line is empty; SET placeholder caret to 0;
        // use place holder when focused
        content_.render(canvas.text_renderer(), info.viewport_region.center,
                        info.viewport_region.extent.x,
                        transform2d_to_3d(info.canvas_transform), info.clip);
    }
}

Cursor Input::cursor(f32x2, f32x2)
{
    return Cursor::Text;
}


List::List(Generator generator, Allocator allocator) :
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

ui::State List::tick(Scope const &, Events const & events, Fn<void(View &)> build)
{
    auto axis = style_.axis == Axis::X ? 0 : 1;

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

    // TODO: ScrollBar: NEED TO GET SIZE INFO

    for (auto & item : state_.items)
    {
        build(*item);
    }

    return ui::State{.scrollable = true, .viewport = true};
}

void List::size(f32x2 allocated, Span<f32x2> sizes)
{
    fill(sizes, style_.item_frame(style_.frame(allocated)));
}

Layout List::fit(f32x2 allocated, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    auto      frame      = style_.frame(allocated);
    f32x2     extent     = {};
    auto axis       = style_.axis == Axis::X ? 0 : 1;
    auto cross_axis = style_.axis == Axis::X ? 1 : 0;

    // Calculate total extent along main axis
    for (auto size : sizes)
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

Radio & Radio::color(u8x4 c)
{
    style_.color = c;
    return *this;
}

Radio & Radio::inner_color(u8x4 c)
{
    style_.inner_color = c;
    return *this;
}

Radio & Radio::inner_hovered_color(u8x4 c)
{
    style_.inner_hovered_color = c;
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

ui::State Radio::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    if (events.pointer_down() || (events.focus_over() && ctx.key.down(KeyCode::Return)))
    {
        state_.value = !state_.value;
        cb.changed(state_.value);
    }

    state_.hovered = events.pointer_over();

    return ui::State{.pointable = !state_.disabled,
                     .clickable = !state_.disabled,
                     .focusable = !state_.disabled};
}

Layout Radio::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    return {.extent = style_.frame(allocated)};
}

void Radio::render(Canvas & canvas, RenderInfo const & info)
{
    canvas.rrect({.area         = info.canvas_region,
                  .corner_radii = style_.corner_radii,
                  .stroke       = 1,
                  .thickness    = f32x2::splat(style_.thickness),
                  .tint         = style_.color,
                  .clip         = info.clip});

    if (state_.value)
    {
        auto inner_extent = info.canvas_region.extent * (state_.hovered ? 0.75F : 0.5F);
        auto inner_color =
          state_.hovered ? style_.inner_hovered_color : style_.inner_color;

        canvas.circle({
          .area{info.canvas_region.center, inner_extent},
          .tint = inner_color,
          .clip = info.clip
        });
    }
}

Cursor Radio::cursor(f32x2, f32x2)
{
    return Cursor::Pointer;
}


ScalarDragBox::ScalarDragBox(TextStyle const & style, FontStyle const & font,
                             Allocator allocator) :
  input_{U""_s, style, font, allocator}
{
    input_.multiline(false).tab_input(false).enter_submits(false);
}

void ScalarDragBox::scalar_parse(Str32 text, ScalarInfo const & spec, Scalar & scalar)
{
    if (text.is_empty())
    {
        return;
    }

    spec.match(
      [&](F32Info const & spec) {
          f32 value      = 0;
          auto [ptr, ec] = fast_float::from_chars(text.pbegin(), text.pend(), value);
          if (ec != std::errc{} || value < spec.min || value > spec.max)
          {
              return;
          }
          scalar = value;
      },
      [&](I32Info const & spec) {
          i32 value      = 0;
          auto [ptr, ec] = fast_float::from_chars(text.pbegin(), text.pend(), value);
          if (ec != std::errc{} || value < spec.min || value > spec.max)
          {
              return;
          }
          scalar = value;
      });
}

void ScalarDragBox::format_()
{
    u8 buffer[1'024];

    sformat(allocator, style_.format, state_.scalar)
      .match([&](auto & text) { input_.content(text.view().as_c8()); },
             [&](auto &) { input_.content(U"[Truncated]"_s); });
}

ScalarDragBox & ScalarDragBox::on_update(Fn<void(Scalar)> fn)
{
    cb.update = fn;
    return *this;
}

ui::State ScalarDragBox::tick(Scope const & scope, Events const & events,
                              Fn<void(View &)> build)
{
    state_.dragging = events.drag_update();

    // TODO: fix input

    if (events.drag_start() &&
        (ctx.key.down(KeyCode::LeftCtrl) || ctx.key.down(KeyCode::RightCtrl)))
    {
        state_.input_mode = !state_.input_mode;
    }

    if (state_.dragging && !state_.input_mode)
    {
        auto      h = events.hit_info.unwrap_or();
        auto t = clamp(unlerp(h.viewport_region.begin().x,
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

void ScalarDragBox::size(f32x2 allocated, Span<f32x2> sizes)
{
    auto child = style_.frame(allocated) - style_.padding.axes();
    child.x    = max(child.x, 0.0F);
    child.y    = max(child.y, 0.0F);
    fill(sizes, child);
}

Layout ScalarDragBox::fit(f32x2 allocated, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    auto frame  = style_.frame(allocated);
    auto padded = sizes[0] + style_.padding.axes();
    frame.x     = max(frame.x, padded.x);
    frame.y     = max(frame.y, padded.y);
    fill(centers, f32x2::zero());

    return {.extent = frame};
}

void ScalarDragBox::render(Canvas & canvas, RenderInfo const & info)
{
    canvas.rrect({.area         = info.canvas_region,
                  .corner_radii = style_.corner_radii,
                  .stroke       = style_.stroke,
                  .thickness    = f32x2::splat(style_.thickness),
                  .tint         = style_.color,
                  .clip         = info.clip});

    if (!state_.input_mode)
    {
        auto t =
          state_.spec.match([&](F32Info & v) { return v.uninterp(state_.scalar[v0]); },
                            [&](I32Info & v) { return v.uninterp(state_.scalar[v1]); });

        auto thumb_rect = CRect::from_offset(
          info.canvas_region.begin(), info.canvas_region.extent * f32x2{t, 1});

        canvas.rrect({.area         = thumb_rect,
                      .corner_radii = style_.corner_radii,
                      .tint         = style_.thumb_color,
                      .clip         = info.clip});
    }
}

Cursor ScalarDragBox::cursor(f32x2, f32x2)
{
    return state_.disabled ? Cursor::Default : Cursor::EWResize;
}

ScalarBox::ScalarBox(Str32 decrease_text, Str32 increase_text,
                     TextStyle const & button_text_style,
                     TextStyle const & drag_text_style, FontStyle const & icon_font,
                     FontStyle const & text_font, Allocator allocator) :
  Flex{allocator},
  dec_{decrease_text, button_text_style, icon_font, allocator},
  inc_{increase_text, button_text_style, icon_font, allocator},
  drag_{drag_text_style, text_font, allocator}
{
    Flex::axis(Axis::X)
      .wrap(false)
      .main_align(MainAlign::Start)
      .cross_align(0)
      .frame(Frame{}.rel({1, 1}));

    padding(Padding::all(5)).corner_radii(CornerRadii::all(7.5F));

    dec_.on_pressed({this, [](ScalarBox * b) { b->step(-1); }});

    inc_.on_pressed({this, [](ScalarBox * b) { b->step(1); }});

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

ScalarBox & ScalarBox::padding(Padding p)
{
    dec_.padding(p);
    inc_.padding(p);
    drag_.style_.padding = p;
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

ScalarBox & ScalarBox::drag_text_style(TextStyle const & style, FontStyle const & font,
                                       usize first, usize count)
{
    drag_.input_.content_run(style, font, first, count)
      .stub_run(style, font, first, count);
    return *this;
}

ui::State ScalarBox::tick(Scope const &, Events const &, Fn<void(View &)> build)
{
    build(dec_);
    build(drag_);
    build(inc_);
    return ui::State{};
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

Switch & Switch::on_color(u8x4 c)
{
    style_.on_color = c;
    return *this;
}

Switch & Switch::on_hovered_color(u8x4 c)
{
    style_.on_hovered_color = c;
    return *this;
}

Switch & Switch::off_color(u8x4 c)
{
    style_.off_color = c;
    return *this;
}

Switch & Switch::off_hovered_color(u8x4 c)
{
    style_.off_hovered_color = c;
    return *this;
}

Switch & Switch::track_color(u8x4 c)
{
    style_.track_color = c;
    return *this;
}

Switch & Switch::corner_radii(CornerRadii const & r)
{
    style_.corner_radii = r;
    return *this;
}

Switch & Switch::frame(Frame f)
{
    style_.frame = f;
    return *this;
}

ui::State Switch::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    if (events.pointer_down() || (events.focus_over() && ctx.key.down(KeyCode::Return)))
    {
        state_.value = !state_.value;
        cb.changed(state_.value);
    }

    state_.hovered = events.pointer_over();

    return ui::State{.pointable = !state_.disabled,
                     .clickable = !state_.disabled,
                     .focusable = !state_.disabled};
}

Layout Switch::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    return {.extent = style_.frame(allocated)};
}

void Switch::render(Canvas & canvas, RenderInfo const & info)
{
    auto thumb_extent = info.canvas_region.extent;
    thumb_extent.x *= 0.5F;
    f32x2 const alignment{state_.value ? ALIGNMENT_RIGHT : ALIGNMENT_LEFT,
                          ALIGNMENT_CENTER};
    auto thumb_center = info.canvas_region.center +
                        space_align(info.canvas_region.extent, thumb_extent, alignment);

    u8x4 thumb_color;
    if (state_.hovered)
    {
        thumb_color = state_.value ? style_.on_hovered_color : style_.off_hovered_color;
    }
    else
    {
        thumb_color = state_.value ? style_.on_color : style_.off_color;
    }

    canvas
      .rrect({
        .area         = info.canvas_region,
        .corner_radii = style_.corner_radii,
        .tint         = style_.track_color,
        .clip         = info.clip
    })
      .rrect({.area{thumb_center, thumb_extent},
              .corner_radii = style_.corner_radii,
              .tint         = thumb_color,
              .clip         = info.clip});
}

Cursor Switch::cursor(f32x2, f32x2)
{
    return state_.disabled ? Cursor::Default : Cursor::Pointer;
}
 */
