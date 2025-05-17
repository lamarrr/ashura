/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/canvas.h"
#include "ashura/engine/input.h"
#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

/// @brief Simple Adaptive Layout Constraint Model
/// @param offset adding or subtracting from the source size, i.e. value should
/// be source size - 20px
/// @param scale scales the source size, i.e. value should be 0.5 of source
/// size
/// @param rmin  clamps the source size relatively. i.e. value should be at
/// least 0.5 of source size
/// @param rmax  clamps the source size relatively. i.e. value should be at
/// most 0.5 of source size
/// @param min clamps the source size, i.e. value should be at least 20px
/// @param max clamps the source size, i.e. value should be at most 100px
struct Size
{
  f32 offset = 0;
  f32 scale  = 0;
  f32 rmin   = 0;
  f32 rmax   = 1;
  f32 min    = 0;
  f32 max    = F32_INF;

  constexpr f32 operator()(f32 value) const
  {
    return clamp(clamp(offset + value * scale, rmin * value, rmax * value), min,
                 max);
  }
};

struct Frame
{
  Size x{};
  Size y{};

  constexpr Frame()                          = default;
  constexpr Frame(Frame const &)             = default;
  constexpr Frame(Frame &&)                  = default;
  constexpr Frame & operator=(Frame const &) = default;
  constexpr Frame & operator=(Frame &&)      = default;
  constexpr ~Frame()                         = default;

  constexpr Frame(Size width, Size height) : x{width}, y{height}
  {
  }

  constexpr Frame(Vec2 extent, bool constrain = true) :
    x{.offset = extent.x, .rmax = constrain ? 1 : F32_INF},
    y{.offset = extent.y, .rmax = constrain ? 1 : F32_INF}
  {
  }

  constexpr Vec2 operator()(Vec2 extent) const
  {
    return Vec2{x(extent.x), y(extent.y)};
  }

  constexpr Frame offset(Vec2 extent) const
  {
    Frame out{*this};
    out.x.offset = extent.x;
    out.y.offset = extent.y;
    return out;
  }

  constexpr Frame offset(f32 width, f32 height) const
  {
    return offset({width, height});
  }

  constexpr Frame scale(Vec2 extent) const
  {
    Frame out{*this};
    out.x.scale = extent.x;
    out.y.scale = extent.y;
    return out;
  }

  constexpr Frame scale(f32 width, f32 height) const
  {
    return scale({width, height});
  }

  constexpr Frame rmin(Vec2 extent) const
  {
    Frame out{*this};
    out.x.rmin = extent.x;
    out.y.rmin = extent.y;
    return out;
  }

  constexpr Frame rmin(f32 width, f32 height) const
  {
    return rmin({width, height});
  }

  constexpr Frame rmax(Vec2 extent) const
  {
    Frame out{*this};
    out.x.rmax = extent.x;
    out.y.rmax = extent.y;
    return out;
  }

  constexpr Frame rmax(f32 width, f32 height) const
  {
    return rmax({width, height});
  }

  constexpr Frame min(Vec2 extent) const
  {
    Frame out{*this};
    out.x.min = extent.x;
    out.y.min = extent.y;
    return out;
  }

  constexpr Frame min(f32 width, f32 height) const
  {
    return min({width, height});
  }

  constexpr Frame max(Vec2 extent) const
  {
    Frame out{*this};
    out.x.max = extent.x;
    out.y.max = extent.y;
    return out;
  }

  constexpr Frame max(f32 width, f32 height) const
  {
    return max({width, height});
  }

  constexpr Size & operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr Size const & operator[](usize i) const
  {
    return (&x)[i];
  }
};

struct CornerRadii
{
  f32 tl = 0;
  f32 tr = 0;
  f32 bl = 0;
  f32 br = 0;

  static constexpr CornerRadii all(f32 r)
  {
    return {r, r, r, r};
  }

  constexpr operator Vec4() const
  {
    return Vec4{tl, tr, bl, br};
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
  Vec2 center = {};
  Vec2 zoom   = {1, 1};
};

struct HitInfo
{
  /// @brief viewport-space region of the view that was hit
  /// with (0, 0) as the center of the viewport
  Vec2 viewport_hit;

  /// @brief canvas-space region that was hit
  Vec2 canvas_hit;

  /// @brief the viewport-space region of the view
  CRect viewport_region;

  /// @brief the canvas-space region of the view
  CRect canvas_region;

  constexpr Vec2 zoom() const
  {
    return canvas_region.extent / viewport_region.extent;
  }
};

struct Events
{
  enum Type : u8
  {
    /// @brief view has been mounted to the view tree and has now received an ID.
    Mount       = 0,
    /// @brief the pointer has entered the view's area
    PointerIn   = 1,
    /// @brief the pointer has left the view's area
    PointerOut  = 2,
    /// @brief the pointer is hovering the view
    PointerOver = 3,
    /// @brief the pointer has been pressed down on the view
    PointerDown = 4,
    /// @brief the pointer's press has been released from the view
    PointerUp   = 5,
    /// @brief a scroll request has been sent to the view
    Scroll      = 6,
    /// @brief drag event has begun on this view
    DragStart   = 7,
    /// @brief an update on the drag state has been gotten
    DragUpdate  = 8,
    /// @brief the dragging of this view has completed/canceled
    DragEnd     = 9,
    /// @brief drag data has entered this view and might be dropped
    DragIn      = 10,
    /// @brief drag data has left the view without being dropped
    DragOut     = 11,
    /// @brief drag data is hovering this view as destination without being dropped
    DragOver    = 12,
    /// @brief drag data is now available for the view to consume
    Drop        = 13,
    /// @brief the view has received focus
    FocusIn     = 14,
    /// @brief the view has lost focus
    FocusOut    = 15,
    /// @brief the view currently has active focus
    FocusOver   = 16,
    /// @brief a key went down whilst this view has focus
    KeyDown     = 17,
    /// @brief a key went up whilst this view has focus
    KeyUp       = 18,
    /// @brief the view has received composition text whilst it has focus
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

  Bits::Type bits = Bits::None;

  /// @brief the view's hit data
  Option<HitInfo> hit_info = none;

  /// @brief scroll request
  Option<ScrollInfo> scroll_info = none;

  constexpr bool mount() const
  {
    return bits & Bits::Mount;
  }

  constexpr bool pointer_in() const
  {
    return bits & Bits::PointerIn;
  }

  constexpr bool pointer_out() const
  {
    return bits & Bits::PointerOut;
  }

  constexpr bool pointer_over() const
  {
    return bits & Bits::PointerOver;
  }

  constexpr bool pointer_down() const
  {
    return bits & Bits::PointerDown;
  }

  constexpr bool pointer_up() const
  {
    return bits & Bits::PointerUp;
  }

  constexpr bool scroll() const
  {
    return bits & Bits::Scroll;
  }

  constexpr bool drag_start() const
  {
    return bits & Bits::DragStart;
  }

  constexpr bool drag_update() const
  {
    return bits & Bits::DragUpdate;
  }

  constexpr bool drag_end() const
  {
    return bits & Bits::DragEnd;
  }

  constexpr bool drag_in() const
  {
    return bits & Bits::DragIn;
  }

  constexpr bool drag_out() const
  {
    return bits & Bits::DragOut;
  }

  constexpr bool drag_over() const
  {
    return bits & Bits::DragOver;
  }

  constexpr bool drop() const
  {
    return bits & Bits::Drop;
  }

  constexpr bool focus_in() const
  {
    return bits & Bits::FocusIn;
  }

  constexpr bool focus_out() const
  {
    return bits & Bits::FocusOut;
  }

  constexpr bool focus_over() const
  {
    return bits & Bits::FocusOver;
  }

  constexpr bool key_down() const
  {
    return bits & Bits::KeyDown;
  }

  constexpr bool key_up() const
  {
    return bits & Bits::KeyUp;
  }

  constexpr bool text_input() const
  {
    return bits & Bits::TextInput;
  }
};

struct FocusRect
{
  CRect area = {};
  CRect clip = {};
};

struct DropCtx
{
  enum class Phase : u8
  {
    None  = 0,
    Begin = 1,
    Over  = 2,
    End   = 3
  };

  Phase phase = Phase::None;

  /// @brief current drop data type
  DropType type = DropType::None;

  /// @brief drag data associated with the current drag operation (if any, otherwise empty)
  Vec<u8> data;

  explicit DropCtx(AllocatorRef allocator) : data{allocator}
  {
  }

  DropCtx(DropCtx const &)             = delete;
  DropCtx & operator=(DropCtx const &) = delete;
  DropCtx(DropCtx &&)                  = default;
  DropCtx & operator=(DropCtx &&)      = default;
  ~DropCtx()                           = default;

  void clear();

  DropCtx & copy(DropCtx const & other);
};

/// @brief Global View Context, Properties of the context all the views for
/// a specific window are in.
struct Ctx
{
  /// @brief timestamp of current frame
  time_point timestamp;

  /// @brief time elapsed between previous and current frame
  nanoseconds timedelta;

  WindowState window;

  /// @brief windows' current frame mouse state
  MouseState mouse;

  /// @brief windows' current frame keyboard state
  KeyState key;

  DropCtx drop;

  /// @brief is the application closing
  bool closing;

  /// @brief canvas-space region the system is currently focused on
  Option<FocusRect> focused;

  Option<Cursor> cursor;

  void * user_data = nullptr;

  Ctx(AllocatorRef allocator, void * user_data) :
    timestamp{},
    timedelta{},
    window{},
    mouse{},
    key{allocator},
    drop{allocator},
    closing{false},
    focused{none},
    cursor{none},
    user_data{user_data}
  {
  }

  Ctx(Ctx const &)             = delete;
  Ctx(Ctx &&)                  = default;
  Ctx & operator=(Ctx const &) = delete;
  Ctx & operator=(Ctx &&)      = default;
  ~Ctx()                       = default;

  void tick(InputState const & input);
};

/// @brief makes a zoom transform matrix relative to the center of a viewport.
/// defines the translation and scaling components.
/// @return zoom transform matrix

struct State
{
  /// @brief Tab Index for Focus-Based Navigation. desired tab index, `None`
  /// means the default tab order based on the hierarchy of the parent to
  /// children and siblings (depth-first traversal). Negative values are
  /// focused before positive values.
  Option<i32> tab = none;

  /// @brief if set, will be treated as a text input area
  Option<TextInputInfo> text = none;

  /// @brief if the view should be hidden from view (will not receive
  /// visual events, but still receive tick events)
  bool hidden : 1 = false;

  /// @brief can receive mouse enter/move/leave events
  bool pointable : 1 = false;

  /// @brief can receive mouse press events
  bool clickable : 1 = false;

  /// @brief can receive mouse scroll events
  bool scrollable : 1 = false;

  /// @brief can the view produce drag data
  bool draggable : 1 = false;

  /// @brief can the view receive drag data
  bool droppable : 1 = false;

  /// @brief can receive keyboard focus (ordered by `tab`) and keyboard events
  bool focusable : 1 = false;

  /// @brief grab focus of the user
  bool grab_focus : 1 = false;

  /// @brief is view a viewport
  bool viewport : 1 = false;

  /// @brief request the view system to defer shutdown to next frame
  bool defer_close : 1 = false;
};

struct Theme
{
  Vec4U8 background       = {};
  Vec4U8 surface          = {};
  Vec4U8 surface_variant  = {};
  Vec4U8 primary          = {};
  Vec4U8 primary_variant  = {};
  Vec4U8 error            = {};
  Vec4U8 warning          = {};
  Vec4U8 success          = {};
  Vec4U8 active           = {};
  Vec4U8 inactive         = {};
  Vec4U8 on_background    = {};
  Vec4U8 on_surface       = {};
  Vec4U8 on_primary       = {};
  Vec4U8 on_error         = {};
  Vec4U8 on_warning       = {};
  Vec4U8 on_success       = {};
  Vec4U8 focus            = {};
  Vec4U8 highlight        = {};
  Vec4U8 caret            = {};
  f32    head_font_height = {};
  f32    body_font_height = {};
  f32    line_height      = {};
  FontId head_font        = FontId::None;
  FontId body_font        = FontId::None;
  FontId icon_font        = FontId::None;
  void * user_data        = nullptr;
};

extern Theme theme;

struct Layout
{
  /// @brief extent of the view within the parent. if it is a viewport,
  /// this is the visible extent of the viewport within the parent viewport.
  Vec2 extent = {};

  /// @brief inner extent, if it is a viewport
  Vec2 viewport_extent = {};

  Vec2 viewport_center = {};

  Vec2 viewport_zoom = {1, 1};

  /// @brief viewport-space re-positioning of the view
  Option<Vec2> fixed_center = none;
};

enum class ViewId : u64
{
  None = U64_MAX
};

/// @brief Base view class.
/// Views are plain visual elements that define spatial relationships,
/// visual state changes, and forward events to other subsystems.
/// @note State changes must only happen in the `tick` method. Child view modifications
/// should be handled with it as well.
///
/// The coordinate system used is one in which the center of the screen is (0, 0) and
/// ranges from [-0.5w, +0.5w] on both axes. i.e. top-left is [-0.5w, -0.5h]
/// and bottom-right is [+0.5w, +0.5h].
struct View
{
  /// @brief id of the view if mounted, otherwise `ViewId::None`
  ViewId id_ = ViewId::None;

  bool hot_ = false;

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

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a subsystem that would
  /// handle it. i.e. using the multi-tasking or asset-loading systems.
  /// @param ctx the associated context of the previous frame
  /// @param events events due to the previous frame's state
  /// @param build callback to be called to insert subviews.
  constexpr virtual State tick(Ctx const & ctx, Events const & events,
                               Fn<void(View &)> build)
  {
    (void) ctx;
    (void) events;
    (void) build;
    return {};
  }

  /// @brief distributes the size allocated to it to its child views.
  /// @param allocated the size allocated to this view
  /// @param[out] sizes sizes allocated to the children.
  constexpr virtual void size(Vec2 allocated, Span<Vec2> sizes)
  {
    fill(sizes, allocated);
  }

  /// @brief fits itself around its children and positions child views
  /// relative to its center
  /// @param allocated the size allocated to this view
  /// @param sizes sizes of the child views
  /// @param[out] centers parent-space centers of the child views
  /// @return this view's fitted extent
  constexpr virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                               Span<Vec2> centers)
  {
    (void) allocated;
    (void) sizes;
    fill(centers, Vec2{0, 0});
    return {};
  }

  /// @brief returns the stacking layer index
  /// @param allocated stacking layer index allocated to this view
  /// by parent. This functions similar to the CSS stacking context. The layer
  /// index has a higher priority over the z-index and events do not bubble through it.
  /// @return stack index for the view
  constexpr virtual i32 layer(i32 allocated, Span<i32> indices)
  {
    fill(indices, allocated);
    return allocated;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param allocated z-index allocated to this view by parent
  /// @param[out] indices z-index assigned to children
  /// @return preferred z_index
  constexpr virtual i32 z_index(i32 allocated, Span<i32> indices)
  {
    fill(indices, allocated);
    return allocated;
  }

  /// @brief record draw commands needed to render this view. this method is
  /// only called if the view passes the visibility tests. this is called on
  /// every frame.
  /// @param canvas canvas to render view into
  /// @param viewport_region viewport-space region of the view (before zoom transform)
  /// @param canvas_region canvas-space region of the view (after zoom transform)
  /// @param zoom zoom scale of the view
  /// @param clip canvas-space clip of the view (after zoom transform)
  constexpr virtual void render(Canvas & canvas, CRect const & viewport_region,
                                CRect const & canvas_region, CRect const & clip)
  {
    (void) canvas;
    (void) viewport_region;
    (void) canvas_region;
    (void) clip;
  }

  /// @brief Select cursor type given a pointed region of the view.
  /// @param extent layout extent of the view
  /// @param position local-space position of the pointer
  /// @return preferred cursor type
  constexpr virtual Cursor cursor(Vec2 extent, Vec2 position)
  {
    (void) extent;
    (void) position;
    return Cursor::Default;
  }
};

}    // namespace ui
}    // namespace ash
