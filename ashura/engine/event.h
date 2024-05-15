#pragma once

#include "ashura/engine/key.h"
#include "ashura/std/types.h"

namespace ash
{

enum class SystemTheme : u8
{
  None  = 0,
  Light = 1,
  Dark  = 2
};

enum class KeyAction : u8
{
  None    = 0,
  Press   = 1,
  Release = 2
};

struct KeyEvent
{
  Key          key       = Key::KEY_UNKNOWN;
  KeyModifiers modifiers = KeyModifiers::None;
  KeyAction    action    = KeyAction::Press;
};

struct MouseMotionEvent
{
  uid  mouse_id    = UID_INVALID;
  Vec2 position    = {};
  Vec2 translation = {};
};

struct MouseClickEvent
{
  uid          mouse_id = UID_INVALID;
  Vec2         position = {};
  u32          clicks   = 0;
  MouseButtons button   = MouseButtons::None;
  KeyAction    action   = KeyAction::Press;
};

struct MouseWheelEvent
{
  uid  mouse_id = UID_INVALID;
  Vec2 position;
  Vec2 translation;
};

enum class WindowEventTypes : u32
{
  None           = 0x000000,
  Shown          = 0x000001,
  Hidden         = 0x000002,
  Exposed        = 0x000004,
  Moved          = 0x000008,
  Resized        = 0x000010,
  SurfaceResized = 0x000020,
  Minimized      = 0x000040,
  Maximized      = 0x000080,
  Restored       = 0x000100,
  MouseEnter     = 0x000200,
  MouseLeave     = 0x000400,
  FocusGained    = 0x000800,
  FocusLost      = 0x001000,
  CloseRequested = 0x002000,
  TakeFocus      = 0x004000,
  Key            = 0x008000,
  MouseMotion    = 0x010000,
  MouseClick     = 0x020000,
  MouseWheel     = 0x040000,
  Destroyed      = 0x080000,
  All            = 0xFFFFFF
};

ASH_DEFINE_ENUM_BIT_OPS(WindowEventTypes)

struct WindowEvent
{
  union
  {
    KeyEvent         key;
    MouseMotionEvent mouse_motion;
    MouseClickEvent  mouse_click;
    MouseWheelEvent  mouse_wheel;
    char             none_ = 0;
  };
  WindowEventTypes type = WindowEventTypes::None;
};

}        // namespace ash
