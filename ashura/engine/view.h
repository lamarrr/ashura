/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/canvas.h"
#include "ashura/engine/color.h"
#include "ashura/engine/input.h"
#include "ashura/engine/renderer.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
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
  Size width{};
  Size height{};

  constexpr Frame()                          = default;
  constexpr Frame(Frame const &)             = default;
  constexpr Frame(Frame &&)                  = default;
  constexpr Frame & operator=(Frame const &) = default;
  constexpr Frame & operator=(Frame &&)      = default;
  constexpr ~Frame()                         = default;

  constexpr Frame(Size width, Size height) : width{width}, height{height}
  {
  }

  constexpr Frame(f32 width, f32 height, bool constrain = true) :
    width{.offset = width, .rmax = constrain ? 1 : F32_INF},
    height{.offset = height, .rmax = constrain ? 1 : F32_INF}
  {
  }

  constexpr Vec2 operator()(Vec2 extent) const
  {
    return Vec2{width(extent.x), height(extent.y)};
  }

  constexpr Frame offset(f32 w, f32 h) const
  {
    Frame out{*this};
    out.width.offset  = w;
    out.height.offset = h;
    return out;
  }

  constexpr Frame scale(f32 w, f32 h) const
  {
    Frame out{*this};
    out.width.scale  = w;
    out.height.scale = h;
    return out;
  }

  constexpr Frame rmin(f32 w, f32 h) const
  {
    Frame out{*this};
    out.width.rmin  = w;
    out.height.rmin = h;
    return out;
  }

  constexpr Frame rmax(f32 w, f32 h) const
  {
    Frame out{*this};
    out.width.rmax  = w;
    out.height.rmax = h;
    return out;
  }

  constexpr Frame min(f32 w, f32 h) const
  {
    Frame out{*this};
    out.width.min  = w;
    out.height.min = h;
    return out;
  }

  constexpr Frame max(f32 w, f32 h) const
  {
    Frame out{*this};
    out.width.max  = w;
    out.height.max = h;
    return out;
  }
};

struct CornerRadii
{
  Size tl;
  Size tr;
  Size bl;
  Size br;

  constexpr CornerRadii() : tl{}, tr{}, bl{}, br{}
  {
  }

  constexpr CornerRadii(Size tl, Size tr, Size bl, Size br) :
    tl{tl},
    tr{tr},
    bl{bl},
    br{br}
  {
  }

  constexpr CornerRadii(Size s) : tl{s}, tr{s}, bl{s}, br{s}
  {
  }

  constexpr CornerRadii(f32 s, bool constrained) :
    CornerRadii{
      Size{.offset = s, .rmax = constrained ? 1 : F32_INF}
  }
  {
  }

  // [ ] this should be resolved on both width and height
  constexpr Vec4 operator()(f32 height) const
  {
    return Vec4{tl(height), tr(height), bl(height), br(height)};
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
/// @param drag_start drag event has begun on this view
/// @param dragging an update on the drag state has been gotten
/// @param drag_end the dragging of this view has completed
/// @param drag_in drag data has entered this view and might be dropped
/// @param drag_out drag data has left the view without being dropped
/// @param drag_over drag data is moving over this view as destination without
/// beiung dropped
/// @param drop drag data is now available for the view to consume
/// @param view_hit called on every frame the view is viewed on the viewport.
/// Can be used for partial loading
/// @param view_miss called on every frame that the view is not seen on the
/// viewport this can be because it has hidden visibility, is clipped away, or
/// parent positioned out of the visible region. Can be used for
/// partial unloading.
/// @param focus_in the view has received keyboard focus
/// @param focus_out the view has lost keyboard focus
/// @param text_input the view has received composition text
struct ViewEvents
{
  bool mounted      = false;
  bool view_hit     = false;
  bool mouse_in     = false;
  bool mouse_out    = false;
  bool mouse_down   = false;
  bool mouse_up     = false;
  bool mouse_moved  = false;
  bool mouse_scroll = false;
  bool drag_start   = false;
  bool dragging     = false;
  bool drag_end     = false;
  bool drag_in      = false;
  bool drag_out     = false;
  bool drag_over    = false;
  bool drop         = false;
  bool focus_in     = false;
  bool focus_out    = false;
  bool key_down     = false;
  bool key_up       = false;
  bool text_input   = false;
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

  /// @brief if the view should be hidden from view (will not receive
  /// visual events, but still receive tick events)
  bool hidden = false;

  /// @brief can receive mouse enter/move/leave events
  bool pointable = false;

  /// @brief can receive mouse press events
  bool clickable = false;

  /// @brief can receive mouse scroll events
  bool scrollable = false;

  /// @brief can the view produce drag data
  bool draggable = false;

  /// @brief can the view receive drag data
  bool droppable = false;

  /// @brief can receive keyboard focus (ordered by `tab`) and keyboard
  /// events
  bool focusable = false;

  /// @brief if set, will be treated as a text input area
  Option<TextInputInfo> text = none;

  /// @brief grab focus of the user
  bool grab_focus = false;

  /// @brief is view a viewport
  bool viewport = false;
};

struct CoreViewTheme
{
  Vec4 background        = {};
  Vec4 surface           = {};
  Vec4 surface_variant   = {};
  Vec4 primary           = {};
  Vec4 primary_variant   = {};
  Vec4 secondary         = {};
  Vec4 secondary_variant = {};
  Vec4 error             = {};
  Vec4 warning           = {};
  Vec4 success           = {};
  Vec4 active            = {};
  Vec4 inactive          = {};
  Vec4 on_background     = {};
  Vec4 on_surface        = {};
  Vec4 on_primary        = {};
  Vec4 on_secondary      = {};
  Vec4 on_error          = {};
  Vec4 on_warning        = {};
  Vec4 on_success        = {};
  f32  body_font_height  = {};
  f32  h1_font_height    = {};
  f32  h2_font_height    = {};
  f32  h3_font_height    = {};
  f32  line_height       = {};
  f32  focus_thickness   = 0;
};

// [ ] simpler color model:
// - primary color
// - text color
// - surface color
// - error, warning, success
inline constexpr CoreViewTheme DEFAULT_THEME = {
  .background        = norm(Vec4U8{0x19, 0x19, 0x19, 0xFF}),
  .surface           = norm(Vec4U8{0x33, 0x33, 0x33, 0xFF}),
  .primary           = norm(mdc::DEEP_ORANGE_600),
  .primary_variant   = norm(mdc::DEEP_ORANGE_400),
  .secondary         = norm(mdc::PURPLE_600),
  .secondary_variant = norm(mdc::PURPLE_400),
  .error             = norm(mdc::RED_500),
  .warning           = norm(mdc::YELLOW_800),
  .success           = norm(mdc::GREEN_700),
  .active            = norm(Vec4U8{0x70, 0x70, 0x70, 0xFF}),
  .inactive          = norm(Vec4U8{0x47, 0x47, 0x47, 0xFF}),
  .on_background     = norm(mdc::WHITE),
  .on_surface        = norm(mdc::WHITE),
  .on_primary        = norm(mdc::WHITE),
  .on_secondary      = norm(mdc::WHITE),
  .on_error          = norm(mdc::WHITE),
  .on_warning        = norm(mdc::WHITE),
  .on_success        = norm(mdc::WHITE),
  .body_font_height  = 16,
  .h1_font_height    = 30,
  .h2_font_height    = 27,
  .h3_font_height    = 22,
  .line_height       = 1.2F,
  .focus_thickness   = 1};

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
/// @param id id of the view if mounted, otherwise U64_MAX
/// @param last_rendered_frame last frame the view was rendered at
/// @param focus_idx index in the focus tree
/// @param region canvas-space region of the view
struct View
{
  u64   id_                  = U64_MAX;
  u64   last_rendered_frame_ = 0;
  u32   focus_idx_           = 0;
  CRect region_              = {};
  f32   zoom_                = 1;

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
                                CRect const & clip)
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

}    // namespace ash
