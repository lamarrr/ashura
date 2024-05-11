#pragma once

#include "ashura/engine/key.h"
#include "ashura/gfx/gfx.h"
#include "ashura/gfx/image.h"
#include "ashura/std/option.h"
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
  uid  mouse_id = UID_INVALID;
  Vec2 position;
  Vec2 translation;
};

struct MouseClickEvent
{
  uid          mouse_id = UID_INVALID;
  Vec2         position;
  u32          clicks = 0;
  MouseButtons button = MouseButtons::None;
  KeyAction    action = KeyAction::Press;
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

struct WindowSystem
{
  virtual Option<uid>  create_window(gfx::InstanceImpl instance,
                                     char const       *title)                = 0;
  virtual void         destroy_window(uid window)                      = 0;
  virtual void         set_title(uid window, char const *title)        = 0;
  virtual char const  *get_title(uid window)                           = 0;
  virtual void         maximize(uid window)                            = 0;
  virtual void         minimize(uid window)                            = 0;
  virtual void         set_size(uid window, Vec2U size)                = 0;
  virtual void         center(uid window)                              = 0;
  virtual Vec2U        get_size(uid window)                            = 0;
  virtual Vec2U        get_surface_size(uid window)                    = 0;
  virtual void         set_position(uid window, Vec2I pos)             = 0;
  virtual Vec2I        get_position(uid window)                        = 0;
  virtual void         set_min_size(uid window, Vec2U min)             = 0;
  virtual Vec2U        get_min_size(uid window)                        = 0;
  virtual void         set_max_size(uid window, Vec2U max)             = 0;
  virtual Vec2U        get_max_size(uid window)                        = 0;
  virtual void         set_icon(uid window, ImageSpan<u8 const> image) = 0;
  virtual void         make_bordered(uid window)                       = 0;
  virtual void         make_borderless(uid window)                     = 0;
  virtual void         show(uid window)                                = 0;
  virtual void         hide(uid window)                                = 0;
  virtual void         raise(uid window)                               = 0;
  virtual void         restore(uid window)                             = 0;
  virtual void         request_attention(uid window, bool briefly)     = 0;
  virtual void         make_fullscreen(uid window)                     = 0;
  virtual void         make_windowed(uid window)                       = 0;
  virtual void         make_resizable(uid window)                      = 0;
  virtual void         make_unresizable(uid window)                    = 0;
  virtual uid          listen(uid window, WindowEventTypes event_types,
                              Fn<void(WindowEvent const &)> callback)  = 0;
  virtual void         unlisten(uid window, uid listener)              = 0;
  virtual gfx::Surface get_surface(uid window)                         = 0;
  virtual void         poll_events()                                   = 0;
};

WindowSystem *init_sdl_window_system();

}        // namespace ash