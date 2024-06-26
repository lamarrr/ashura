#pragma once

#include "ashura/engine/canvas.h"
#include "ashura/engine/event.h"
#include "ashura/engine/key.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/text.h"
#include "ashura/std/math.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"

namespace ash
{

/// Simple Adaptive Layout Constraint Model
/// @param offset adding or subtracting from the source size, i.e. value should
/// be source size - 20px
/// @param scale scales the source size, i.e. value should be 0.5 of source
/// size
/// @param min clamps the source size, i.e. value should be at least 20px
/// @param max clamps the source size, i.e. value should be at most 100px
/// @param rmin  clamps the source size relatively. i.e. value should be at
/// least 0.5 of source size
/// @param rmax  clamps the source size relatively. i.e. value should be at
/// most 0.5 of source size
struct SizeConstraint
{
  f32 offset = 0;
  f32 scale  = 0;
  f32 min    = F32_MIN;
  f32 max    = F32_MAX;
  f32 rmin   = 0;
  f32 rmax   = 1;

  constexpr f32 resolve(f32 value) const
  {
    return clamp(clamp(offset + value * scale, min, max), rmin * value,
                 rmax * value);
  }
};

enum class Axis : u8
{
  X = 0,
  Y = 1,
  Z = 2,
  W = 3
};

enum class Axes : u8
{
  None = 0x00,
  X    = 0x01,
  Y    = 0x02,
  Z    = 0x04,
  W    = 0x08
};

ASH_DEFINE_ENUM_BIT_OPS(Axes)

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

/// @param DragStart drag event has begun on this widget
/// @param DragUpdate the mouse has been moved whilst this widget is being
/// dragged
/// @param DragEnd the dragging of this widget has completed
/// @param DragEnter drag data has entered this widget and might be dropped
/// @param DragLeave drag data has left the widget without being dropped
/// @param Drop drag data is now available for the widget to consume
/// @param ViewHit called on every frame the widget is viewed on the viewport.
/// Can be used for partial loading
/// @param ViewMiss called on every frame that the widget is not seen on the
/// viewport this can be because it has hidden visibility, is clipped away, or
/// parent positioned out of the visible region. Can be used for
/// partial unloading.
enum class WidgetEventTypes : u64
{
  None         = 0x0000000000000000ULL,
  MouseDown    = 0x0000000000000001ULL,
  MouseUp      = 0x0000000000000002ULL,
  MousePressed = 0x0000000000000004ULL,
  MouseMove    = 0x0000000000000008ULL,
  MouseEnter   = 0x0000000000000010ULL,
  MouseEscaped = 0x0000000000000020ULL,
  MouseLeave   = 0x0000000000000040ULL,
  MouseScroll  = 0x0000000000000080ULL,
  DragStart    = 0x0000000000000100ULL,
  DragUpdate   = 0x0000000000000200ULL,
  DragEnd      = 0x0000000000000400ULL,
  DragEnter    = 0x0000000000000800ULL,
  DragOver     = 0x0000000000001000ULL,
  DragLeave    = 0x0000000000002000ULL,
  Drop         = 0x0000000000004000ULL,
  ViewHit      = 0x0000000000008000ULL,
  ViewMiss     = 0x0000000000010000ULL,
  FocusIn      = 0x0000000000020000ULL,
  FocusOut     = 0x0000000000040000ULL
};

ASH_DEFINE_ENUM_BIT_OPS(WidgetEventTypes)

/// @param Visible if the widget is visible or not. Visibility propagates down
/// to the children
/// @param Draggable if the widget can receive drag events
/// @param Droppable if the widget can receive drop events
enum class WidgetAttributes : u32
{
  None       = 0x00000000U,
  Visible    = 0x00000001U,
  Clickable  = 0x00000002U,
  Scrollable = 0x00000004U,
  Draggable  = 0x00000008U,
  Droppable  = 0x00000010U,
  Focusable  = 0x00000020U,
};

ASH_DEFINE_ENUM_BIT_OPS(WidgetAttributes)

// TODO(lamarrr): time-based debouncing of keyboard pressing
// IME-text input
// key down, key up, key pressed
// window mouse and key focus should propagate to tree AND focus or unfocus
// widgets SDL_StartTextInput
// TODO(lamarrr): color space, pixel info for color pickers
//
//
//
struct WidgetContext
{
  MouseButtons   button                  = MouseButtons::None;
  Vec2           mouse_position          = {};
  Vec2           mouse_translation       = {};
  u32            num_clicks              = 0;
  Vec2           mouse_wheel_translation = {};
  Span<u8 const> drag_payload            = {};
  SystemTheme    theme                   = SystemTheme::None;
  TextDirection  direction               = TextDirection::LeftToRight;
  u8             key_states[NUM_KEYS]    = {};
};

// TODO(lamarrr): spatial navigation model
/// use spatial testing and scrolling information instead
/// when moved, move to the closest non-obscured one. clipping? CHILDREN
/// navigatable but not visible. as in imgui.
///
/// todo(lamar): scroll on child focus
/// https://github.com/ocornut/imgui/issues/787#issuecomment-361419796 enter
/// parent: prod children, nav to children
/// https://user-images.githubusercontent.com/8225057/74143829-ce67b900-4bfb-11ea-90d9-0de40c944b26.gif
/// clicking with enter keyboard when focused
/// @brief Given child index i, based on the spatial structure of this widget,
/// return the next child that will be focused. i starts from U32_MAX. with
/// U32_MAX meaning none of the children has been focused. It's the
/// responsibility of the parent to layout and provide spatial indices for its
/// children.

/// @brief Base widget class. All widget types must inherit from this struct.
/// Widgets are plain visual elements that define spatial relationships,
/// visual state changes, and forward events to other subsystems.
struct Widget
{
  uid id = UID_MAX;

  Widget()          = default;
  virtual ~Widget() = default;

  /// @brief get child widgets, this is a virtual iterator, return null once
  /// there's no other children
  /// @param i child index
  /// @return child widget pointer or nullptr meaning no more child left.
  virtual Widget *child(u32 i)
  {
    (void) i;
    return nullptr;
  }

  /// @brief distributes the size allocated to it to its child widgets.
  /// @param allocated the size allocated to this widget
  /// @param[out] sizes sizes allocated to the children.
  virtual void size(Vec2 allocated, Span<Vec2> sizes)
  {
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  /// @brief fits itself around its children and positions child widgets
  /// relative to its center
  /// @param allocated the size allocated to this widget
  /// @param sizes sizes of the child widgets
  /// @param[out] offsets offsets of the widgets from the parent's center
  /// @return this widget's fitted extent
  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes, Span<Vec2> offsets)
  {
    (void) allocated;
    (void) sizes;
    fill(offsets, Vec2{0, 0});
    return Vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the widget
  /// @param center the allocated absolute center of this widget relative
  /// to the viewport
  virtual Vec2 position(CRect const &region)
  {
    return region.center;
  }

  /// @brief Used for hit-testing regions of widgets.
  /// @param area area of widget within the viewport
  /// @param offset offset of pointer within area
  /// @return
  virtual bool hit(CRect const &region, Vec2 offset)
  {
    (void) region;
    (void) offset;
    return true;
  }

  /// @brief Used for visibility, scroll, and drag testing
  /// @return
  virtual WidgetAttributes attributes()
  {
    return WidgetAttributes::Visible;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param z_index z-index allocated to this widget by parent
  /// @param[out] allocation z-index assigned to children
  /// @return
  virtual i32 stack(i32 z_index, Span<i32> allocation)
  {
    fill(allocation, z_index + 1);
    return z_index;
  }

  /// @brief this is used for clipping widget views. the provided clip is
  /// relative to the root viewport. Used for nested viewports where there are
  /// multiple intersecting clips.
  virtual CRect clip(CRect const &region, CRect const &allocated)
  {
    (void) region;
    return allocated;
  }

  /// @brief record draw commands needed to render this widget. this method is
  /// only called if the widget passes the visibility tests. this is called on
  /// every frame.
  /// @param canvas
  virtual void render(CRect const &region, Canvas &canvas)
  {
    (void) region;
    (void) canvas;
  }

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a Subsystem that would
  /// handle that. i.e. using the multi-tasking system.
  /// @param dt time passed since last call to this method
  //
  virtual void tick(WidgetContext const &ctx, CRect const &region,
                    nanoseconds dt, WidgetEventTypes events)
  {
    (void) ctx;
    (void) dt;
    (void) events;
  }
};

template <usize N>
constexpr Widget *child_array(Widget *const (&children)[N], u32 i)
{
  return i < N ? children[i] : nullptr;
}

constexpr Widget *child_span(Span<Widget *const> children, u32 i)
{
  return i < children.size() ? children[i] : nullptr;
}

}        // namespace ash
