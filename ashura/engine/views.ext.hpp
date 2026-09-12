/*



struct ComboItem : View
{
    struct State
    {
        bool disabled : 1 = false;

        bool selected : 1 = false;

        bool hovered : 1 = false;

        bool pressed : 1 = false;

        auto id = 0uz;

        Fn<void(usize)> click_hook = noop;
    } state_;

    ComboItem()                              = default;
    ComboItem(ComboItem const &)             = delete;
    ComboItem(ComboItem &&)                  = default;
    ComboItem & operator=(ComboItem const &) = delete;
    ComboItem & operator=(ComboItem &&)      = default;
    virtual ~ComboItem() override            = default;

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;

    virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

struct TextComboItem : ComboItem
{
    struct Style
    {
        Frame frame = Frame{}.abs(1, 1);

        Padding padding = Padding::all(5);

        f32 alignment = ALIGNMENT_LEFT;

        u8x4 color = theme.surface_variant;

        u8x4 hover_color = theme.primary_variant;

        u8x4 selected_color = theme.primary;

        f32 stroke = 0;

        f32 thickness = 1;

        CornerRadii corner_radii = CornerRadii::all(2);
    } style_;

    Text text_;

    TextComboItem(Str32             text,
                  TextStyle const & style = TextStyle{.color = theme.on_surface},
                  FontStyle const & font  = FontStyle{.font   = theme.body_font,
                                                      .height = theme.body_font_height,
                                                      .line_height = theme.line_height},
                  Allocator         allocator = default_allocator);

    TextComboItem(Str8              text,
                  TextStyle const & style = TextStyle{.color = theme.on_surface},
                  FontStyle const & font  = FontStyle{.font   = theme.body_font,
                                                      .height = theme.body_font_height,
                                                      .line_height = theme.line_height},
                  Allocator         allocator = default_allocator);

    TextComboItem(TextComboItem const &)             = delete;
    TextComboItem(TextComboItem &&)                  = default;
    TextComboItem & operator=(TextComboItem const &) = delete;
    TextComboItem & operator=(TextComboItem &&)      = default;
    virtual ~TextComboItem() override                = default;

    TextComboItem & frame(Frame frame);

    TextComboItem & padding(Padding padding);

    TextComboItem & align(f32 alignment);

    TextComboItem & color(u8x4 color);

    TextComboItem & hover_color(u8x4 color);

    TextComboItem & selected_color(u8x4 color);

    TextComboItem & stroke(f32 stroke);

    TextComboItem & thickness(f32 thickness);

    TextComboItem & corner_radii(CornerRadii radii);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;
};

struct Combo : Flex
{
    struct State
    {
        bool disabled : 1 = false;

        Option<usize> selected = none;
    } state_;

    struct Style
    {
        CornerRadii corner_radii = CornerRadii::all(2);

        u8x4 color = theme.surface;

        f32 stroke = 0;

        f32 thickness = 1;
    } style_;

    struct Callbacks
    {
        Fn<void(Option<usize>)> selected = noop;
    } cb;

    Vec<ref<ComboItem>> items_;

    Combo(Allocator allocator = default_allocator);
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

    Combo & frame(Frame frame);

    Combo & item_frame(Frame frame);

    Combo & disable(bool disable);

    Combo & color(u8x4 color);

    Combo & corner_radii(CornerRadii radii);

    Combo & on_selected(Fn<void(Option<usize>)> style);

    Combo & items(InitList<ref<ComboItem>> list);

    Combo & items(Span<ref<ComboItem> const> list);

    usize num_items() const;

    Combo & select(Option<usize> item);

    Option<usize> get_selection() const;

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;
};

// TODO: scroll and clip text if region isn't large enough
// -  wrapping to the next line if not large enough
// -  no wrap
// -  max-len
// -  filter/transform function
// -  secret text input
struct InputCfg
{
    bool wrappable     : 1 = false;
    bool submittable   : 1 = false;
    bool multiline     : 1 = false;
    bool enter_submits : 1 = false;
    bool tab_input     : 1 = false;

    Fn<void(Vec<c32> &, Str32)> insert;
};

// TODO: renderer hooks for regions
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
    } state_;

    struct Style
    {
        TextHighlightStyle highlight = {.color        = theme.highlight,
                                        .corner_radii = f32x4::splat(0)};
        CaretStyle         caret{.color = theme.caret, .thickness = 1.0F};
        usize              lines_per_page = 40;
        usize              tab_width      = 1;
    } style_;

    struct Callbacks
    {
        Fn<void()> edit = noop;

        Fn<void()> submit = noop;

        Fn<void()> focus_in = noop;

        Fn<void()> focus_out = noop;
    } cb;

    Allocator allocator_;

    RenderText content_;

    RenderText stub_;

    TextCompositor compositor_;

    Input(Str32             stub      = U""_s,
          TextStyle const & style     = TextStyle{.color = theme.on_surface},
          FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                                  .height      = theme.body_font_height,
                                                  .line_height = theme.line_height},
          Allocator         allocator = default_allocator);

    Input(Str8 stub, TextStyle const & style = TextStyle{.color = theme.on_surface},
          FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                                  .height      = theme.body_font_height,
                                                  .line_height = theme.line_height},
          Allocator         allocator = default_allocator);

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
                        auto first = 0uz, usize count = USIZE_MAX);

    Input & stub(Str8 text);

    Input & stub(Str32 text);

    Input & stub_run(TextStyle const & style, FontStyle const & font, auto first = 0uz,
                     usize count = USIZE_MAX);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;

    virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

// TODO: segmentation
// TODO: measure function
/// @brief An infinitely scrollable List of elements.
struct List : View
{
    typedef Fn<Option<Dyn<View *>>(Allocator, usize i)> Generator;

    static constexpr auto DEFAULT_GENERATOR =
      [](Allocator, usize) -> Option<Dyn<View *>> { return none; };

    struct State
    {
        /// @brief Effective translation of the entire list
        f32 total_translation = 0;

        /// @brief The view extent of the viewport
        f32 view_extent = 0;

        /// @brief The first of the currently active subset
        usize first_item = 0;

        /// @brief Determined upper bound
        usize max_count = USIZE_MAX;

        usize num_loaded = 0;

        Option<f32> item_size = none;

        /// @brief The item generator
        Generator generator = DEFAULT_GENERATOR;

        Vec<Dyn<View *>> items;

        Slice range() const
        {
            return Slice{first_item, items.size()};
        }

        Option<Slice> visible() const
        {
            return item_size.map([&](f32 s) {
                auto first =
                  static_cast<usize>(std::abs(std::floor((-total_translation) / s)));
                auto count = static_cast<usize>(std::abs(std::ceil(view_extent / s)));
                return Slice{first, count};
            });
        }

    } state_;

    struct Style
    {
        Axis axis = Axis::X;

        Frame frame = Frame{}.abs(1, 1);

        Frame item_frame = Frame{}.abs(1, 1);
    } style_;

    Allocator allocator_;

    List(Generator generator = DEFAULT_GENERATOR,
         Allocator allocator = default_allocator);

    List & generator(Generator generator);

    List & axis(Axis axis);

    List & frame(Frame frame);

    List & item_frame(Frame frame);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;
};

// TODO: implement
struct Modal;


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

// TODO: replicate Checkbox and use font
struct Radio : View
{
    struct State
    {
        bool disabled : 1 = false;

        bool hovered : 1 = false;

        bool value : 1 = false;
    } state_;

    struct Style
    {
        Frame frame = Frame{}.abs(20, 20);

        CornerRadii corner_radii = CornerRadii::all(0.5);

        f32 thickness = 0.5F;

        u8x4 color = theme.inactive;

        u8x4 inner_color = theme.primary;

        u8x4 inner_hovered_color = theme.primary_variant;
    } style_;

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

    Radio & color(u8x4 color);

    Radio & inner_color(u8x4 color);

    Radio & inner_hovered_color(u8x4 color);

    Radio & frame(Frame frame);

    Radio & on_changed(Fn<void(bool)> fn);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;

    virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

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

    f32 step = 0.05F;

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

// TODO: on-focused go to input mode
// TODO: fix alt+click
struct ScalarDragBox : View
{
    struct State
    {
        bool disabled : 1 = false;

        bool input_mode : 1 = false;

        bool dragging : 1 = false;

        ScalarInfo spec = F32Info{};

        Scalar scalar = 0.0F;
    } state_;

    struct Style
    {
        Frame frame = Frame{}.min(200, theme.body_font_height);

        Padding padding = Padding::all(2.5F);

        CornerRadii corner_radii = CornerRadii::all(2);

        u8x4 color = theme.inactive;

        u8x4 thumb_color = theme.inactive;

        f32 stroke = 1.0F;

        f32 thickness = 0.5F;

        Str format = "{}"_s;
    } style_;

    Input input_;

    struct Callbacks
    {
        Fn<void(Scalar)> update = noop;
    } cb;

    ScalarDragBox(TextStyle const & style = TextStyle{.color = theme.on_surface},
                  FontStyle const & font  = FontStyle{.font   = theme.body_font,
                                                      .height = theme.body_font_height,
                                                      .line_height = theme.line_height},
                  Allocator         allocator = default_allocator);

    ScalarDragBox(ScalarDragBox const &)             = delete;
    ScalarDragBox(ScalarDragBox &&)                  = default;
    ScalarDragBox & operator=(ScalarDragBox const &) = delete;
    ScalarDragBox & operator=(ScalarDragBox &&)      = default;
    virtual ~ScalarDragBox() override                = default;

    static void scalar_parse(Str32 text, ScalarInfo const & spec, Scalar &);

    void format_();

    ScalarDragBox & on_update(Fn<void(Scalar)> cb);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;

    virtual Cursor cursor(f32x2 extent, f32x2 offset) override;
};

// TODO: spacing / margin
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
      Str32 decrease_text = U"minus"_s, Str32 increase_text = U"plus"_s,
      TextStyle const & button_text_style = TextStyle{.color = theme.on_primary},
      TextStyle const & drag_text_style   = TextStyle{.color = theme.on_primary},
      FontStyle const & icon_font         = FontStyle{.font        = theme.icon_font,
                                                      .height      = theme.body_font_height,
                                                      .line_height = theme.line_height},
      FontStyle const & text_font         = FontStyle{.font        = theme.body_font,
                                                      .height      = theme.body_font_height,
                                                      .line_height = theme.line_height},
      Allocator         allocator         = default_allocator);

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

    ScalarBox & padding(Padding padding);

    ScalarBox & frame(Frame frame);

    ScalarBox & corner_radii(CornerRadii const & radii);

    ScalarBox & on_update(Fn<void(Scalar)> fn);

    ScalarBox & button_text_style(TextStyle const & style, FontStyle const & font,
                                  auto first = 0uz, usize count = USIZE_MAX);

    ScalarBox & drag_text_style(TextStyle const & style, FontStyle const & font,
                                auto first = 0uz, usize count = USIZE_MAX);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;
};


struct Switch : View
{
    struct State
    {
        bool disabled : 1 = false;

        bool hovered : 1 = false;

        bool value : 1 = false;
    } state_;

    struct Style
    {
        u8x4 on_color = theme.primary;

        u8x4 on_hovered_color = theme.primary_variant;

        u8x4 off_color = theme.active;

        u8x4 off_hovered_color = theme.inactive;

        u8x4 track_color = theme.surface_variant;

        f32 track_thickness = 1;

        f32 track_stroke = 0;

        CornerRadii corner_radii = CornerRadii::all(4);

        Frame frame = Frame{}.abs(40, 20);
    } style_;

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

    Switch & on_color(u8x4 color);

    Switch & on_hovered_color(u8x4 color);

    Switch & off_color(u8x4 color);

    Switch & off_hovered_color(u8x4 color);

    Switch & track_color(u8x4 color);

    Switch & corner_radii(CornerRadii const & radii);

    Switch & frame(Frame frame);

    Switch & thumb_frame(Frame frame);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;

    virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

// TODO: implement
// - coloring specific rows/columns/cells
// - large columns and rows
struct Table : View
{
};

*/
