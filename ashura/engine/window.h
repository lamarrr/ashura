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

enum class Window : uid
{
  None = UID_INVALID
};

struct WindowSystem
{
  virtual Option<Window>  create_window(gfx::InstanceImpl instance,
                                     char const       *title)               = 0;
  virtual void         destroy_window(Window w)                       = 0;
  virtual void         set_title(Window w, char const *title)         = 0;
  virtual char const  *get_title(Window w)                            = 0;
  virtual void         maximize(Window w)                             = 0;
  virtual void         minimize(Window w)                             = 0;
  virtual void         set_size(Window w, Vec2U size)                 = 0;
  virtual void         center(Window w)                               = 0;
  virtual Vec2U        get_size(Window w)                             = 0;
  virtual Vec2U        get_surface_size(Window w)                     = 0;
  virtual void         set_position(Window w, Vec2I pos)              = 0;
  virtual Vec2I        get_position(Window w)                         = 0;
  virtual void         set_min_size(Window w, Vec2U min)              = 0;
  virtual Vec2U        get_min_size(Window w)                         = 0;
  virtual void         set_max_size(Window w, Vec2U max)              = 0;
  virtual Vec2U        get_max_size(Window w)                         = 0;
  virtual void         set_icon(Window w, ImageSpan<u8 const> image)  = 0;
  virtual void         make_bordered(Window w)                        = 0;
  virtual void         make_borderless(Window w)                      = 0;
  virtual void         show(Window w)                                 = 0;
  virtual void         hide(Window w)                                 = 0;
  virtual void         raise(Window w)                                = 0;
  virtual void         restore(Window w)                              = 0;
  virtual void         request_attention(Window w, bool briefly)      = 0;
  virtual void         make_fullscreen(Window w)                      = 0;
  virtual void         make_windowed(Window w)                        = 0;
  virtual void         make_resizable(Window w)                       = 0;
  virtual void         make_unresizable(Window w)                     = 0;
  virtual uid          listen(Window w, WindowEventTypes event_types,
                              Fn<void(WindowEvent const &)> callback) = 0;
  virtual void         unlisten(Window w, uid listener)               = 0;
  virtual gfx::Surface get_surface(Window w)                          = 0;
  virtual void         poll_events()                                  = 0;
};

WindowSystem *init_sdl_window_system();

}        // namespace ash