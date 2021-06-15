
#include <cstdint>

#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {
/*
struct GestureInput {
  enum class Spatial : uint8_t {
    Spatial_Enum_Min__ = 0,
    Clicked = Spatial_Enum_Min__,
    DoubleClicked,
    Hovered,
    DragBegin,
    DragContinue,
    DragEnd,
    Spatial_Enum_Max__ = DragEnd
  } spatial;

  Offset spatial_values[(Spatial::Spatial_Enum_Max__ -
                         Spatial::Spatial_Enum_Min__) +
                        1] = {};
};
*/


// tap
// drag
// flick
// swipe
// double tap
// pinch
// three finger pinch
// three finger swipe
// touch and hold
// rotate
// shake

/*


onDoubleTap → GestureTapCallback?
The user has tapped the screen with a primary button at the same location twice in quick succession. [...]
final
onDoubleTapCancel → GestureTapCancelCallback?
The pointer that previously triggered onDoubleTapDown will not end up causing a double tap. [...]
final
onDoubleTapDown → GestureTapDownCallback?
A pointer that might cause a double tap has contacted the screen at a particular location. [...]
final
onForcePressEnd → GestureForcePressEndCallback?
The pointer is no longer in contact with the screen. [...]
final
onForcePressPeak → GestureForcePressPeakCallback?
The pointer is in contact with the screen and has pressed with the maximum force. The amount of force is at least ForcePressGestureRecognizer.peakPressure. [...]
final
onForcePressStart → GestureForcePressStartCallback?
The pointer is in contact with the screen and has pressed with sufficient force to initiate a force press. The amount of force is at least ForcePressGestureRecognizer.startPressure. [...]
final
onForcePressUpdate → GestureForcePressUpdateCallback?
A pointer is in contact with the screen, has previously passed the ForcePressGestureRecognizer.startPressure and is either moving on the plane of the screen, pressing the screen with varying forces or both simultaneously. [...]
final
onHorizontalDragCancel → GestureDragCancelCallback?
The pointer that previously triggered onHorizontalDragDown did not complete. [...]
final
onHorizontalDragDown → GestureDragDownCallback?
A pointer has contacted the screen with a primary button and might begin to move horizontally. [...]
final
onHorizontalDragEnd → GestureDragEndCallback?
A pointer that was previously in contact with the screen with a primary button and moving horizontally is no longer in contact with the screen and was moving at a specific velocity when it stopped contacting the screen. [...]
final
onHorizontalDragStart → GestureDragStartCallback?
A pointer has contacted the screen with a primary button and has begun to move horizontally. [...]
final
onHorizontalDragUpdate → GestureDragUpdateCallback?
A pointer that is in contact with the screen with a primary button and moving horizontally has moved in the horizontal direction. [...]
final
onLongPress → GestureLongPressCallback?
Called when a long press gesture with a primary button has been recognized. [...]
final
onLongPressEnd → GestureLongPressEndCallback?
A pointer that has triggered a long-press with a primary button has stopped contacting the screen. [...]
final
onLongPressMoveUpdate → GestureLongPressMoveUpdateCallback?
A pointer has been drag-moved after a long press with a primary button. [...]
final
onLongPressStart → GestureLongPressStartCallback?
Called when a long press gesture with a primary button has been recognized. [...]
final
onLongPressUp → GestureLongPressUpCallback?
A pointer that has triggered a long-press with a primary button has stopped contacting the screen. [...]
final
onPanCancel → GestureDragCancelCallback?
The pointer that previously triggered onPanDown did not complete. [...]
final
onPanDown → GestureDragDownCallback?
A pointer has contacted the screen with a primary button and might begin to move. [...]
final
onPanEnd → GestureDragEndCallback?
A pointer that was previously in contact with the screen with a primary button and moving is no longer in contact with the screen and was moving at a specific velocity when it stopped contacting the screen. [...]
final
onPanStart → GestureDragStartCallback?
A pointer has contacted the screen with a primary button and has begun to move. [...]
final
onPanUpdate → GestureDragUpdateCallback?
A pointer that is in contact with the screen with a primary button and moving has moved again. [...]
final
onScaleEnd → GestureScaleEndCallback?
The pointers are no longer in contact with the screen.
final
onScaleStart → GestureScaleStartCallback?
The pointers in contact with the screen have established a focal point and initial scale of 1.0.
final
onScaleUpdate → GestureScaleUpdateCallback?
The pointers in contact with the screen have indicated a new focal point and/or scale.
final
onSecondaryLongPress → GestureLongPressCallback?
Called when a long press gesture with a secondary button has been recognized. [...]
final
onSecondaryLongPressEnd → GestureLongPressEndCallback?
A pointer that has triggered a long-press with a secondary button has stopped contacting the screen. [...]
final
onSecondaryLongPressMoveUpdate → GestureLongPressMoveUpdateCallback?
A pointer has been drag-moved after a long press with a secondary button. [...]
final
onSecondaryLongPressStart → GestureLongPressStartCallback?
Called when a long press gesture with a secondary button has been recognized. [...]
final
onSecondaryLongPressUp → GestureLongPressUpCallback?
A pointer that has triggered a long-press with a secondary button has stopped contacting the screen. [...]
final
onSecondaryTap → GestureTapCallback?
A tap with a secondary button has occurred. [...]
final
onSecondaryTapCancel → GestureTapCancelCallback?
The pointer that previously triggered onSecondaryTapDown will not end up causing a tap. [...]
final
onSecondaryTapDown → GestureTapDownCallback?
A pointer that might cause a tap with a secondary button has contacted the screen at a particular location. [...]
final
onSecondaryTapUp → GestureTapUpCallback?
A pointer that will trigger a tap with a secondary button has stopped contacting the screen at a particular location. [...]
final
onTap → GestureTapCallback?
A tap with a primary button has occurred. [...]
final
onTapCancel → GestureTapCancelCallback?
The pointer that previously triggered onTapDown will not end up causing a tap. [...]
final
onTapDown → GestureTapDownCallback?
A pointer that might cause a tap with a primary button has contacted the screen at a particular location. [...]
final
onTapUp → GestureTapUpCallback?
A pointer that will trigger a tap with a primary button has stopped contacting the screen at a particular location. [...]
final
onTertiaryTapCancel → GestureTapCancelCallback?
The pointer that previously triggered onTertiaryTapDown will not end up causing a tap. [...]
final
onTertiaryTapDown → GestureTapDownCallback?
A pointer that might cause a tap with a tertiary button has contacted the screen at a particular location. [...]
final
onTertiaryTapUp → GestureTapUpCallback?
A pointer that will trigger a tap with a tertiary button has stopped contacting the screen at a particular location. [...]
final
onVerticalDragCancel → GestureDragCancelCallback?
The pointer that previously triggered onVerticalDragDown did not complete. [...]
final
onVerticalDragDown → GestureDragDownCallback?
A pointer has contacted the screen with a primary button and might begin to move vertically. [...]
final
onVerticalDragEnd → GestureDragEndCallback?
A pointer that was previously in contact with the screen with a primary button and moving vertically is no longer in contact with the screen and was moving at a specific velocity when it stopped contacting the screen. [...]
final
onVerticalDragStart → GestureDragStartCallback?
A pointer has contacted the screen with a primary button and has begun to move vertically. [...]
final
onVerticalDragUpdate → GestureDragUpdateCallback?
A pointer that is in contact with the screen with a primary button and moving vertically has moved in the vertical direction. [...]
final

*/
}  // namespace ui
}  // namespace vlk
