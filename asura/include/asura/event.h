#pragma once

#include "asura/primitives.h"

namespace asr {

enum class WindowEvent : u8 {
  None,
  Shown,
  Hidden,
  Exposed,
  Moved,
  // window size changed by user
  Resized,
  // window size changed by user or via window API
  SizeChanged,
  Minimized,
  Maximized,
  Restored,
  Enter,
  Leave,
  FocusGained,
  FocusLost,
  Close,
  TakeFocus
};

enum class MouseButton : u8 {
  Primary,
  Secondary,
  Middle,
  A1,
  A2,
  A3,
  A4,
  A5
};

enum class KeyModifier : u8 {};

enum class MouseID : u32 {};

enum class MouseAction : u8 {
  Press,
  Release,
};

struct MouseMotionEvent {
  MouseID mouse_id{};
  OffsetI offset;
  OffsetI translation;
};

struct MouseClickEvent {
  MouseID mouse_id{};
  OffsetI offset;
  u32 clicks = 0;
  MouseButton button = MouseButton::Primary;
  MouseAction action = MouseAction::Press;
  KeyModifier modifier;  // TODO(lamarrr)
};

struct Keyboard;

struct ClipBoardEvent;  // on_update only

struct DeviceOrientationEvent;

// TODO(lamarrr):
struct Controller;

struct PointerLock;

}  // namespace asr
