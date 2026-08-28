/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_text.hpp"
#include "ashura/engine/text.hpp"
#include "ashura/engine/view.hpp"
#include "ashura/std/types.hpp"

namespace ash
{
namespace ui
{

struct Shadow
{
    /// @brief The offset of the shadow from the view
    f32x2 offset = f32x2{1, 1};

    /// @brief The feather of the shadow
    f32 feather = 1;

    /// @brief The spread of the shadow
    f32 spread = 0;

    /// @brief The tint of the shadow
    ColorGradient tint = colors::BLACK;
};

/// @brief A space that occupies a given extent.
/// This is useful for creating padding or spacing between other views.
struct Space : View
{
    struct Style_
    {
        Frame frame{};
    };

    Style_ style_;

    /// @brief Constructs a blank space view with a given frame
    /// @param frame the frame of the space
    Space(Frame frame);

    /// @brief Constructs a blank space view with a given extent
    /// @param extent the extent of the space
    Space(f32x2 extent = f32x2{0, 0});

    Space(Space const &)             = delete;
    Space(Space &&)                  = default;
    Space & operator=(Space const &) = delete;
    Space & operator=(Space &&)      = default;
    virtual ~Space() override        = default;

    /// @brief Sets the frame of the space
    /// @param frame the frame of the space
    Space & frame(Frame frame);

    /// @brief Sets the extent of the space
    /// @param extent the extent of the space
    Space & extent(f32x2 extent);

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;
};

/// @brief A text view that can display and edit text
struct Text : View
{
    enum class CursorActionType : u8
    {
        None              = 0,
        Unselect          = 1,
        Escape            = 2,
        SelectLeft        = 3,
        SelectRight       = 4,
        SelectUp          = 5,
        SelectDown        = 6,
        SelectToLineStart = 7,
        SelectToLineEnd   = 8,
        SelectPageUp      = 9,
        SelectPageDown    = 10,
        SelectAll         = 11,
        SelectWord        = 12,
        SelectLine        = 13,
        Left              = 14,
        Right             = 15,
        LineStart         = 16,
        LineEnd           = 17,
        Up                = 18,
        Down              = 19,
        PageUp            = 20,
        PageDown          = 21,
        Insert            = 22,
        Cut               = 23,
        Copy              = 24,
        Paste             = 25,
        NewLine           = 26,
        Tab               = 27,
        Backspace         = 28,
        Delete            = 29,
        Home              = 30,
        End               = 31,
        Hit               = 32,
        HitSelectSpan     = 33
    };

    struct CursorAction
    {
        CursorActionType type = CursorActionType::None;

        Rc<Str32> text = static_rc(U""_s);

        f32x2 center = f32x2::zero();

        f32x4x4 transform = f32x4x4::identity();

        f32x2 transformed_position = f32x2::zero();
    };

    enum class CoreActionType : u8
    {
        None        = 0,
        ReplaceText = 1,
        Undo        = 2,
        Redo        = 3,
        Submit      = 4
    };

    struct CoreAction
    {
        CoreActionType type = CoreActionType::None;

        Option<RenderText> text = none;
    };

    using Action = Enum<CursorAction, CoreAction, None>;

    struct Cfg;
    struct State;

    using InputToActionsMap =
      Fn<Vec<Action>(ui::Scope const &, Cfg const &, Events const &, Allocator)>;

    using Renderer =
      Fn<void(ui::Scope const &, Cfg const &, State const &, TextRenderInfo const &,
              TextPlacementInfo const &, f32x2, Canvas)>;

    struct Cfg
    {
        static void default_clipboard_setter(Str32 str);

        static StrVec32 default_clipboard_getter(Allocator allocator);

        static Vec<Action> default_input_to_actions_map(ui::Scope const & scope,
                                                        Cfg const &       cfg,
                                                        Events const &    events,
                                                        Allocator         allocator);

        static void default_renderer(ui::Scope const & scope, Cfg const & cfg,
                                     State const & s, TextRenderInfo const & info,
                                     TextPlacementInfo const & placement, f32x2 center,
                                     Canvas canvas);

        bool copyable : 1 = false;

        bool highlightable : 1 = false;

        bool enable_cursor : 1 = false;

        bool editable : 1 = false;

        bool enable_undo_redo : 1 = false;

        bool enable_multiline_input : 1 = false;

        bool accept_tab_input : 1 = true;

        bool enter_submits : 1 = false;

        u16 lines_per_page = 20;

        Rc<Fn<void(Str32)>> clipboard_setter{default_clipboard_setter, rc_noop};

        Rc<Fn<StrVec32(Allocator)>> clipboard_getter{default_clipboard_getter, rc_noop};

        Rc<Fn<void(Str32)>> on_submit{noop, rc_noop};

        Rc<InputToActionsMap> input_to_actions_map{default_input_to_actions_map,
                                                   rc_noop};

        Rc<Renderer> renderer{default_renderer, rc_noop};
    };

    /// @brief Handles interaction and state updates for text views
    /// It will issue actions that the text view can then execute to update its
    /// internal state
    struct State
    {
        Allocator allocator_;

        TextModel text_;

        Vec<Slice> highlights_;

        Vec<TextHighlightStyle> highlight_styles_;

        State(Allocator allocator) :
          allocator_{allocator},
          text_{allocator},
          highlights_{allocator},
          highlight_styles_{allocator}
        {
        }

        void tick(ui::Scope const & scope, Cfg const & cfg, Events const & events);

        ui::Layout fit(ui::Scope const & scope, f32x2 allocated,
                       Span<f32x2 const> sizes, Span<f32x2> center);

        void render(ui::Scope const & scope, Cfg const & cfg, Canvas canvas,
                    ui::RenderInfo const & info);
    };

    Allocator allocator_;
    State     state_;
    Cfg       cfg_;
    Frame     frame_;

    explicit Text(Allocator allocator);

    /// @brief Constructs a text view with the given allocator, UTF-32 text, style, and font
    /// @param allocator The allocator to use for memory management
    /// @param text The text to display in the view
    /// @param style The text style to apply to the text
    /// @param font The font style to apply to the text
    Text(Allocator allocator, Rc<Str32> text, TextStyle const & style,
         FontStyle const & font);

    /// @brief Constructs a text view with the given allocator, UTF-32 text view, style, and font
    /// @param allocator The allocator to use for memory management
    /// @param text The text to display in the view
    /// @param style The text style to apply to the text
    /// @param font The font style to apply to the text
    Text(Allocator allocator, Str32 text, TextStyle const & style,
         FontStyle const & font);

    /// @brief Constructs a text view with the given allocator, UTF-8 text, style, and font
    /// @param allocator The allocator to use for memory management
    /// @param text The text to display in the view
    /// @param style The text style to apply to the text
    /// @param font The font style to apply to the text
    Text(Allocator allocator, Str8 text, TextStyle const & style,
         FontStyle const & font);

    Text(Text const &)             = delete;
    Text(Text &&)                  = default;
    Text & operator=(Text const &) = delete;
    Text & operator=(Text &&)      = default;
    virtual ~Text() override       = default;

    /// @brief Sets whether the text view is copyable
    /// @param v True if the text view should be copyable, false otherwise
    Text & copyable(bool v);

    /// @brief Sets whether the text view is highlightable
    /// @param v True if the text view should be highlightable, false otherwise
    Text & highlightable(bool v);

    /// @brief Sets whether the text view should be editable
    /// @param v True if the text view should be editable, false otherwise
    Text & editable(bool v);

    /// @brief Sets whether the text view should enable undo/redo functionality
    /// @param v True if the text view should enable undo/redo, false otherwise
    Text & enable_undo_redo(bool v);

    /// @brief Sets whether the text view should enable multiline input
    /// @param v True if the text view should enable multiline input, false otherwise
    Text & enable_multiline_input(bool v);

    /// @brief Sets whether the text view should accept tab input
    /// @param v True if the text view should accept tab input, false otherwise
    Text & accept_tab_input(bool v);

    /// @brief Sets whether pressing enter should submit the text view
    /// @param v True if pressing enter should submit, false otherwise
    Text & enter_submits(bool v);

    /// @brief Sets the number of lines per page for the text view
    /// @param v The number of lines per page to set
    Text & lines_per_page(u16 v);

    Text & input_to_actions_map(Rc<InputToActionsMap> map);

    /// @brief Sets the renderer for the text view
    /// @param renderer The renderer to set for the text view
    Text & renderer(Rc<Renderer> renderer);

    /// @brief Sets the highlights and highlight styles for the text view
    /// @param highlights The highlights to set for the text view
    /// @param highlight_styles The highlight styles to set for the text view
    Text & highlights(Span<Slice const>              highlights,
                      Span<TextHighlightStyle const> highlight_styles);

    /// @brief Clears the highlights and highlight styles for the text view
    Text & clear_highlights();

    /// @brief Sets the text style for the text view
    /// @param style The text style to set for the text view
    Text & style(TextRunsStyle style);

    /// @brief Sets the text style for the text view
    /// @param style The text style to set for the text view
    /// @param font The font style to set for the text view
    Text & style(TextStyle style, FontStyle font);

    /// @brief Sets whether the text view should wrap text
    /// @param v True if the text view should wrap text, false otherwise
    Text & wrap(bool v);

    /// @brief Sets whether the text view should use kerning
    /// @param v True if the text view should use kerning, false otherwise
    Text & use_kerning(bool v);

    /// @brief Sets whether the text view should use ligatures
    /// @param v True if the text view should use ligatures, false otherwise
    Text & use_ligatures(bool v);

    /// @brief Sets the font scale for the text view
    /// @param v The font scale to set
    Text & font_scale(f32 v);

    /// @brief Sets the text direction for the text view
    /// @param v The text direction to set
    Text & direction(TextDirection v);

    /// @brief Sets the language for the text view
    /// @param v The language to set
    Text & language(Str v);

    /// @brief Sets the alignment for the text view
    /// @param v The alignment to set
    Text & alignment(f32 v);

    /// @brief Gets the current text of the text view as a UTF-32 string
    /// @return The current text of the text view
    Str32 str() const;

    /// @brief Sets the text of the text view using a UTF-32 string
    /// @param str The UTF-32 string to set as the text
    Text & str(Str32 str);

    /// @brief Sets the text of the text view using a UTF-8 string
    /// @param str The UTF-8 string to set as the text
    Text & str(Str8 str);

    /// @brief Sets the text of the text view using a reference-counted UTF-32 string
    /// @param v The reference-counted UTF-32 string to set as the text
    Text & str(Rc<Str32> v);

    /// @brief Sets the frame for the text view
    /// @param frame The frame to set for the text view
    Text & frame(Frame const & frame);

    virtual ViewState tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Scope const & scope, Canvas canvas,
                        RenderInfo const & info) override;

    virtual Cursor cursor(Scope const & scope, f32x2 extent, f32x2 position) override;
};

/// @brief An image view that can display images with various fitting options
struct Image : View
{
    /// @brief Specifies how the image should be fitted within its frame
    enum class Fit : u8
    {
        /// @brief Try to contain the image within the frame
        /// without distorting it (preserving aspect ratio)
        Contain = 0,

        /// @brief Crop the image to fit within the frame
        Crop = 1,

        /// @brief Distort the image on either axis to fill the frame
        Stretch = 2
    };

    struct Style
    {
        /// @brief Aspect ratio of the image. (width / height)
        Option<f32> aspect_ratio = none;

        /// @brief Frame of the image view, resolved relative to the allocated space
        Frame frame = Frame{}.abs(250, 250);

        /// @brief Corner radii of the image view
        CornerRadii radii = CornerRadii::all(2);

        /// @brief Tint color of the image view
        ColorGradient tint = colors::WHITE;

        /// @brief How the image should be fitted within its frame
        Fit fit = Fit::Contain;

        /// @brief Alignment of the image within its frame
        f32x2 alignment = ALIGNMENT_CENTER_CENTER;
    };

    struct State
    {
        ImageInfo info;
    };

    Style   style_;
    ImageId source_;
    State   state_;

    /// @brief Constructs an image view with the given source
    /// @param src The source of the image
    explicit Image(ImageId src);

    /// @brief Sets the source of the image view
    /// @param src The source of the image
    Image & source(ImageId src);

    /// @brief Sets the aspect ratio of the image view
    /// @param ratio The aspect ratio to set (width / height)
    Image & aspect_ratio(f32x2 ratio);

    /// @brief Sets the aspect ratio of the image view
    /// @param ratio The aspect ratio to set (width / height)
    Image & aspect_ratio(Option<f32> ratio);

    /// @brief Sets the frame of the image view
    /// @param frame The frame to set for the image view
    Image & frame(Frame const & frame);

    /// @brief Sets the corner radii of the image view
    /// @param radii The corner radii to set for the image view
    Image & radii(CornerRadii const & radii);

    /// @brief Sets the tint color of the image view
    /// @param color The tint color to set for the image view
    Image & tint(ColorGradient const & color);

    /// @brief Sets how the image should be fitted within its frame
    /// @param fit The fit mode to set for the image view
    Image & fit(Fit fit);

    /// @brief Sets the alignment of the image within its frame
    /// @param alignment The alignment to set for the image view
    Image & align(f32x2 alignment);

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Scope const & scope, Canvas canvas,
                        RenderInfo const & info) override;
};

/// @brief A flexible layout view that arranges its items along a specified axis
struct Flex : View
{
    struct Style
    {
        /// @brief flex axis to layout items along
        Axis axis : 2 = Axis::X;

        bool wrap : 1 = true;

        /// @brief main-axis alignment. specifies how free space is used on the main axis
        MainAlign main_align : 3 = MainAlign::Start;

        /// @brief cross-axis alignment. affects how free space is used on the cross axis
        f32 cross_align = 0;

        Frame frame = Frame{}.rel(1, 1);

        Enum<Frame, Vec<Frame>> item_frame = Frame{}.rel(1, 1);
    };

    /// @brief Allocator used for memory management within the Flex view
    Allocator allocator_;

    Style style_;

    /// @brief items of the flex view
    Vec<ref<View>> items_;

    /// @brief Constructs a flex view with the given allocator
    /// @param allocator The allocator to use for memory management
    explicit Flex(Allocator allocator);
    Flex(Flex const &)             = delete;
    Flex(Flex &&)                  = default;
    Flex & operator=(Flex const &) = delete;
    Flex & operator=(Flex &&)      = default;
    virtual ~Flex() override       = default;

    /// @brief Sets the axis along which the items of the flex view will be laid out
    /// @param axis The axis to set for the flex view
    Flex & axis(Axis axis);

    /// @brief Sets whether the items of the flex view should wrap to the next line when they exceed the available space
    /// @param wrap True if the items should wrap, false otherwise
    Flex & wrap(bool wrap);

    /// @brief Sets the main-axis alignment for the flex view, which specifies how free space is used on the main axis
    /// @param align The main-axis alignment to set for the flex view
    Flex & main_align(MainAlign align);

    /// @brief Sets the cross-axis alignment for the flex view, which affects how free space is used on the cross axis
    /// @param align The cross-axis alignment to set for the flex view
    Flex & cross_align(f32 align);

    /// @brief Sets the frame for the flex view
    /// @param frame The frame to set for the flex view
    Flex & frame(Frame const & frame);

    /// @brief Sets the frame for the items of the flex view
    /// @param frame The frame to set for the items of the flex view
    Flex & item_frame(Frame const & frame);

    /// @brief Sets the frames for the items of the flex view using a span of frames
    /// @param frames The span of frames to set for the items of the flex view
    Flex & item_frame(Span<Frame const> frames);

    /// @brief Sets the frames for the items of the flex view using an initializer list of frames
    /// @param frames The initializer list of frames to set for the items of the flex view
    Flex & item_frame(InitList<Frame> frames);

    /// @brief Sets the items of the flex view using an initializer list of references to views
    /// @param list The initializer list of references to views to set as the items of the flex view
    Flex & items(InitList<ref<View>> list);

    /// @brief Sets the items of the flex view using a span of references to views
    /// @param list The span of references to views to set as the items of the flex
    Flex & items(Span<ref<View> const> list);

    virtual ViewState tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(Scope const & scope, f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;
};

/// @brief A View that stacks its items on top of each other.
struct Stack : View
{
    struct Style
    {
        Enum<i32, Vec<i32>> stack_order = 1;

        Enum<f32x2, Vec<f32x2>> alignment = ALIGNMENT_CENTER_CENTER;

        Enum<Frame, Vec<Frame>> item_frame = Frame{}.rel(1, 1);

        Frame frame = Frame{}.rel(1, 1);
    };

    Allocator      allocator_;
    Style          style_;
    Vec<ref<View>> items_;

    Stack(Allocator allocator);
    Stack(Stack const &)             = delete;
    Stack(Stack &&)                  = default;
    Stack & operator=(Stack const &) = delete;
    Stack & operator=(Stack &&)      = default;
    virtual ~Stack() override        = default;

    /// @brief Sets whether the stack view should reverse the order of its items
    /// @param reverse True if the stack view should reverse the order of its items, false
    Stack & reverse(bool reverse);

    /// @brief Sets the order of the items in the stack view using a span of indices
    /// @param order The span of indices to set as the order of the items in the stack view
    Stack & stack_order(Span<i32 const> order);

    /// @brief Sets the order of the items in the stack view using an initializer list of indices
    /// @param order The initializer list of indices to set as the order of the items in the stack view
    Stack & stack_order(InitList<i32> order);

    /// @brief Sets the alignment of the items in the stack view
    /// @param alignment The alignment to set for the items in the stack view
    Stack & align(f32x2 alignment);

    /// @brief Sets the alignment of the items in the stack view
    /// @param alignment The alignment to set for the items in the stack view
    Stack & align(Span<f32x2 const> alignment);

    /// @brief Sets the alignment of the items in the stack view
    /// @param alignment The alignment to set for the items in the stack view
    Stack & align(InitList<f32x2> alignment);

    /// @brief Sets the frame for the items of the stack view using a span of frames
    /// @param frames The span of frames to set for the items of the stack view
    Stack & items(InitList<ref<View>> list);

    /// @brief Sets the frame for the items of the stack view using a span of frames
    /// @param list The span of references to views to set as the items of the stack
    Stack & items(Span<ref<View> const> list);

    /// @brief Sets the frame for the items of the stack view using a span of frames
    /// @param frame The frame to set for the items of the stack view
    Stack & frame(Frame const & frame);

    /// @brief Sets the frame for the items of the stack view using a span of frames
    /// @param frames The span of frames to set for the items of the stack view
    Stack & item_frame(Frame const & frame);

    /// @brief Sets the frame for the items of the stack view using a span of frames
    /// @param list The span of frames to set for the items of the stack view
    Stack & item_frame(InitList<Frame> list);

    /// @brief Sets the frame for the items of the stack view using a span of frames
    /// @param list The span of frames to set for the items of the stack view
    Stack & item_frame(Span<Frame const> list);

    virtual ViewState tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(Scope const & scope, f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(Scope const & scope, f32x2, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual i32 z_index(Scope const & scope, i32 allocated, Span<i32> indices) override;
};

/// @brief A button view that can be clicked, held, and released
struct Button : View
{
    enum class Shape : u8
    {
        RRect    = 0,
        Squircle = 1
    };

    struct State
    {
        bool disabled : 1 = false;
        bool pointed  : 1 = false;
        bool focused  : 1 = false;
        bool held     : 1 = false;
        bool hovered  : 1 = false;
    };

    struct Style
    {
        ColorGradient idle_color = ColorGradient{mdc::BLUE_300};

        ColorGradient disabled_color = ColorGradient{mdc::BLUE_100};

        ColorGradient hovered_color = ColorGradient{mdc::BLUE_400};

        ColorGradient pressed_color = ColorGradient{mdc::BLUE_500};

        ColorGradient focus_ring_color = ColorGradient{mdc::GRAY_100};

        bool stroke : 1 = false;

        Shape shape : 2 = Shape::RRect;

        f32 thickness = 1.0F;

        Padding padding = {};

        CornerRadii corner_radii = CornerRadii::all(2);

        Frame frame = Frame{}.rel(1, 1);

        Frame item_frame = Frame{}.rel(1, 1);

        f32 focus_ring_thickness = 1.0F;

        Option<Shadow> shadow = none;
    };

    struct Callbacks
    {
        Dyn<Fn<void(u32)>> on_click{noop, dyn_noop};
        Dyn<Fn<void()>>    on_hold{noop, dyn_noop};
        Dyn<Fn<void()>>    on_release{noop, dyn_noop};
        Dyn<Fn<void()>>    on_hover{noop, dyn_noop};
        Dyn<Fn<void()>>    on_blur{noop, dyn_noop};
        Dyn<Fn<void()>>    on_focus_in{noop, dyn_noop};
        Dyn<Fn<void()>>    on_focus_out{noop, dyn_noop};
    };

    State          state_;
    Style          style_;
    Callbacks      callbacks_;
    Option<View &> item_;

    Button()                           = default;
    Button(Button const &)             = delete;
    Button(Button &&)                  = default;
    Button & operator=(Button const &) = delete;
    Button & operator=(Button &&)      = default;
    virtual ~Button() override         = default;

    /// @brief Sets whether the button is disabled
    /// @param disable True if the button should be disabled, false otherwise
    Button & disable(bool disable);

    /// @brief Sets the color of the button
    /// @param color The color to set for the button
    Button & color(ColorGradient const & color);

    /// @brief Sets the idle color of the button
    /// @param color The idle color to set for the button
    Button & idle_color(ColorGradient const & color);

    /// @brief Sets the disabled color of the button
    /// @param color The disabled color to set for the button
    Button & disabled_color(ColorGradient const & color);

    /// @brief Sets the hovered color of the button
    /// @param color The hovered color to set for the button
    Button & hovered_color(ColorGradient const & color);

    /// @brief Sets the pressed color of the button
    /// @param color The pressed color to set for the button
    Button & pressed_color(ColorGradient const & color);

    /// @brief Sets the focus ring color of the button
    /// @param color The focus ring color to set for the button
    Button & focus_ring_color(ColorGradient const & color);

    /// @brief Sets the shape of the button to a rounded rectangle with the specified corner radii
    /// @param radii The corner radii to set for the button
    Button & rrect(CornerRadii const & radii);

    /// @brief Sets the shape of the button to a squircle with the specified degree
    /// @param degree The degree of the squircle to set for the button
    Button & squircle(f32 degree = 5);

    /// @brief Sets whether the button should have a stroke (border)
    /// @param stroke Whether the button should have a stroke (border)
    Button & stroke(bool stroke);

    /// @brief Sets the thickness of the button's border
    /// @param thickness The thickness of the button's border to set
    Button & thickness(f32 thickness);

    /// @brief Sets the padding of the button
    /// @param padding The padding to set for the button
    Button & padding(Padding const & padding);

    /// @brief Sets the thickness of the button's focus ring
    /// @param thickness The thickness of the button's focus ring to set
    Button & focus_ring_thickness(f32 thickness);

    /// @brief Sets the frame of the item view of the button
    /// @param frame The frame to set for the item view of the button
    Button & item_frame(Frame const & frame);

    /// @brief Sets the frame of the view of the button
    /// @param frame The frame to set for the view of the button
    Button & frame(Frame const & frame);

    /// @brief Set the callback function to be called when the button is clicked
    /// @param fn The callback function to set for the button click event
    Button & on_click(Dyn<Fn<void(u32)>> fn);

    /// @brief Set the callback function to be called when the button is clicked
    /// @param fn The callback function to set for the button click event
    Button & on_click(Fn<void(u32)> fn);

    /// @brief Set the callback function to be called when the button is held
    /// @param fn The callback function to set for the button hold event
    Button & on_hold(Dyn<Fn<void()>> fn);

    /// @brief Set the callback function to be called when the button is held
    /// @param fn The callback function to set for the button hold event
    Button & on_hold(Fn<void()> fn);

    /// @brief Set the callback function to be called when the button is released
    /// @param fn The callback function to set for the button release event
    Button & on_release(Dyn<Fn<void()>> fn);

    /// @brief Set the callback function to be called when the button is released
    /// @param fn The callback function to set for the button release event
    Button & on_release(Fn<void()> fn);

    /// @brief Set the callback function to be called when the button is hovered
    /// @param fn The callback function to set for the button hover event
    Button & on_hover(Dyn<Fn<void()>> fn);

    /// @brief Set the callback function to be called when the button is hovered
    /// @param fn The callback function to set for the button hover event
    Button & on_hover(Fn<void()> fn);

    /// @brief Set the callback function to be called when the button loses focus
    /// @param fn The callback function to set for the button blur event
    Button & on_blur(Dyn<Fn<void()>> fn);

    /// @brief Set the callback function to be called when the button loses focus
    /// @param fn The callback function to set for the button blur event
    Button & on_blur(Fn<void()> fn);

    /// @brief Set the callback function to be called when the button is focused in
    /// @param fn The callback function to set for the button focus in event
    Button & on_focus_in(Dyn<Fn<void()>> fn);

    /// @brief Set the callback function to be called when the button is focused in
    /// @param fn The callback function to set for the button focus in event
    Button & on_focus_in(Fn<void()> fn);

    /// @brief Set the callback function to be called when the button is focused out
    /// @param fn The callback function to set for the button focus out event
    Button & on_focus_out(Dyn<Fn<void()>> fn);

    /// @brief Set the callback function to be called when the button is focused out
    /// @param fn The callback function to set for the button focus out event
    Button & on_focus_out(Fn<void()> fn);

    /// @brief Sets the item view of the button
    /// @param item The item view to set for the button
    Button & item(Option<View &> item);

    /// @brief Sets the shadow of the button
    /// @param shadow The shadow to set for the button
    Button & shadow(Option<Shadow> shadow);

    virtual ViewState tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(Scope const & scope, f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Scope const & scope, Canvas canvas,
                        RenderInfo const & info) override;

    virtual Cursor cursor(Scope const & scope, f32x2 extent, f32x2 position) override;
};

/// @brief A view that can contain a single item view and apply styling such as background color, border, and padding.
struct Box : View
{
    struct Style
    {
        Option<ImageInfo> background_image = none;

        ColorGradient background_image_tint = colors::WHITE;

        ColorGradient background_color = {};

        f32 border_thickness = 0.0F;

        ColorGradient border_color = colors::BLACK;

        CornerRadii radii = CornerRadii::all(0);

        Padding padding = Padding::all(0);

        f32 background_blur = 0.0F;

        Frame frame = Frame{}.rel(1, 1);

        Frame item_frame = Frame{}.rel(1, 1);

        Option<Shadow> shadow = none;
    };

    Style          style_;
    Option<View &> item_;

    Box()                        = default;
    Box(Box const &)             = delete;
    Box(Box &&)                  = default;
    Box & operator=(Box const &) = delete;
    Box & operator=(Box &&)      = default;
    virtual ~Box() override      = default;

    /// @brief Sets the item view of the box
    /// @param item The item view to set for the box
    Box & item(Option<View &> item);

    /// @brief Sets the background image of the box
    /// @param image The background image to set for the box
    Box & background_image(Option<ImageId>       image,
                           ColorGradient const & tint = colors::WHITE);

    /// @brief Sets the background color of the box
    /// @param color The background color to set for the box
    Box & background_color(ColorGradient const & color);

    /// @brief Sets the border thickness of the box
    /// @param thickness The border thickness to set for the box
    Box & border_thickness(f32 thickness);

    /// @brief Sets the border color of the box
    /// @param color The border color to set for the box
    Box & border_color(ColorGradient const & color);

    /// @brief Sets the corner radii of the box
    /// @param radii The corner radii to set for the box
    Box & radii(CornerRadii const & radii);

    /// @brief Sets the padding of the box
    /// @param padding The padding to set for the box
    Box & padding(Padding const & padding);

    /// @brief Sets the frame of the box
    /// @param frame The frame to set for the box
    Box & background_blur(f32 blur);

    /// @brief Sets the frame of the box
    /// @param frame The frame to set for the box
    Box & frame(Frame const & frame);

    /// @brief Sets the frame of the box
    /// @param frame The frame to set for the box
    Box & item_frame(Frame const & frame);

    /// @brief Sets the shadow of the box
    /// @param shadow The shadow to set for the box
    Box & shadow(Option<Shadow> shadow);

    virtual ViewState tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(Scope const & scope, f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Scope const & scope, Canvas canvas,
                        RenderInfo const & info) override;
};

/// @brief A checkbox view that can be checked or unchecked
struct CheckBox : View
{
    struct Style
    {
        Frame frame;
    };

    struct State
    {
        bool checked = false;
    };

    struct Callbacks
    {
        Dyn<Fn<void(bool)>> on_change{noop, dyn_noop};
    };

    Allocator allocator_;
    Style     style_;
    State     state_;
    Callbacks callbacks_;
    Text      checked_text_;
    Text      unchecked_text_;

    // TODO: animation control and states

    explicit CheckBox(Allocator allocator);
    CheckBox(CheckBox const &)             = delete;
    CheckBox(CheckBox &&)                  = default;
    CheckBox & operator=(CheckBox const &) = delete;
    CheckBox & operator=(CheckBox &&)      = default;
    virtual ~CheckBox() override           = default;

    virtual ViewState tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual void size(Scope const & scope, f32x2 allocated, Span<f32x2> sizes) override;

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Scope const & scope, Canvas canvas,
                        RenderInfo const & info) override;
};

struct Switch
{
};

struct Slider
{
};

struct ScrollView
{
};

}    // namespace ui
}    // namespace ash
