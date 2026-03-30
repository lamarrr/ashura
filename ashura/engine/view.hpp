/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/canvas.hpp"
#include "ashura/engine/input.hpp"
#include "ashura/std/math.hpp"
#include "ashura/std/range.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

namespace ui
{

/// @brief Simple Adaptive Layout Constraint Model
struct Size
{
    f32 abs_     = 0;
    f32 rel_     = 0;
    f32 rel_min_ = 0;
    f32 rel_max_ = 1;
    f32 min_     = 0;
    f32 max_     = F32_INF;

    /// @brief Adding or subtracting from the source size, i.e. value should
    /// be source size - 20px
    constexpr Size & abs(f32 s)
    {
        abs_ = s;
        return *this;
    }

    /// @brief Scales the source size, i.e. value should be 0.5 of source
    /// size
    constexpr Size & rel(f32 s)
    {
        rel_ = s;
        return *this;
    }

    /// @brief  clamps the source size relatively. i.e. value should be at
    /// least 0.5 of source size
    constexpr Size & rel_min(f32 s)
    {
        rel_min_ = s;
        return *this;
    }

    /// @brief  clamps the source size relatively. i.e. value should be at
    /// most 0.5 of source size
    constexpr Size & rel_max(f32 s)
    {
        rel_max_ = s;
        return *this;
    }

    /// @brief Clamps the source size, i.e. value should be at least 20px
    constexpr Size & min(f32 s)
    {
        min_ = s;
        return *this;
    }

    /// @brief Clamps the source size, i.e. value should be at most 100px
    constexpr Size & max(f32 s)
    {
        max_ = s;
        return *this;
    }

    constexpr Size & constrain(bool c)
    {
        rel_max_ = c ? 1 : F32_INF;
        return *this;
    }

    constexpr f32 operator()(f32 anchor) const
    {
        return clamp(clamp(abs_ + anchor * rel_, rel_min_ * anchor, rel_max_ * anchor),
                     min_, max_);
    }
};

struct Frame
{
    Size x_{};
    Size y_{};

    constexpr f32x2 operator()(f32 anchor_x, f32 anchor_y) const
    {
        return f32x2{x_(anchor_x), y_(anchor_y)};
    }

    constexpr f32x2 operator()(f32x2 anchor) const
    {
        return this->operator()(anchor.x(), anchor.y());
    }

    constexpr Frame & abs(f32 x, f32 y)
    {
        x_.abs(x);
        y_.abs(y);
        return *this;
    }

    constexpr Frame & abs(f32x2 anchor)
    {
        return abs(anchor.x(), anchor.y());
    }

    constexpr Frame & rel(f32 x, f32 y)
    {
        x_.rel(x);
        y_.rel(y);
        return *this;
    }

    constexpr Frame & rel(f32x2 anchor)
    {
        return rel(anchor.x(), anchor.y());
    }

    constexpr Frame & rel_min(f32 x, f32 y)
    {
        x_.rel_min(x);
        y_.rel_min(y);
        return *this;
    }

    constexpr Frame & rel_min(f32x2 anchor)
    {
        return rel_min(anchor.x(), anchor.y());
    }

    constexpr Frame & rel_max(f32 x, f32 y)
    {
        x_.rel_max(x);
        y_.rel_max(y);
        return *this;
    }

    constexpr Frame & rel_max(f32x2 anchor)
    {
        return rel_max(anchor.x(), anchor.y());
    }

    constexpr Frame & min(f32 x, f32 y)
    {
        x_.min(x);
        y_.min(y);
        return *this;
    }

    constexpr Frame & min(f32x2 anchor)
    {
        return min(anchor.x(), anchor.y());
    }

    constexpr Frame & max(f32 x, f32 y)
    {
        x_.max(x);
        y_.max(y);
        return *this;
    }

    constexpr Frame & max(f32x2 anchor)
    {
        return max(anchor.x(), anchor.y());
    }

    constexpr Frame & constrain(bool x, bool y)
    {
        x_.constrain(x);
        y_.constrain(y);
        return *this;
    }

    constexpr Size & operator[](usize i)
    {
        return (&x_)[i];
    }

    constexpr Size const & operator[](usize i) const
    {
        return (&x_)[i];
    }
};

struct CornerRadii
{
    /// @brief Top-left
    f32 tl = 0;

    /// @brief Top-right
    f32 tr = 0;

    /// @brief Bottom-left
    f32 bl = 0;

    /// @brief Bottom-right
    f32 br = 0;

    static constexpr CornerRadii all(f32 r)
    {
        return {r, r, r, r};
    }

    constexpr operator f32x4() const
    {
        return f32x4{tl, tr, bl, br};
    }
};

struct Padding
{
    /// @brief Left
    f32 l = 0;

    /// @brief Top
    f32 t = 0;

    /// @brief Right
    f32 r = 0;

    /// @brief Bottom
    f32 b = 0;

    static constexpr Padding all(f32 r)
    {
        return {r, r, r, r};
    }

    constexpr operator f32x4() const
    {
        return f32x4{l, t, r, b};
    }

    constexpr f32 vert() const
    {
        return l + r;
    }

    constexpr f32 horz() const
    {
        return t + b;
    }

    constexpr f32x2 axes() const
    {
        return f32x2{horz(), vert()};
    }
};

enum class MainAlign : u8
{
    Start        = 0,
    End          = 1,
    SpaceBetween = 2,
    SpaceAround  = 3,
    SpaceEvenly  = 4
};

struct ScrollInfo
{
    f32x2 center = {};
    f32x2 zoom   = {1, 1};
};

struct HitInfo
{
    /// @brief Viewport-space region of the view that was hit
    /// with (0, 0) as the center of the viewport
    f32x2 viewport_hit;

    /// @brief Canvas-space region that was hit
    f32x2 canvas_hit;

    /// @brief The viewport-space region of the view
    CRect viewport_region;

    /// @brief The canvas-space region of the view
    CRect canvas_region;

    affinef32x3 canvas_transform = affinef32x3::identity();

    constexpr f32x2 zoom() const
    {
        return canvas_region.extent / viewport_region.extent;
    }
};

struct Events
{
    enum Type : u8
    {
        /// @brief View has been mounted to the view tree and has now received an
        /// ID.
        Mount       = 0,
        /// @brief The pointer has entered the view's area
        PointerIn   = 1,
        /// @brief The pointer has left the view's area
        PointerOut  = 2,
        /// @brief The pointer is hovering the view
        PointerOver = 3,
        /// @brief The pointer has been pressed down on the view
        PointerDown = 4,
        /// @brief The pointer's press has been released from the view
        PointerUp   = 5,
        /// @brief A scroll request has been sent to the view
        Scroll      = 6,
        /// @brief Drag event has begun on this view
        DragStart   = 7,
        /// @brief An update on the drag state has been gotten
        DragUpdate  = 8,
        /// @brief The dragging of this view has completed/canceled
        DragEnd     = 9,
        /// @brief Drag data has entered this view and might be dropped
        DragIn      = 10,
        /// @brief Drag data has left the view without being dropped
        DragOut     = 11,
        /// @brief Drag data is hovering this view as destination without being
        /// dropped
        DragOver    = 12,
        /// @brief Drag data is now available for the view to consume
        Drop        = 13,
        /// @brief The view has received focus
        FocusIn     = 14,
        /// @brief The view has lost focus
        FocusOut    = 15,
        /// @brief The view currently has active focus
        FocusOver   = 16,
        /// @brief A key went down whilst this view has focus
        KeyDown     = 17,
        /// @brief A key went up whilst this view has focus
        KeyUp       = 18,
        /// @brief The view has received composition text whilst it has focus
        TextInput   = 19
    };

    struct Bits
    {
        enum Type : u32
        {
            None        = 0,
            Mount       = 1U << Events::Mount,
            PointerIn   = 1U << Events::PointerIn,
            PointerOut  = 1U << Events::PointerOut,
            PointerOver = 1U << Events::PointerOver,
            PointerDown = 1U << Events::PointerDown,
            PointerUp   = 1U << Events::PointerUp,
            Scroll      = 1U << Events::Scroll,
            DragStart   = 1U << Events::DragStart,
            DragUpdate  = 1U << Events::DragUpdate,
            DragEnd     = 1U << Events::DragEnd,
            DragIn      = 1U << Events::DragIn,
            DragOut     = 1U << Events::DragOut,
            DragOver    = 1U << Events::DragOver,
            Drop        = 1U << Events::Drop,
            FocusIn     = 1U << Events::FocusIn,
            FocusOut    = 1U << Events::FocusOut,
            FocusOver   = 1U << Events::FocusOver,
            KeyDown     = 1U << Events::KeyDown,
            KeyUp       = 1U << Events::KeyUp,
            TextInput   = 1U << Events::TextInput
        };

        static constexpr Type at(Events::Type e)
        {
            return static_cast<Type>(1 << static_cast<u8>(e));
        }
    };

    Bits::Type bits_ = Bits::None;

    /// @brief The view's hit data
    Option<HitInfo> hit_info_ = none;

    /// @brief Scroll request
    Option<ScrollInfo> scroll_info_ = none;

    constexpr bool mount() const
    {
        return bits_ & Bits::Mount;
    }

    constexpr bool pointer_in() const
    {
        return bits_ & Bits::PointerIn;
    }

    constexpr bool pointer_out() const
    {
        return bits_ & Bits::PointerOut;
    }

    constexpr bool pointer_over() const
    {
        return bits_ & Bits::PointerOver;
    }

    constexpr bool pointer_down() const
    {
        return bits_ & Bits::PointerDown;
    }

    constexpr bool pointer_up() const
    {
        return bits_ & Bits::PointerUp;
    }

    constexpr bool scroll() const
    {
        return bits_ & Bits::Scroll;
    }

    constexpr bool drag_start() const
    {
        return bits_ & Bits::DragStart;
    }

    constexpr bool drag_update() const
    {
        return bits_ & Bits::DragUpdate;
    }

    constexpr bool drag_end() const
    {
        return bits_ & Bits::DragEnd;
    }

    constexpr bool drag_in() const
    {
        return bits_ & Bits::DragIn;
    }

    constexpr bool drag_out() const
    {
        return bits_ & Bits::DragOut;
    }

    constexpr bool drag_over() const
    {
        return bits_ & Bits::DragOver;
    }

    constexpr bool drop() const
    {
        return bits_ & Bits::Drop;
    }

    constexpr bool focus_in() const
    {
        return bits_ & Bits::FocusIn;
    }

    constexpr bool focus_out() const
    {
        return bits_ & Bits::FocusOut;
    }

    constexpr bool focus_over() const
    {
        return bits_ & Bits::FocusOver;
    }

    constexpr bool key_down() const
    {
        return bits_ & Bits::KeyDown;
    }

    constexpr bool key_up() const
    {
        return bits_ & Bits::KeyUp;
    }

    constexpr bool text_input() const
    {
        return bits_ & Bits::TextInput;
    }
};

struct FocusRect
{
    CRect area = {};
    CRect clip = {};
};

struct CoreTheme
{
    u8x4   background       = {};
    u8x4   surface          = {};
    u8x4   surface_variant  = {};
    u8x4   primary          = {};
    u8x4   primary_variant  = {};
    u8x4   error            = {};
    u8x4   warning          = {};
    u8x4   success          = {};
    u8x4   active           = {};
    u8x4   inactive         = {};
    u8x4   on_background    = {};
    u8x4   on_surface       = {};
    u8x4   on_primary       = {};
    u8x4   on_error         = {};
    u8x4   on_warning       = {};
    u8x4   on_success       = {};
    u8x4   focus            = {};
    u8x4   highlight        = {};
    u8x4   caret            = {};
    f32    head_font_height = {};
    f32    body_font_height = {};
    f32    line_height      = {};
    FontId head_font        = FontId::Default;
    FontId body_font        = FontId::Default;
    FontId icon_font        = FontId::Default;
};

extern CoreTheme default_core_theme();

struct InputScope
{
    SystemState * sys_;

    WindowState * win_;

    InputScope(SystemState & sys, WindowState & win) : sys_{&sys}, win_{&win}
    {
    }

    InputScope(InputScope const &)             = delete;
    InputScope(InputScope &&)                  = default;
    InputScope & operator=(InputScope const &) = delete;
    InputScope & operator=(InputScope &&)      = default;
    ~InputScope()                              = default;

    time_point timestamp() const
    {
        return sys_->timestamp();
    }

    nanoseconds timedelta() const
    {
        return sys_->timedelta();
    }

    WindowState const & window() const
    {
        return *win_;
    }

    KeyState const & key() const
    {
        return win_->key();
    }

    MouseState const & mouse() const
    {
        return win_->mouse();
    }

    DropState const & drop() const
    {
        return win_->drop();
    }

    SystemState const & system() const
    {
        return *sys_;
    }

    ThemeState const & theme() const
    {
        return sys_->theme();
    }
};

using UserDataMap = ByteDict<Dyn<void *>>;

struct ViewSysScope
{
    /// @brief Canvas-space region the system is currently focused on
    Option<FocusRect> focus_rect_;

    Option<Cursor> cursor_;

    CoreTheme core_theme_;

    bool closing_;

    bool closing_deferred_;

    /// @brief Current frame id
    u64 frame_ = 0;

    UserDataMap user_data_map_;

    ViewSysScope(CoreTheme const & core_theme, UserDataMap user_data_map) :
      focus_rect_{none},
      cursor_{none},
      core_theme_{core_theme},
      closing_{false},
      closing_deferred_{false},
      frame_{0},
      user_data_map_{std::move(user_data_map)}
    {
    }

    Option<FocusRect> focus_rect() const
    {
        return focus_rect_;
    }

    Option<Cursor> cursor() const
    {
        return cursor_;
    }

    CoreTheme const & core_theme() const
    {
        return core_theme_;
    }

    bool closing() const
    {
        return closing_;
    }

    bool closing_deferred() const
    {
        return closing_deferred_;
    }

    u64 frame() const
    {
        return frame_;
    }

    Option<void *> get_user_data(Span<u8 const> tag) const;
};

/// @brief Global View Scope, Properties of the scope all the views for
/// a specific window are in.
struct Scope
{
    InputScope input_;

    ViewSysScope * view_;

    Scope(InputScope input, ViewSysScope & view) :
      input_{std::move(input)},
      view_{&view}
    {
    }

    Scope(Scope const &)             = delete;
    Scope(Scope &&)                  = default;
    Scope & operator=(Scope const &) = delete;
    Scope & operator=(Scope &&)      = default;
    ~Scope()                         = default;

    time_point timestamp() const
    {
        return input_.timestamp();
    }

    nanoseconds timedelta() const
    {
        return input_.timedelta();
    }

    WindowState const & window() const
    {
        return input_.window();
    }

    KeyState const & key() const
    {
        return input_.key();
    }

    MouseState const & mouse() const
    {
        return input_.mouse();
    }

    DropState const & drop() const
    {
        return input_.drop();
    }

    SystemState const & system() const
    {
        return input_.system();
    }

    ThemeState const & theme() const
    {
        return input_.theme();
    }

    CoreTheme const & core_theme() const
    {
        return view_->core_theme();
    }

    Option<FocusRect> focus_rect() const
    {
        return view_->focus_rect();
    }

    Option<void *> get_user_data(Span<u8 const> tag) const
    {
        return view_->get_user_data(tag);
    }
};

enum class TabIndex : i32
{
    Auto = I32_MAX
};

/// @brief Makes a zoom transform matrix relative to the center of a viewport.
/// defines the translation and scaling components.
/// @return zoom transform matrix
struct ViewState
{
    /// @brief Tab Index for Focus-Based Navigation. desired tab index, `None`
    /// means the default tab order based on the hierarchy of the parent to
    /// children and siblings (depth-first traversal). Negative values are
    /// focused before positive values.
    TabIndex tab_index = TabIndex::Auto;

    /// @brief If set, will be treated as a text input area
    TextInputInfo text = {};

    /// @brief If the view should be hidden from view (will not receive
    /// visual events, but still receive tick events)
    bool hidden : 1 = false;

    /// @brief Can receive mouse enter/move/leave events
    bool pointable : 1 = false;

    /// @brief Can receive mouse press events
    bool clickable : 1 = false;

    /// @brief Can receive mouse scroll events
    bool scrollable : 1 = false;

    /// @brief Can the view produce drag data
    bool draggable : 1 = false;

    /// @brief Can the view receive drag data
    bool droppable : 1 = false;

    /// @brief Can receive keyboard focus (ordered by `tab`) and keyboard events
    bool focusable : 1 = false;

    /// @brief Grab focus of the user
    bool grab_focus : 1 = false;

    /// @brief Is view a viewport
    bool viewport : 1 = false;

    /// @brief Request the view system to defer shutdown to next frame
    bool defer_close : 1 = false;
};

struct Layout
{
    /// @brief Extent of the view within the parent. if it is a viewport,
    /// this is the visible extent of the viewport within the parent viewport.
    f32x2 extent = {};

    /// @brief Inner extent, if it is a viewport
    f32x2 viewport_extent = {};

    f32x2 viewport_center = {};

    f32x2 viewport_zoom = {1, 1};

    /// @brief Viewport-space re-positioning of the view
    Option<f32x2> fixed_center = none;
};

enum class ViewId : u64
{
    None = U64_MAX
};

struct RenderInfo
{
    /// @brief Viewport-space region of the view (before zoom transform)
    CRect viewport_region = {};

    /// @brief Canvas-space region of the view (after zoom transform)
    CRect canvas_region = {};

    /// @brief Canvas-space clip of the view (after zoom transform)
    CRect clip = MAX_CLIP;

    /// @brief Displacement and scale transform from the viewports to canvas-space
    affinef32x3 canvas_transform = affinef32x3::identity();
};

struct LayerStack
{
    i32 views         = 0x0000'0000;
    i32 viewport_bars = 0x000F'FFFF;
    i32 modals        = 0x001F'FFFF;
    i32 overlays      = 0x002F'FFFF;
};

inline constexpr LayerStack LAYERS;

struct ViewInternalState
{
    i32           tab_index       = 0;
    u16           viewport        = 0;
    bool          hidden      : 1 = false;
    bool          pointable   : 1 = false;
    bool          clickable   : 1 = false;
    bool          scrollable  : 1 = false;
    bool          draggable   : 1 = false;
    bool          droppable   : 1 = false;
    bool          focusable   : 1 = false;
    bool          is_viewport : 1 = false;
    TextInputInfo input           = {};
};

/// @brief Base view class.
/// Views are plain visual elements that define spatial relationships,
/// visual state changes, and forward events to other subsystems.
/// @note State changes must only happen in the `tick` method. Child view
/// modifications should be handled with it as well.
///
/// The coordinate system used is one in which the center of the screen is (0,
/// 0) and ranges from [-0.5w, +0.5w] on both axes. i.e. top-left is [-0.5w,
/// -0.5h] and bottom-right is [+0.5w, +0.5h].
struct View
{
    /// @brief Id of the view if mounted, otherwise `ViewId::None`

    ViewId id_ = ViewId::None;

    bool hot_ = false;

    ViewInternalState state_ = {};

    constexpr View()                         = default;
    constexpr View(View const &)             = default;
    constexpr View(View &&)                  = default;
    constexpr View & operator=(View const &) = default;
    constexpr View & operator=(View &&)      = default;
    constexpr virtual ~View()                = default;

    /// @returns the ID currently allocated to the view or none
    constexpr ViewId id() const
    {
        return id_;
    }

    /// @brief Called on every frame. used for state changes, animations, task
    /// dispatch and lightweight processing related to the GUI. heavy-weight and
    /// non-sub-millisecond tasks should be dispatched to a subsystem that would
    /// handle it. i.e. using the multi-tasking or asset-loading systems.
    /// @param scope the associated scope of the previous frame
    /// @param events events due to the previous frame's state
    /// @param build callback to be called to insert subviews.
    constexpr virtual ViewState tick(Scope const & scope, Events const & events,
                                     Fn<void(View &)> build)
    {
        (void) scope;
        (void) events;
        (void) build;
        return {};
    }

    /// @brief Distributes the size allocated to it to its child views.
    /// @param allocated the size allocated to this view
    /// @param[out] sizes sizes allocated to the children.
    constexpr virtual void size(Scope const & scope, f32x2 allocated, Span<f32x2> sizes)
    {
        (void) scope;
        fill(sizes, allocated);
    }

    /// @brief Fits itself around its children and positions child views
    /// relative to its center
    /// @param allocated the size allocated to this view
    /// @param sizes sizes of the child views
    /// @param[out] centers parent-space centers of the child views
    /// @return this view's fitted extent
    constexpr virtual Layout fit(Scope const & scope, f32x2 allocated,
                                 Span<f32x2 const> sizes, Span<f32x2> centers)
    {
        (void) scope;
        (void) allocated;
        (void) sizes;
        fill(centers, f32x2{0, 0});
        return {};
    }

    /// @brief Returns the stacking layer index
    /// @param allocated stacking layer index allocated to this view
    /// by parent. This functions similar to the CSS stacking context. The layer
    /// index has a higher priority over the z-index and events do not bubble
    /// through it.
    /// @return stack index for the view
    constexpr virtual i32 layer(Scope const & scope, i32 allocated, Span<i32> indices)
    {
        (void) scope;
        fill(indices, allocated);
        return allocated;
    }

    /// @brief Returns the z-index of itself and assigns z-indices to its children
    /// @param allocated z-index allocated to this view by parent
    /// @param[out] indices z-index assigned to children
    /// @return preferred z_index
    constexpr virtual i32 z_index(Scope const & scope, i32 allocated, Span<i32> indices)
    {
        (void) scope;
        fill(indices, allocated);
        return allocated;
    }

    /// @brief Record draw commands needed to render this view. this method is
    /// only called if the view passes the visibility tests. this is called on
    /// every frame.
    /// @param canvas canvas to render view into
    /// @param info information needed to render the view into its alloted canvas
    /// space
    constexpr virtual void render(Scope const & scope, Canvas canvas,
                                  RenderInfo const & info)
    {
        (void) scope;
        (void) canvas;
        (void) info;
    }

    /// @brief Select cursor type given a pointed region of the view.
    /// @param extent layout extent of the view
    /// @param position local-space position of the pointer
    /// @return preferred cursor type
    constexpr virtual Cursor cursor(Scope const & scope, f32x2 extent, f32x2 position)
    {
        (void) scope;
        (void) extent;
        (void) position;
        return Cursor::Default;
    }
};

}    // namespace ui
}    // namespace ash
