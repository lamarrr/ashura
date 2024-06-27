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

/// Simple Layout Constraint Model
/// @param offset adding or subtracting from the source size, i.e. value should
/// be source size - 20px
/// @param scale scales the source size, i.e. value should be 0.5 of source
/// size
/// @param min clamps the source size, i.e. value should be at least 20px
/// @param max clamps the source size, i.e. value should be at most 100px
/// @param minrel clamps the source size relatively. i.e. value should be at
/// least 0.5 of source size
/// @param maxrel clamps the source size relatively. i.e. value should be at
/// most 0.5 of source size
struct SizeConstraint
{
  f32 offset = 0;
  f32 scale  = 0;
  f32 min    = F32_MIN;
  f32 max    = F32_MAX;
  f32 minrel = 0;
  f32 maxrel = 1;

  constexpr f32 resolve(f32 value) const
  {
    return clamp(clamp(offset + value * scale, min, max), minrel * value,
                 maxrel * value);
  }
};

enum class Axis : u8
{
  Horizontal = 0,
  Vertical   = 1
};

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

/// @param ViewHit called on every frame the widget is viewed on the viewport.
/// @param ViewMiss called on every frame that the widget is not seen on the
/// viewport this can be because it has hidden visibility, is clipped away, or
/// parent positioned out of the visible region
enum class WidgetEventTypes : u32
{
  None         = 0x00000000,
  MouseDown    = 0x00000001,
  MouseUp      = 0x00000002,
  MouseMove    = 0x00000004,
  MouseEnter   = 0x00000008,
  MouseEscaped = 0x00000010,
  MouseLeave   = 0x00000020,
  MouseScroll  = 0x00000040,
  DragStart    = 0x00000080,
  DragUpdate   = 0x00000100,
  DragEnd      = 0x00000200,
  DragEnter    = 0x00000400,
  DragOver     = 0x00000800,
  DragLeave    = 0x00001000,
  Drop         = 0x00002000,
  ViewHit      = 0x00004000,
  ViewMiss     = 0x00008000
};

ASH_DEFINE_ENUM_BIT_OPS(WidgetEventTypes)

/// @param Visible if the widget is visible or not. Visibility propagates down
/// to the children
enum class WidgetAttributes : u8
{
  None       = 0x00,
  Visible    = 0x01,
  Clickable  = 0x02,
  Scrollable = 0x04,
  Draggable  = 0x08
};

ASH_DEFINE_ENUM_BIT_OPS(WidgetAttributes)

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
};

/// @brief Base widget class. All widget types must inherit from this struct.
/// Widgets are plain visual elements that define spatial relationships,
/// visual state changes, and forward events to other subsystems.
struct Widget
{
  Widget()          = default;
  virtual ~Widget() = default;

  /// @brief get child widgets, this is a virtual iterator, return null once
  /// there's no other children
  /// @return
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
  virtual Vec2 position(Vec2 center, Vec2 extent)
  {
    (void) extent;
    return center;
  }

  /// @brief Used for hit-testing regions of widgets.
  /// @param area area of widget within the viewport
  /// @param offset offset of pointer within area
  /// @return
  virtual bool hit(Vec2 center, Vec2 extent, Vec2 offset)
  {
    (void) center;
    (void) extent;
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
  virtual CRect clip(Vec2 center, Vec2 extent, CRect allocated)
  {
    (void) center;
    (void) extent;
    return allocated;
  }

  /// @brief record draw commands needed to render this widget. this method is
  /// only called if the widget passes the visibility tests. this is called on
  /// every frame.
  /// @param canvas
  virtual void render(Vec2 center, Vec2 extent, Canvas &canvas)
  {
    (void) center;
    (void) extent;
    (void) canvas;
  }

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a Subsystem that would
  /// handle that. i.e. using the multi-tasking system.
  /// @param dt time passed since last call to this method
  //
  virtual void tick(WidgetContext const &ctx, nanoseconds dt,
                    WidgetEventTypes events)
  {
    (void) ctx;
    (void) dt;
    (void) events;
  }

  uid id = UID_MAX;
};

}        // namespace ash
