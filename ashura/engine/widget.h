#pragma once

#include "ashura/engine/key.h"
#include "ashura/std/option.h"
#include "ashura/std/rect.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"

namespace ash
{

/// Simple Layout Constraint Model
/// @bias: adding or subtracting from the source size, i.e. value should be
/// source size - 20px
/// @scale: scales the source size, i.e. value should be 0.5 of source size
/// @min: clamps the source size, i.e. value should be at least 20px
/// @max: clamps the source size, i.e. value should be at most 100px
/// @minr: clamps the source size relatively. i.e. value should be at least 0.5
/// of source size
/// @maxr: clamps the source size relatively. i.e. value should be at most 0.5
/// of source size
struct LayoutConstraint
{
  f32 bias  = 0;
  f32 scale = 0;
  f32 min   = F32_MIN;
  f32 max   = F32_MAX;
  f32 minr  = 0;
  f32 maxr  = 1;

  static constexpr LayoutConstraint relative(f32 scale)
  {
    return LayoutConstraint{.scale = scale};
  }

  static constexpr LayoutConstraint absolute(f32 value)
  {
    return LayoutConstraint{.bias = value};
  }

  constexpr LayoutConstraint with_min(f32 v) const
  {
    return LayoutConstraint{.bias  = bias,
                            .scale = scale,
                            .min   = v,
                            .max   = max,
                            .minr  = minr,
                            .maxr  = maxr};
  }

  constexpr LayoutConstraint with_max(f32 v) const
  {
    return LayoutConstraint{.bias  = bias,
                            .scale = scale,
                            .min   = min,
                            .max   = v,
                            .minr  = minr,
                            .maxr  = maxr};
  }

  constexpr LayoutConstraint with_minr(f32 v) const
  {
    return LayoutConstraint{.bias  = bias,
                            .scale = scale,
                            .min   = min,
                            .max   = max,
                            .minr  = v,
                            .maxr  = maxr};
  }

  constexpr LayoutConstraint with_maxr(f32 v) const
  {
    return LayoutConstraint{.bias  = bias,
                            .scale = scale,
                            .min   = min,
                            .max   = max,
                            .minr  = minr,
                            .maxr  = v};
  }

  constexpr f32 resolve(f32 value) const
  {
    return clamp(clamp(bias + value * scale, min, max), minr * value,
                 maxr * value);
  }
};

struct LayoutConstraint2D
{
  LayoutConstraint x, y;

  static constexpr LayoutConstraint2D relative(f32 x, f32 y)
  {
    return LayoutConstraint2D{.x = LayoutConstraint::relative(x),
                              .y = LayoutConstraint::relative(y)};
  }

  static constexpr LayoutConstraint2D relative(Vec2 xy)
  {
    return relative(xy.x, xy.y);
  }

  static constexpr LayoutConstraint2D absolute(f32 x, f32 y)
  {
    return LayoutConstraint2D{.x = LayoutConstraint::absolute(x),
                              .y = LayoutConstraint::absolute(y)};
  }

  static constexpr LayoutConstraint2D absolute(Vec2 xy)
  {
    return absolute(xy.x, xy.y);
  }

  constexpr LayoutConstraint2D with_min(f32 nx, f32 ny) const
  {
    return LayoutConstraint2D{.x = x.with_min(nx), .y = y.with_min(ny)};
  }

  constexpr LayoutConstraint2D with_max(f32 nx, f32 ny) const
  {
    return LayoutConstraint2D{.x = x.with_max(nx), .y = y.with_max(ny)};
  }

  constexpr LayoutConstraint2D with_minr(f32 nx, f32 ny) const
  {
    return LayoutConstraint2D{.x = x.with_minr(nx), .y = y.with_minr(ny)};
  }

  constexpr LayoutConstraint2D with_maxr(f32 nx, f32 ny) const
  {
    return LayoutConstraint2D{.x = x.with_maxr(nx), .y = y.with_maxr(ny)};
  }

  constexpr Vec2 resolve(f32 xsrc, f32 ysrc) const
  {
    return Vec2{x.resolve(xsrc), y.resolve(ysrc)};
  }

  constexpr Vec2 resolve(Vec2 src) const
  {
    return resolve(src.x, src.y);
  }
};

struct BorderRadius
{
  LayoutConstraint top_left, top_right, bottom_right, bottom_left;

  static constexpr BorderRadius relative(f32 tl, f32 tr, f32 br, f32 bl)
  {
    return BorderRadius{.top_left     = LayoutConstraint::relative(tl),
                        .top_right    = LayoutConstraint::relative(tr),
                        .bottom_right = LayoutConstraint::relative(br),
                        .bottom_left  = LayoutConstraint::relative(bl)};
  }

  static constexpr BorderRadius relative(Vec4 v)
  {
    return relative(v.x, v.y, v.z, v.w);
  }

  static constexpr BorderRadius relative(f32 v)
  {
    return relative(v, v, v, v);
  }

  static constexpr BorderRadius absolute(f32 tl, f32 tr, f32 br, f32 bl)
  {
    return BorderRadius{.top_left     = LayoutConstraint::absolute(tl),
                        .top_right    = LayoutConstraint::absolute(tr),
                        .bottom_right = LayoutConstraint::absolute(br),
                        .bottom_left  = LayoutConstraint::absolute(bl)};
  }

  static constexpr BorderRadius absolute(Vec4 v)
  {
    return absolute(v.x, v.y, v.z, v.w);
  }

  static constexpr BorderRadius absolute(f32 v)
  {
    return absolute(v, v, v, v);
  }

  constexpr Vec4 resolve(f32 w, f32 h) const
  {
    f32 src = min(w, h) / 2;
    return Vec4{.x = top_left.resolve(src),
                .y = top_right.resolve(src),
                .z = bottom_right.resolve(src),
                .w = bottom_left.resolve(src)};
  }

  constexpr Vec4 resolve(Vec2 wh) const
  {
    return resolve(wh.x, wh.y);
  }
};

/// @H: horizontal
/// @V: vertical
enum class Direction : u8
{
  H = 0,
  V = 1
};

constexpr Vec2 ALIGN_TOP_LEFT      = Vec2{0, 0};
constexpr Vec2 ALIGN_TOP_CENTER    = Vec2{0.5f, 0};
constexpr Vec2 ALIGN_TOP_RIGHT     = Vec2{1, 0};
constexpr Vec2 ALIGN_LEFT_CENTER   = Vec2{0, 0.5f};
constexpr Vec2 ALIGN_CENTER        = Vec2{0.5f, 0.5f};
constexpr Vec2 ALIGN_RIGHT_CENTER  = Vec2{1, 0.5f};
constexpr Vec2 ALIGN_BOTTOM_LEFT   = Vec2{0, 1};
constexpr Vec2 ALIGN_BOTTOM_CENTER = Vec2{0.5f, 1};
constexpr Vec2 ALIGN_BOTTOM_RIGHT  = Vec2{1, 1};

enum class MainAlign : u8
{
  Start        = 0,
  End          = 1,
  SpaceBetween = 2,
  SpaceAround  = 3,
  SpaceEvenly  = 4
};

enum class CrossAlign : u8
{
  Start  = 0,
  End    = 1,
  Center = 2
};

enum class TextRenderStyleWrap : u8
{
  None   = 0,
  Letter = 1,
  Word   = 2,
  Line   = 3
};

// https://fossheim.io/writing/posts/css-text-gradient/
struct TextRenderStyle
{
  // gfx::LinearColorGradient color_gradient;
  TextRenderStyleWrap wrap = TextRenderStyleWrap::None;
};

struct WidgetDebugInfo
{
  Span<char const> type;
};

typedef struct WidgetSystem WidgetSystem;

// TODO(lamarrr): we might need request detach so child widgets can request to
// be removed and remove all callbacks they may have attached or cancel tasks
// they have pending.
// consider: having tokens that de-register themselves once deleted

// TODO(lamarrr): we need re-calculable offsets so we can shift the parents
// around without shifting the children this is important for cursors, drag
// and drop? this might mean we need to totally remove the concept of area.
// storing transformed area might not be needed?

/// @brief Base widget class. All widget types must inherit from this struct.
/// all methods are already implemented with reasonable defaults.
struct Widget
{
  Widget()          = default;
  virtual ~Widget() = default;

  /// @brief get child widgets
  /// @param ctx
  /// @return
  virtual Span<Widget *const> get_children(WidgetSystem &);

  /// @brief get debug and logging information
  /// @param ctx
  /// @return
  virtual WidgetDebugInfo get_debug_info(WidgetSystem &)
  {
    return WidgetDebugInfo{.type = {"Widget", 6}};
  }

  /// @brief distributes the size allocated to it to its child widgets.
  /// unlike CSS. has the advantage that children wouldn't need extra attributes
  /// for specific kind of placements i.e. relative, absolute, etc.
  /// @param ctx
  /// @param allocated_size the size allocated to this widget
  /// @param[out] children_allocation sizes allocated to the children.
  virtual void allocate_size(WidgetSystem &, Vec2 allocated_size,
                             Span<Vec2> children_allocation)
  {
    (void) allocated_size;
    fill(children_allocation, Vec2{0, 0});
  }

  /// @brief fits itself around its children and positions child widgets
  /// along/relative to itself (i.e. position {0, 0} means the child will be
  /// placed on the top left of the parent)
  /// @param ctx
  /// @param allocated_size the size allocated to this widget. the widget can
  /// decide to disregard or fit to this as needed.
  /// @param children_sizes sizes of the child widgets
  /// @param[out] children_positions positions of the children widget on the
  /// parent
  /// @return this widget's fitted extent
  virtual Vec2 fit(WidgetSystem &, Vec2 allocated_size,
                   Span<Vec2 const> children_allocations,
                   Span<Vec2 const> children_sizes,
                   Span<Vec2>       children_positions)
  {
    (void) allocated_size;
    (void) children_allocations;
    (void) children_sizes;
    (void) children_positions;
    return Vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the widget
  /// @param ctx
  /// @param allocated_position the allocated absolute position of this widget
  /// @return
  virtual Vec2 position(WidgetSystem &, Vec2 allocated_position)
  {
    return allocated_position;
  }

  /// @brief returns the visibility of this widget. an invisible widget will not
  /// be drawn nor receive mouse/touch events. parents can decide the visibility
  /// of eacg child
  /// @param ctx
  /// @param allocated_visibility
  /// @param[out] children_allocation visibility assigned to children
  /// @return
  virtual bool get_visibility(WidgetSystem &, bool allocated_visibility,
                              Span<bool> children_allocation)
  {
    fill(children_allocation, allocated_visibility);
    return allocated_visibility;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param ctx
  /// @param allocated_z_index
  /// @param[out] children_allocation z-index assigned to children
  /// @return
  virtual i32 z_stack(WidgetSystem &, i32 allocated_z_index,
                      Span<i32> children_allocation)
  {
    fill(children_allocation, allocated_z_index + 1);
    return allocated_z_index;
  }

  /// @brief this is used for clipping widget views. the provided clip is
  /// relative to the root widget's axis (0, 0). this can be used for nested
  /// viewports where there are multiple intersecting clips. transforms do not
  /// apply to the clip rects. this is used for visibility testing and
  /// eventually actual vertex culling. a nested viewport for example can
  /// therefore use the intersection of its allocated clip and it's own viewport
  /// clip and assign that to its children, whilst using the allocated clip on
  /// itself.
  /// @param ctx
  /// @param allocated_clip
  /// @param[out] children_allocation
  /// @return
  virtual Rect clip(WidgetSystem &, Rect allocated_clip,
                    Span<Rect> children_allocation)
  {
    fill(children_allocation, allocated_clip);
    return allocated_clip;
  }

  /// @brief record draw commands needed to render this widget. this method is
  /// only called if the widget passes the visibility tests. this is called on
  /// every frame.
  /// @param ctx
  /// @param canvas
  ///
  ///
  virtual void draw(WidgetSystem &);
  // TODO(lamarrr): draw_tooltip();

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a Subsystem that would
  /// handle that. i.e. using the multi-tasking system.
  /// @param ctx
  /// @param interval time passed since last call to this method
  virtual void tick(WidgetSystem &, Nanoseconds interval);

  /// @brief called on every frame the widget is viewed on the viewport.
  /// @param ctx
  virtual void on_view_hit(WidgetSystem &);

  /// @brief called on every frame that the widget is not seen on the viewport
  /// this can be because it has hidden visibility, is clipped away, or parent
  /// positioned out of the visible region
  /// @param ctx
  virtual void on_view_miss(WidgetSystem &);

  // this needs to happen before mouse actions as some widgets .i.e. some
  // widgets don't need to intercept or receive mouse events
  /// return true if accepts hit test at position.
  virtual bool hit_test(WidgetSystem &, Vec2 mouse_position);

  /// return true if accepts scroll at position
  virtual bool scroll_test(WidgetSystem &);

  virtual void on_mouse_down(WidgetSystem &, MouseButtons button,
                             Vec2 mouse_position, u32 nclicks);

  virtual void on_mouse_up(WidgetSystem &, MouseButtons button,
                           Vec2 mouse_position, u32 nclicks);

  // TODO(lamarrr): how do we fix translation and zooming? i.e. positioning once
  // transform is applied
  virtual void on_mouse_move(WidgetSystem &, Vec2 mouse_position,
                             Vec2 translation);

  virtual void on_mouse_enter(WidgetSystem &, Vec2 mouse_position);

  virtual void on_mouse_leave(WidgetSystem &, Option<Vec2> mouse_position);

  // virtual bool on_mouse_wheel(Context& ctx, Vec2 translation, Vec2
  // mouse_position?). propagates up
  ///
  /// callback to begin drag operation
  /// if this returns false, it is treated as a click operation.???
  virtual Span<char const> on_drag_start(WidgetSystem &, Vec2 mouse_position);

  /// @brief called when theres a drag position update
  /// @param ctx
  /// @param mouse_position current global drag position
  /// @param translation difference between this drag update and the last drag
  /// update position
  virtual void on_drag_update(WidgetSystem &, Vec2 mouse_position,
                              Vec2 translation, Span<u8 const> drag_data);

  /// the drop of the drag data has been ended
  virtual void on_drag_end(WidgetSystem &, Vec2 mouse_position);

  /// this widget has begun receiving drag data, i.e. it has been dragged onto
  ///
  /// returns true if widget can accept this drag event
  virtual void on_drag_enter(WidgetSystem &, Span<u8 const> drag_data);

  /// this widget has previously begun receiving drag data, but the mouse is
  /// still dragging within it
  virtual void on_drag_over(WidgetSystem &, Span<u8 const> drag_data);

  /// the drag event has left this widget
  virtual void on_drag_leave(WidgetSystem &, Option<Vec2> mouse_position);

  /// drop of drag data on this widget, return true if accept drag data
  virtual bool on_drop(WidgetSystem &, Vec2 mouse_position,
                       Span<u8 const> drag_data);

  virtual void on_tap(WidgetSystem &);

  virtual void on_touch_cancel(WidgetSystem &);

  virtual void on_touch_end(WidgetSystem &);

  virtual void on_touch_move(WidgetSystem &);

  virtual void on_touch_start(WidgetSystem &);

  virtual void on_touch_enter(WidgetSystem &);

  virtual void on_touch_leave(WidgetSystem &);

  /// id used to recognise the widget. checked every frame. if
  /// one is not present or removed. a new uuid is generated and
  /// assigned.
  Option<uid> id;
  Rect        area;
};

}        // namespace ash
