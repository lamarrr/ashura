#pragma once

#include "ashura/primitives.h"

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

enum class MouseButton : u8 { Primary, Secondary, Middle, A1, A2, A3, A4, A5 };

enum class KeyModifiers : u8 {
  None = 0,
  Alt = 1,
  Ctrl = 2,
  Shift = 4,
  Meta = 8  // "⌘" present on mac systems
};

STX_DEFINE_ENUM_BIT_OPS(KeyModifiers)

enum class MouseID : u32 {};

enum class MouseAction : u8 {
  Press,
  Release,
};

struct MouseMotionEvent {
  MouseID mouse_id{};
  offseti offset;
  offseti translation;
};

struct MouseClickEvent {
  MouseID mouse_id{};
  offseti offset;
  u32 clicks = 0;
  MouseButton button = MouseButton::Primary;
  MouseAction action = MouseAction::Press;
  KeyModifiers modifiers = KeyModifiers::None;  // TODO(lamarrr)
};

struct Keyboard;

struct ClipBoardEvent;  // on_update only

struct DeviceOrientationEvent;

struct Controller;

struct PointerLock;

}  // namespace asr
