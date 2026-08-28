/*
Button & Button::disable(bool d)
{
    state_.disabled = d;
    return *this;
}

Button & Button::color(u8x4 c)
{
    style_.color = c;
    return *this;
}

Button & Button::hovered_color(u8x4 c)
{
    style_.hovered_color = c;
    return *this;
}

Button & Button::disabled_color(u8x4 c)
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


TextButton::TextButton(Str32 text, TextStyle const & style, FontStyle const & font,
                       Allocator allocator) :
  text_{text, style, font, allocator}
{
}

TextButton::TextButton(Str8 text, TextStyle const & style, FontStyle const & font,
                       Allocator allocator) :
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

TextButton & TextButton::color(u8x4 c)
{
    Button::color(c);
    return *this;
}

TextButton & TextButton::hovered_color(u8x4 c)
{
    Button::color(c);
    return *this;
}

TextButton & TextButton::disabled_color(u8x4 c)
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

TextButton & TextButton::padding(Padding p)
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

ui::State TextButton::tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build)
{
    ui::State state_ = Button::tick(ctx, events, build);
    build(text_);
    return state_;
}


CheckBox::CheckBox(Str32 text, TextStyle const & style, FontStyle const & font,
                   Allocator allocator) :
  icon_{text, style, font, allocator}
{
}

CheckBox::CheckBox(Str8 text, TextStyle const & style, FontStyle const & font,
                   Allocator allocator) :
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

CheckBox & CheckBox::box_color(u8x4 c)
{
    style_.box_color = c;
    return *this;
}

CheckBox & CheckBox::box_hovered_color(u8x4 c)
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

CheckBox & CheckBox::on_changed(Fn<void(bool)> f)
{
    cb.changed = f;
    return *this;
}

ui::State CheckBox::tick(Scope const & scope, Events const & events,
                         Fn<void(View &)> build)
{
    if (events.pointer_down() || (events.focus_over() && ctx.key.down(KeyCode::Return)))
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

void CheckBox::size(f32x2 allocated, Span<f32x2> sizes)
{
    fill(sizes, allocated);
}

Layout CheckBox::fit(f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    fill(centers, f32x2{});
    return {.extent = style_.frame(sizes[0])};
}

void CheckBox::render(Canvas & canvas, RenderInfo const & info)
{
    u8x4 tint;
    if (state_.hovered && !state_.held && !state_.disabled)
    {
        tint = style_.box_hovered_color;
    }
    else
    {
        tint = style_.box_color;
    }

    canvas.rrect({.area         = info.canvas_region,
                  .corner_radii = style_.corner_radii,
                  .stroke       = 1,
                  .thickness    = f32x2::splat(style_.thickness),
                  .tint         = tint,
                  .clip         = info.clip});
}

Cursor CheckBox::cursor(f32x2, f32x2)
{
    return state_.disabled ? Cursor::Default : Cursor::Pointer;
}


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


Icon::Icon(Str32 text, TextStyle const & style, FontStyle const & font,
           Allocator allocator) :
  text_{allocator}
{
    text_.text(text).run(style, font);
}

Icon::Icon(Str8 text, TextStyle const & style, FontStyle const & font,
           Allocator allocator) :
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

ui::State Icon::tick(Scope const &, Events const &, Fn<void(View &)>)
{
    return ui::State{.hidden = state_.hidden};
}

Layout Icon::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    text_.layout(allocated.x);
    return Layout{.extent = text_.get_layout().extent};
}

void Icon::render(Canvas & canvas, RenderInfo const & info)
{
    text_.render(canvas.text_renderer(), info.viewport_region.center,
                 info.viewport_region.extent.x,
                 transform2d_to_3d(info.canvas_transform), info.clip);
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
    fill(centers, f32x2{0, 0});

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


ScrollBar::ScrollBar() : style_{}
{
}

ScrollBar & ScrollBar::center(f32 v)
{
    state_.scroll.center(v);
    return *this;
}

ScrollBar & ScrollBar::delta(f32 v)
{
    state_.scroll.delta(v);
    return *this;
}

ScrollBar & ScrollBar::extent(f32 visible, f32 content, f32 track)
{
    state_.scroll.extent(visible, content, track);
    return *this;
}

ScrollBar & ScrollBar::thickness(f32 t)
{
    style_.thickness = t;
    return *this;
}

ScrollBar & ScrollBar::disable(bool d)
{
    state_.disabled = d;
    return *this;
}

ScrollBar & ScrollBar::thumb_color(u8x4 color)
{
    style_.thumb_color = color;
    return *this;
}

ScrollBar & ScrollBar::thumb_hovered_color(u8x4 color)
{
    style_.thumb_hovered_color = color;
    return *this;
}

ScrollBar & ScrollBar::thumb_dragging_color(u8x4 color)
{
    style_.thumb_dragging_color = color;
    return *this;
}

ScrollBar & ScrollBar::thumb_corner_radii(CornerRadii const & c)
{
    style_.thumb_corner_radii = c;
    return *this;
}

ScrollBar & ScrollBar::track_color(u8x4 color)
{
    style_.track_color = color;
    return *this;
}

ScrollBar & ScrollBar::track_corner_radii(CornerRadii const & c)
{
    style_.track_corner_radii = c;
    return *this;
}

ScrollBar & ScrollBar::axis(Axis axis)
{
    style_.axis = axis;
    return *this;
}

ui::State ScrollBar::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    auto main_axis = (style_.axis == Axis::X) ? 0 : 1;

    if (events.drag_update())
    {
        // TODO: +- nan,  +- inf
        // TODO: center is relative to the content extent
        auto h     = events.hit_info.unwrap_or();
        auto begin = h.viewport_region.begin()[main_axis];
        auto end   = h.viewport_region.end()[main_axis];
        auto scale =
          h.viewport_region.extent[main_axis] / state_.scroll.content_extent();
        auto thumb_extent = scale * state_.scroll.visible_extent();
        auto track_begin  = begin + 0.5F * thumb_extent;
        auto track_end    = end - 0.5F * thumb_extent;
        auto thumb_pos    = clamp(h.viewport_hit[main_axis], track_begin, track_end);
        auto t            = unlerp(track_begin, track_end, thumb_pos);
        state_.scroll.center(lerp(
          0.0F, state_.scroll.content_extent() - state_.scroll.visible_extent(), t));
    }

    if (events.focus_over())
    {
        if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Left)) ||
            (style_.axis == Axis::Y && ctx.key.down(KeyCode::Up)))
        {
            state_.scroll.center(clamp(
              state_.scroll.center() -
                state_.scroll.delta() * state_.scroll.visible_extent(),
              0.0F, state_.scroll.content_extent() - state_.scroll.visible_extent()));
        }
        else if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Right)) ||
                 (style_.axis == Axis::Y && ctx.key.down(KeyCode::Down)))
        {
            state_.scroll.center(clamp(
              state_.scroll.center() +
                state_.scroll.delta() * state_.scroll.visible_extent(),
              0.0F, state_.scroll.content_extent() - state_.scroll.visible_extent()));
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

Layout ScrollBar::fit(f32x2, Span<f32x2 const>, Span<f32x2>)
{
    auto main_axis  = (style_.axis == Axis::X) ? 0 : 1;
    auto cross_axis = (style_.axis == Axis::X) ? 1 : 0;

    f32x2 size;

    size[main_axis]  = state_.scroll.track_extent();
    size[cross_axis] = style_.thickness;

    return {.extent = size};
}

void ScrollBar::render(Canvas & canvas, RenderInfo const & info)
{
    auto main_axis  = (style_.axis == Axis::X) ? 0 : 1;
    auto cross_axis = (style_.axis == Axis::X) ? 1 : 0;

    // TODO: nan, inf
    auto scale = info.canvas_region.extent[main_axis] / state_.scroll.content_extent();
    auto thumb_extent = state_.scroll.visible_extent() * scale;
    auto t =
      unlerp(0.0F, state_.scroll.content_extent() - state_.scroll.visible_extent(),
             state_.scroll.center());
    auto thumb_center = info.canvas_region.begin()[main_axis] +
                             0.5F * thumb_extent +
                             t * (info.canvas_region.extent[main_axis] - thumb_extent);

    CRect thumb_rect;

    thumb_rect.center[main_axis]  = thumb_center;
    thumb_rect.center[cross_axis] = info.canvas_region.center[cross_axis];
    thumb_rect.extent[main_axis]  = thumb_extent;
    thumb_rect.extent[cross_axis] = info.canvas_region.extent[cross_axis];

    u8x4 thumb_color;
    u8x4 track_color = style_.track_color;

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
      .rrect({.area         = info.canvas_region,
              .corner_radii = style_.track_corner_radii,
              .stroke       = 0,
              .tint         = track_color,
              .clip         = info.clip})
      .rrect({.area         = thumb_rect,
              .corner_radii = style_.thumb_corner_radii,
              .stroke       = 0,
              .tint         = thumb_color,
              .clip         = info.clip});
}

ScrollContent::ScrollContent(View & child) : child_{child}
{
}

ScrollContent & ScrollContent::frame(Frame f)
{
    style_.frame = f;
    return *this;
}

ui::State ScrollContent::tick(Scope const &, Events const &, Fn<void(View &)> build)
{
    build(child_);
    return ui::State{};
}

void ScrollContent::size(f32x2 allocated, Span<f32x2> sizes)
{
    sizes[0] = style_.frame(allocated);
}

Layout ScrollContent::fit(f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    centers[0] = f32x2::splat(0);
    return {.extent = sizes[0]};
}

ScrollPort::ScrollPort(View & child) : content_{child}
{
}

ScrollPort & ScrollPort::frame(Frame f)
{
    style_.frame = f;
    return *this;
}

ui::State ScrollPort::tick(Scope const &, Events const &, Fn<void(View &)> build)
{
    build(content_);
    return ui::State{.viewport = true};
}

void ScrollPort::size(f32x2 allocated, Span<f32x2> sizes)
{
    fill(sizes, allocated);
}

Layout ScrollPort::fit(f32x2 allocated, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    centers[0]          = f32x2::splat(0);
    auto content_extent = sizes[0];
    auto visible_extent = style_.frame(allocated);

    state_.content_extent = content_extent;
    state_.visible_extent = visible_extent;

    return {.extent          = visible_extent,
            .viewport_extent = content_extent,
            .viewport_center = state_.center,
            .viewport_zoom   = state_.zoom};
}

ScrollView::ScrollView(View & child) : x_bar_{}, y_bar_{}, port_{child}
{
    x_bar_.axis(Axis::X);
    y_bar_.axis(Axis::Y);
}

ScrollView & ScrollView::disable(bool d)
{
    x_bar_.disable(d);
    y_bar_.disable(d);
    return *this;
}

ScrollView & ScrollView::item(View & v)
{
    port_.content_.child_ = v;
    return *this;
}

ScrollView & ScrollView::thumb_color(u8x4 c)
{
    x_bar_.thumb_color(c);
    y_bar_.thumb_color(c);
    return *this;
}

ScrollView & ScrollView::thumb_hovered_color(u8x4 c)
{
    x_bar_.thumb_hovered_color(c);
    y_bar_.thumb_hovered_color(c);
    return *this;
}

ScrollView & ScrollView::thumb_dragging_color(u8x4 c)
{
    x_bar_.thumb_dragging_color(c);
    y_bar_.thumb_dragging_color(c);
    return *this;
}

ScrollView & ScrollView::thumb_corner_radii(CornerRadii const & c)
{
    x_bar_.thumb_corner_radii(c);
    y_bar_.thumb_corner_radii(c);
    return *this;
}

ScrollView & ScrollView::track_color(u8x4 c)
{
    x_bar_.track_color(c);
    y_bar_.track_color(c);
    return *this;
}

ScrollView & ScrollView::track_corner_radii(CornerRadii const & c)
{
    x_bar_.track_corner_radii(c);
    y_bar_.track_corner_radii(c);
    return *this;
}

ScrollView & ScrollView::axes(Axes a)
{
    x_bar_.state_.hidden = has_bits(a, Axes::X);
    y_bar_.state_.hidden = has_bits(a, Axes::Y);
    return *this;
}

ScrollView & ScrollView::view_frame(Frame f)
{
    port_.style_.frame = f;
    return *this;
}

ScrollView & ScrollView::content_frame(Frame f)
{
    port_.content_.frame(f);
    return *this;
}

ScrollView & ScrollView::bar_thickness(f32 x, f32 y)
{
    x_bar_.thickness(x);
    y_bar_.thickness(y);
    return *this;
}

ui::State ScrollView::tick(Scope const &, Events const & events, Fn<void(View &)> build)
{
    f32 y_bar_nudge = 0;

    if (!x_bar_.state_.disabled && !y_bar_.state_.disabled)
    {
        // prevent overlap of the bars
        y_bar_nudge = x_bar_.style_.thickness + x_bar_.style_.nudge;
    }

    x_bar_.extent(port_.state_.visible_extent.x, port_.state_.content_extent.x,
                  port_.state_.visible_extent.x);
    y_bar_.extent(port_.state_.visible_extent.y, port_.state_.content_extent.y,
                  max(port_.state_.visible_extent.y - y_bar_nudge, 0.0F));

    if (events.scroll())
    {
        auto scroll = events.scroll_info.unwrap();

        if (!x_bar_.state_.disabled)
        {
            x_bar_.state_.scroll.center(scroll.center.x);
        }

        if (!y_bar_.state_.disabled)
        {
            y_bar_.state_.scroll.center(scroll.center.y);
        }
    }

    // TODO: remove
    port_.state_.center = f32x2{0, -700} + f32x2{x_bar_.state_.scroll.center(),
                                                 y_bar_.state_.scroll.center()};

    build(x_bar_);
    build(y_bar_);
    build(port_);

    return ui::State{.scrollable = !(x_bar_.state_.disabled && y_bar_.state_.disabled)};
}

void ScrollView::size(f32x2 allocated, Span<f32x2> sizes)
{
    // TODO: the barsd will have invalid extents
    fill(sizes, allocated);
}

Layout ScrollView::fit(f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    centers[0] =
      space_align(port_.state_.visible_extent, sizes[0], ALIGNMENT_BOTTOM_LEFT);
    centers[1] =
      space_align(port_.state_.visible_extent, sizes[1], ALIGNMENT_TOP_RIGHT);
    centers[2] = {0, 0};

    return {.extent = port_.state_.visible_extent};
}

i32 ScrollView::layer(i32 allocated, Span<i32> layers)
{
    // needs to be at a different stacking context since this will be placed
    // on top of the viewport
    layers[0] = LAYERS.viewport_bars;
    layers[1] = LAYERS.viewport_bars;
    layers[2] = allocated;
    return allocated;
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

Slider & Slider::thumb_color(u8x4 c)
{
    style_.thumb_color = c;
    return *this;
}

Slider & Slider::thumb_hovered_color(u8x4 c)
{
    style_.thumb_hovered_color = c;
    return *this;
}

Slider & Slider::thumb_dragging_color(u8x4 c)
{
    style_.thumb_dragging_color = c;
    return *this;
}

Slider & Slider::thumb_corner_radii(CornerRadii const & c)
{
    style_.thumb_corner_radii = c;
    return *this;
}

Slider & Slider::track_color(u8x4 c)
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

ui::State Slider::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    auto main_axis = (style_.axis == Axis::X) ? 0 : 1;

    if (events.drag_update())
    {
        auto      h = events.hit_info.unwrap_or();
        auto thumb_begin =
          h.viewport_region.begin()[main_axis] + style_.thumb_size * 0.5F;
        auto thumb_end =
          h.viewport_region.end()[main_axis] - style_.thumb_size * 0.5F;
        state_.t =
          clamp(unlerp(thumb_begin, thumb_end, h.viewport_hit[main_axis]), 0.0F, 1.0F);
        auto value =
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

Layout Slider::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    return {.extent = style_.frame(allocated)};
}

void Slider::render(Canvas & canvas, RenderInfo const & info)
{
    auto main_axis  = (style_.axis == Axis::X) ? 0 : 1;
    auto cross_axis = (style_.axis == Axis::Y) ? 0 : 1;

    u8x4 thumb_color;

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

    auto thumb_begin =
      info.canvas_region.begin()[main_axis] + style_.thumb_size * 0.5F;
    auto thumb_end =
      info.canvas_region.end()[main_axis] - style_.thumb_size * 0.5F;
    auto thumb_center = lerp(thumb_begin, thumb_end, state_.t);

    CRect thumb_rect;

    thumb_rect.center[main_axis]  = thumb_center;
    thumb_rect.center[cross_axis] = info.canvas_region.center[cross_axis];
    thumb_rect.extent             = f32x2::splat(style_.thumb_size);

    CRect track_rect;

    track_rect.center             = info.canvas_region.center;
    track_rect.extent[main_axis]  = thumb_end - thumb_begin;
    track_rect.extent[cross_axis] = style_.track_size;

    f32x2 coverage_begin;
    coverage_begin[main_axis]  = thumb_begin;
    coverage_begin[cross_axis] = track_rect.begin()[cross_axis];

    f32x2 coverage_end;
    coverage_end[main_axis]  = thumb_center;
    coverage_end[cross_axis] = track_rect.end()[cross_axis];

    auto coverage_rect = CRect::range(coverage_begin, coverage_end);

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

Cursor Slider::cursor(f32x2, f32x2)
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
