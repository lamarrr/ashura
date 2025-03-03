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

/// @param mounted view has been mounted to the view tree and has now
/// received an ID.
/// @param view_hit if the view was rendered on the previous frame
/// @param drag_start drag event has begun on this view
/// @param dragging an update on the drag state has been gotten
/// @param drag_end the dragging of this view has completed
/// @param drag_in drag data has entered this view and might be dropped
/// @param drag_out drag data has left the view without being dropped
/// @param drag_over drag data is moving over this view as destination without
/// beiung dropped
/// @param drop drag data is now available for the view to consume
/// @param view_miss called on every frame that the view is not seen on the
/// viewport this can be because it has hidden visibility, is clipped away, or
/// parent positioned out of the visible region. Can be used for
/// partial unloading.
/// @param focus_in the view has received keyboard focus
/// @param focus_out the view has lost keyboard focus
/// @param text_input the view has received composition text
struct alignas(u32) ViewEvents
{
  bool mounted      : 1 = false;
  bool view_hit     : 1 = false;
  bool mouse_in     : 1 = false;
  bool mouse_out    : 1 = false;
  bool mouse_down   : 1 = false;
  bool mouse_up     : 1 = false;
  bool mouse_moved  : 1 = false;
  bool mouse_scroll : 1 = false;
  bool drag_start   : 1 = false;
  bool dragging     : 1 = false;
  bool drag_end     : 1 = false;
  bool drag_in      : 1 = false;
  bool drag_out     : 1 = false;
  bool drag_over    : 1 = false;
  bool drop         : 1 = false;
  bool focus_in     : 1 = false;
  bool focus_out    : 1 = false;
  bool key_down     : 1 = false;
  bool key_up       : 1 = false;
  bool text_input   : 1 = false;
};

/// @brief Global View Context, Properties of the context all the views for
/// a specific window are in.
using ViewContext = InputState;

/// @brief makes a zoom transform matrix relative to the center of a viewport.
/// defines the translation and scaling components.
/// @return zoom transform matrix
constexpr Affine3 scroll_transform(Vec2 viewport_extent, Vec2 view_extent,
                                   Vec2 t, f32 scale)
{
  Vec2 const low    = -0.5F * viewport_extent + 0.5F * view_extent;
  Vec2 const high   = 0.5F * viewport_extent - 0.5F * view_extent;
  Vec2 const center = lerp(low, high, t);
  return translate2d(center * scale) * scale2d(Vec2::splat(scale));
}

struct ViewState
{
  /// @brief Tab Index for Focus-Based Navigation. desired tab index, I32_MIN
  /// meaning the default tab order based on the hierarchy of the parent to
  /// children and siblings (depth-first traversal). Negative values are
  /// focused before positive values.
  i32 tab = I32_MIN;

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

  /// @brief can receive keyboard focus (ordered by `tab`) and keyboard
  /// events
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
  f32    head_font_height = {};
  f32    body_font_height = {};
  f32    line_height      = {};
  f32    focus_thickness  = 1;
  FontId head_font        = FontId::Invalid;
  FontId body_font        = FontId::Invalid;
  FontId icon_font        = FontId::Invalid;
};

extern Theme theme;

/// @param extent extent of the view within the parent. if it is a viewport,
/// this is the visible extent of the viewport within the parent viewport.
/// @param viewport_extent inner extent, if it is a viewport
/// @param viewport_transform the transform a viewport applies to its contained
/// views, this is recursively applied to contained views.
/// @param fixed_position the canvas-space re-positioning of the view
struct ViewLayout
{
  Vec2         extent             = {};
  Vec2         viewport_extent    = {};
  Affine3      viewport_transform = Affine3::identity();
  Option<Vec2> fixed_position     = none;
};

/// @brief Base view class. All view types must inherit from this struct.
/// Views are plain visual elements that define spatial relationships,
/// visual state changes, and forward events to other subsystems.
/// @note State changes must only happen in the `tick` method. for child view
/// switching, it should be handled by a flag in the tick method and switch in
/// the child method based on the flag.
///
/// The coordinate system used is one in which the center of the screen is (0,
/// 0) and ranges from [-0.5w, +0.5w] on both axes. i.e. top-left is [-0.5w,
/// -0.5h] and bottom-right is [+0.5w, +0.5h].
struct View
{
  /// @brief id of the view if mounted, otherwise U64_MAX
  u64 id_ = U64_MAX;

  /// @brief id of last frame the view was rendered on
  u64 last_rendered_frame_ = 0;

  /// @brief index in the focus tree
  u32 focus_idx_ = 0;

  /// @brief zoom scale of the views
  f32 zoom_ = 1;

  /// @brief canvas-space region of the view
  CRect region_ = {};

  constexpr View()                         = default;
  constexpr View(View const &)             = delete;
  constexpr View(View &&)                  = delete;
  constexpr View & operator=(View const &) = delete;
  constexpr View & operator=(View &&)      = delete;
  constexpr virtual ~View()                = default;

  /// @returns the ID currently allocated to the view or U64_MAX
  constexpr u64 id() const
  {
    return id_;
  }

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a subsystem that would
  /// handle it. i.e. using the multi-tasking or asset-loading systems.
  /// @param region canvas-space region the view is on
  /// @param build callback to be called to insert subviews.
  constexpr virtual ViewState tick(ViewContext const & ctx,
                                   CRect const & region, f32 zoom,
                                   ViewEvents const & events,
                                   Fn<void(View &)>   build)
  {
    (void) ctx;
    (void) region;
    (void) zoom;
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
  constexpr virtual ViewLayout fit(Vec2 allocated, Span<Vec2 const> sizes,
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
  /// index has a higher priority over the z-index.
  /// @return stack index for the view
  constexpr virtual i32 stack(i32 allocated)
  {
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
  /// @param region canvas-space region of the view
  /// @param zoom zoom scale of the view
  /// @param clip canvas-space clip of the view, applied by viewports.
  /// @param canvas canvas to render view into
  constexpr virtual void render(Canvas & canvas, CRect const & region, f32 zoom,
                                Rect const & clip)
  {
    (void) canvas;
    (void) region;
    (void) zoom;
    (void) clip;
  }

  /// @brief Used for hit-testing regions of views.
  /// @param region canvas-space region of the view
  /// @param position canvas-space position of the pointer
  /// @return true if in hit region
  constexpr virtual bool hit(CRect const & region, f32 zoom, Vec2 position)
  {
    (void) region;
    (void) zoom;
    (void) position;
    return true;
  }

  /// @brief Select cursor type given a pointed region of the view.
  /// @param region canvas-space region of the view
  /// @param position canvas-space position of the pointer
  constexpr virtual Cursor cursor(CRect const & region, f32 zoom, Vec2 position)
  {
    (void) region;
    (void) zoom;
    (void) position;
    return Cursor::Default;
  }

  /// @brief Called when the viewport is needed to zoom itself, scaling its
  /// inner extent
  /// @param zoom zoom to apply to the inner extent
  constexpr virtual void zoom(Affine3 const & transform)
  {
    (void) transform;
  }
};

}    // namespace ui
}    // namespace ash
