#pragma once

#include "ashura/engine/event.h"
#include "ashura/engine/key.h"
#include "ashura/renderer/renderer.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// Simple Layout Constraint Model
/// @offset: adding or subtracting from the source size, i.e. value should be
/// source size - 20px
/// @scale: scales the source size, i.e. value should be 0.5 of source size
/// @min: clamps the source size, i.e. value should be at least 20px
/// @max: clamps the source size, i.e. value should be at most 100px
/// @minrel: clamps the source size relatively. i.e. value should be at least 0.5
/// of source size
/// @maxrel: clamps the source size relatively. i.e. value should be at most 0.5
/// of source size
struct LayoutConstraint
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

enum class Direction : u8
{
  Horizontal = 0,
  Vertical   = 1
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

/// @ViewHit: called on every frame the widget is viewed on the viewport.
/// @ViewMiss: called on every frame that the widget is not seen on the viewport
/// this can be because it has hidden visibility, is clipped away, or parent
/// positioned out of the visible region
enum class WidgetEventTypes : u32
{
  None         = 0x00000000,
  MouseDown    = 0x00000001,
  MouseUp      = 0x00000002,
  MouseMove    = 0x00000004,
  MouseEnter   = 0x00000008,
  MouseEscaped = 0x00000010,
  MouseLeave   = 0x00000020,
  MouseWheel   = 0x00000040,
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

struct GlobalWidgetState
{
  MouseButtons   button            = MouseButtons::None;
  Vec2           mouse_position    = {};
  Vec2           mouse_translation = {};
  u32            num_clicks        = 0;
  Span<u8 const> drag_payload      = {};
  SystemTheme    theme             = SystemTheme::None;
};

/// @Visible: an invisible widget will not be drawn nor receive mouse/touch
/// events.
///
/// @Hittable: this needs to happen before mouse actions as some widgets .i.e.
/// some widgets don't need to intercept or receive mouse events return true if
/// accepts hit test at position.
enum class WidgetAttributes : u8
{
  None       = 0x00,
  Visible    = 0x01,
  Hittable   = 0x02,
  Scrollable = 0x04,
  Draggable  = 0x08
};

/// TODO(lamarrr): event context and names to global context? we can have
/// thousands of same widgets that might need processing as well. theming
/// reaction
//
// listener id for each event the widget emits?
//
// TODO(lamarrr): syncing different widget states?
// each widget should forward events to a global dict, the dict key is specified
// and changed by the user.
//
//

/// @brief Base widget class. All widget types must inherit from this struct.
/// all methods are already implemented with reasonable defaults.
/// Widgets are plain visual elements that define spatial relationships and
/// visual state changes, and forward events to other subsystems.
struct Widget
{
  Widget()          = default;
  virtual ~Widget() = default;

  /// @brief get child widgets, this is a virtual iterator, return null once
  /// there's no other children
  /// @return
  virtual Widget *get_child(u32 i)
  {
    (void) i;
    return nullptr;
  }

  /// @brief distributes the size allocated to it to its child widgets.
  /// unlike CSS. has the advantage that children wouldn't need extra attributes
  /// for specific kind of placements i.e. relative, absolute, etc.
  /// @param allocated_size the size allocated to this widget
  /// @param[out] children_allocation sizes allocated to the children.
  virtual void allocate_size(Vec2       allocated_size,
                             Span<Vec2> children_allocation)
  {
    (void) allocated_size;
    fill(children_allocation, Vec2{0, 0});
  }

  /// @brief fits itself around its children and positions child widgets
  /// along/relative to itself (i.e. position {0, 0} means the child will be
  /// placed on the center of the parent)
  /// @param allocated_size the size allocated to this widget. the widget can
  /// decide to disregard or fit to this as needed.
  /// @param children_sizes sizes of the child widgets
  /// @param[out] children_positions positions of the children widget on the
  /// parent
  /// @return this widget's fitted extent
  virtual Vec2 fit(Vec2 allocated_size, Span<Vec2 const> children_sizes,
                   Span<Vec2> children_positions)
  {
    (void) allocated_size;
    (void) children_sizes;
    (void) children_positions;
    return Vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the widget
  /// @param allocated_position the allocated absolute position of this widget
  virtual Vec2 position(Vec2 allocated_position)
  {
    return allocated_position;
  }

  virtual WidgetAttributes get_attributes(Vec2 position)
  {
    return WidgetAttributes::Visible;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param allocated_z_index
  /// @param[out] children_allocation z-index assigned to children
  /// @return
  virtual i32 z_stack(i32 allocated_z_index, Span<i32> children_allocation)
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
  virtual Rect clip(Rect parent_clip)
  {
    return parent_clip;
  }

  /// @brief record draw commands needed to render this widget. this method is
  /// only called if the widget passes the visibility tests. this is called on
  /// every frame.
  /// @param canvas
  virtual void render(Renderer &renderer)
  {
    (void) renderer;
  }

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a Subsystem that would
  /// handle that. i.e. using the multi-tasking system.
  /// @param interval time passed since last call to this method
  //
  // attached by widget system to global context
  // virtual Span<u8 const> get_drag_payload(WidgetSystem &);
  virtual void tick(u64 diff, WidgetEventTypes events)
  {
    (void) diff;
    (void) events;
  }

  uid id = UID_MAX;
};

}        // namespace ash
