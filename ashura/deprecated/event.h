#pragma once

#include "SDL3/SDL_keycode.h"
#include "ashura/primitives.h"
#include "stx/allocator.h"
#include "stx/enum.h"
#include "stx/fn.h"
#include "stx/vec.h"

namespace ash
{

using WindowID      = u32;
using MouseID       = u32;
using AudioDeviceID = u32;

enum class KeyAction : u8
{
  Press,
  Release,
};

struct MouseMotionEvent
{
  MouseID mouse_id = 0;
  Vec2    position;
  Vec2    translation;
};

struct MouseClickEvent
{
  MouseID     mouse_id = 0;
  Vec2        position;
  u32         clicks = 0;
  MouseButton button = MouseButton::None;
  KeyAction   action = KeyAction::Press;
};

struct MouseWheelEvent
{
  MouseID mouse_id = 0;
  Vec2    position;
  Vec2    translation;
};

struct ClipBoardEvent;        // TODO(lamarrr): on_update only
struct DeviceOrientationEvent;        // TODO(lamarrr)
struct PointerLock;        // TODO(lamarrr)
struct KeyEvent
{
  Key          key       = UNKNOWN_Key;
  KeyModifiers modifiers = KeyModifiers::None;
  KeyAction    action    = KeyAction::Press;
};
struct AudioDeviceEvent
{
  AudioDeviceID device_id  = 0;
  bool          is_capture = false;
};
struct WindowEventListeners
{
  stx::Vec<std::pair<WindowEvents, stx::UniqueFn<void(WindowEvents)>>> general;
  stx::Vec<stx::UniqueFn<void(MouseClickEvent)>>  mouse_click;
  stx::Vec<stx::UniqueFn<void(MouseMotionEvent)>> mouse_motion;
  stx::Vec<stx::UniqueFn<void(MouseWheelEvent)>>  mouse_wheel;
  stx::Vec<stx::UniqueFn<void(KeyEvent)>>         key;
};
struct GlobalEventListeners
{
  stx::Vec<stx::UniqueFn<void(AudioDeviceEvent)>> audio_event;
  stx::Vec<stx::UniqueFn<void()>>                 system_theme;
};

}        // namespace ash
