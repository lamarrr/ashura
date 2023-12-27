#pragma once
#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>

#include "ashura/canvas.h"
#include "ashura/context.h"
#include "ashura/event.h"
#include "ashura/primitives.h"
#include "ashura/uuid.h"
#include "stx/async.h"
#include "stx/fn.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"
#include <type_traits>

namespace ash
{

template <typename T, typename Base>
concept Impl = std::is_base_of_v<Base, T>;

struct EdgeInsets
{
  f32 left = 0, top = 0, right = 0, bottom = 0;

  static constexpr EdgeInsets all(f32 v)
  {
    return EdgeInsets{.left = v, .top = v, .right = v, .bottom = v};
  }

  static constexpr EdgeInsets horizontal(f32 v)
  {
    return EdgeInsets{.left = v, .top = 0, .right = v, .bottom = 0};
  }

  static constexpr EdgeInsets vertical(f32 v)
  {
    return EdgeInsets{.left = 0, .top = v, .right = 0, .bottom = v};
  }

  constexpr f32 y() const
  {
    return top + bottom;
  }

  constexpr f32 x() const
  {
    return left + right;
  }

  constexpr Vec2 xy() const
  {
    return Vec2{x(), y()};
  }

  constexpr Vec2 top_left() const
  {
    return Vec2{left, top};
  }
};

struct BorderRadius
{
  Constraint top_left, top_right, bottom_right, bottom_left;

  static constexpr BorderRadius relative(f32 tl, f32 tr, f32 br, f32 bl)
  {
    return BorderRadius{.top_left     = Constraint::relative(tl),
                        .top_right    = Constraint::relative(tr),
                        .bottom_right = Constraint::relative(br),
                        .bottom_left  = Constraint::relative(bl)};
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
    return BorderRadius{.top_left     = Constraint::absolute(tl),
                        .top_right    = Constraint::absolute(tr),
                        .bottom_right = Constraint::absolute(br),
                        .bottom_left  = Constraint::absolute(bl)};
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
    f32 const src = op::min(w, h) / 2;
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

enum class Direction : u8
{
  H = 0,        /// Horizontal
  V = 1         /// Vertical
};

enum class Wrap : u8
{
  None = 0,
  Wrap = 1
};

typedef Vec2 Alignment;

constexpr Alignment ALIGN_TOP_LEFT      = Vec2{0, 0};
constexpr Alignment ALIGN_TOP_CENTER    = Vec2{0.5f, 0};
constexpr Alignment ALIGN_TOP_RIGHT     = Vec2{1, 0};
constexpr Alignment ALIGN_LEFT_CENTER   = Vec2{0, 0.5f};
constexpr Alignment ALIGN_CENTER        = Vec2{0.5f, 0.5f};
constexpr Alignment ALIGN_RIGHT_CENTER  = Vec2{1, 0.5f};
constexpr Alignment ALIGN_BOTTOM_LEFT   = Vec2{0, 1};
constexpr Alignment ALIGN_BOTTOM_CENTER = Vec2{0.5f, 1};
constexpr Alignment ALIGN_BOTTOM_RIGHT  = Vec2{1, 1};

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
  None,
  Letter,
  Word,
  Line
};
// https://fossheim.io/writing/posts/css-text-gradient/
struct TextRenderStyle
{
  gfx::LinearColorGradient color_gradient;
  TextRenderStyleWrap      wrap = TextRenderStyleWrap::None;
};

enum class Visibility : u8
{
  Visible,
  Hidden
};

struct WidgetDebugInfo
{
  std::string_view type;
};

struct DragData
{
  stx::String                      type;
  stx::Unique<stx::Span<u8 const>> data =
      stx::Unique{stx::Span<u8 const>{}, stx::noop_manager};
};

// TODO(lamarrr): we might need request detach so child widgets can request to
// be removed and remove all callbacks they may have attached or cancel tasks
// they have pending.
// consider: having tokens that de-register themselves once deleted

/// @brief Base widget class. All widget types must inherit from this struct.
/// all methods are already implemented with reasonable defaults.
struct Widget
{
  Widget()
  {
  }

  virtual ~Widget()
  {
  }

  /// @brief get child widgets
  /// @param ctx
  /// @return
  virtual stx::Span<Widget *const> get_children(Context &ctx)
  {
    return {};
  }

  /// @brief get debug and logging information
  /// @param ctx
  /// @return
  virtual WidgetDebugInfo get_debug_info(Context &ctx)
  {
    return WidgetDebugInfo{.type = "Widget"};
  }

  // TODO(lamarrr): we need re-calculable offsets so we can shift the parents
  // around without shifting the children this is important for cursors, drag
  // and drop? this might mean we need to totally remove the concept of area.
  // storing transformed area might not be needed?

  /// @brief distributes the size allocated to it to its child widgets.
  /// unlike CSS. has the advantage that children wouldn't need extra attributes
  /// for specific kind of placements i.e. relative, absolute, etc.
  /// @param ctx
  /// @param allocated_size the size allocated to this widget
  /// @param[out] children_allocation sizes allocated to the children.
  virtual void allocate_size(Context &ctx, Vec2 allocated_size,
                             stx::Span<Vec2> children_allocation)
  {
    children_allocation.fill(Vec2{0, 0});
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
  virtual Vec2 fit(Context &ctx, Vec2 allocated_size,
                   stx::Span<Vec2 const> children_allocations,
                   stx::Span<Vec2 const> children_sizes,
                   stx::Span<Vec2>       children_positions)
  {
    return Vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the widget
  /// @param ctx
  /// @param allocated_position the allocated absolute position of this widget
  /// @return
  virtual Vec2 position(Context &ctx, Vec2 allocated_position)
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
  virtual Visibility get_visibility(Context              &ctx,
                                    Visibility            allocated_visibility,
                                    stx::Span<Visibility> children_allocation)
  {
    children_allocation.fill(allocated_visibility);
    return allocated_visibility;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param ctx
  /// @param allocated_z_index
  /// @param[out] children_allocation z-index assigned to children
  /// @return
  virtual i32 z_stack(Context &ctx, i32 allocated_z_index,
                      stx::Span<i32> children_allocation)
  {
    children_allocation.fill(allocated_z_index + 1);
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
  virtual Rect clip(Context &ctx, Rect allocated_clip,
                    stx::Span<Rect> children_allocation)
  {
    children_allocation.fill(allocated_clip);
    return allocated_clip;
  }

  /// @brief record draw commands needed to render this widget. this method is
  /// only called if the widget passes the visibility tests. this is called on
  /// every frame.
  /// @param ctx
  /// @param canvas
  ///
  ///
  virtual void draw(Context &ctx, gfx::Canvas &canvas)
  {
  }

  // TODO(lamarrr): draw_tooltip();

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a Subsystem that would
  /// handle that. i.e. using the multi-tasking system.
  /// @param ctx
  /// @param interval time passed since last call to this method
  virtual void tick(Context &ctx, std::chrono::nanoseconds interval)
  {
  }

  /// @brief called on every frame the widget is viewed on the viewport.
  /// @param ctx
  virtual void on_view_hit(Context &ctx)
  {
  }

  /// @brief called on every frame that the widget is not seen on the viewport
  /// this can be because it has hidden visibility, is clipped away, or parent
  /// positioned out of the visible region
  /// @param ctx
  virtual void on_view_miss(Context &ctx)
  {
  }

  // this needs to happen before mouse actions as some widgets .i.e. some
  // widgets don't need to intercept or receive mouse events
  virtual bool hit_test(Context &ctx, Vec2 mouse_position)
  {
    return false;
  }

  virtual bool scroll_test(Context &ctx)
  {
    return false;
  }

  virtual void on_mouse_down(Context &ctx, MouseButton button,
                             Vec2 mouse_position, u32 nclicks)
  {
  }

  virtual void on_mouse_up(Context &ctx, MouseButton button,
                           Vec2 mouse_position, u32 nclicks)
  {
  }

  // TODO(lamarrr): how do we fix translation and zooming? i.e. positioning once
  // transform is applied
  virtual void on_mouse_move(Context &ctx, Vec2 mouse_position,
                             Vec2 translation)
  {
  }

  virtual void on_mouse_enter(Context &ctx, Vec2 mouse_position)
  {
  }

  virtual void on_mouse_leave(Context &ctx, stx::Option<Vec2> mouse_position)
  {
  }

  // virtual bool on_mouse_wheel(Context& ctx, Vec2 translation, Vec2
  // mouse_position?). propagates up

  /// callback to begin drag operation
  /// if this returns false, it is treated as a click operation.???
  virtual stx::Option<DragData> on_drag_start(Context &ctx, Vec2 mouse_position)
  {
    return stx::None;
  }

  /// @brief called when theres a drag position update
  /// @param ctx
  /// @param mouse_position current global drag position
  /// @param translation difference between this drag update and the last drag
  /// update position
  virtual void on_drag_update(Context &ctx, Vec2 mouse_position,
                              Vec2 translation, DragData const &drag_data)
  {
  }

  /// the drop of the drag data has been ended
  virtual void on_drag_end(Context &ctx, Vec2 mouse_position)
  {
  }

  /// this widget has begun receiving drag data, i.e. it has been dragged onto
  ///
  /// returns true if widget can accept this drag event
  virtual void on_drag_enter(Context &ctx, DragData const &drag_data)
  {
  }

  /// this widget has previously begun receiving drag data, but the mouse is
  /// still dragging within it
  virtual void on_drag_over(Context &ctx, DragData const &drag_data)
  {
  }

  /// the drag event has left this widget
  virtual void on_drag_leave(Context &ctx, stx::Option<Vec2> mouse_position)
  {
  }

  /// drop of drag data on this widget
  virtual bool on_drop(Context &ctx, Vec2 mouse_position,
                       DragData const &drag_data)
  {
    return false;
  }

  //
  virtual void on_tap(Context &ctx)
  {
  }

  //
  virtual void on_touch_cancel(Context &ctx)
  {
  }

  //
  virtual void on_touch_end(Context &ctx)
  {
  }

  //
  virtual void on_touch_move(Context &ctx)
  {
  }

  //
  virtual void on_touch_start(Context &ctx)
  {
  }

  //
  virtual void on_touch_enter(Context &ctx)
  {
  }

  //
  virtual void on_touch_leave(Context &ctx)
  {
  }

  /// id used to recognise the widget. checked every frame. if
  /// one is not present or removed. a new uuid is generated and
  /// assigned.
  stx::Option<uuid> id;
  Rect              area;
};

inline Widget *__find_widget_recursive(Context &ctx, Widget &widget, uuid id)
{
  if (widget.id.contains(id))
  {
    return &widget;
  }

  for (Widget *child : widget.get_children(ctx))
  {
    Widget *found = __find_widget_recursive(ctx, *child, id);
    if (found != nullptr)
    {
      return found;
    }
  }

  return nullptr;
}

}        // namespace ash
