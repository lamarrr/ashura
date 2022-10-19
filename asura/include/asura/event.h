#pragma once

#include "asura/primitives.h"

namespace asr {

enum class WindowEvent : uint8_t {
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

enum class MouseButton : uint8_t {
  Primary,
  Secondary,
  Middle,
  A1,
  A2,
  A3,
  A4,
  A5
};

enum class KeyModifier : uint8_t {};

enum class MouseID : uint32_t {};

enum class MouseAction : uint8_t {
  Press,
  Release,
};

struct MouseMotionEvent {
  MouseID mouse_id{};
  IOffset offset;
  IOffset translation;
};

struct MouseClickEvent {
  MouseID mouse_id{};
  IOffset offset;
  uint32_t clicks = 0;
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
