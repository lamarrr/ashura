#pragma once

#include "ashura/engine/event.h"
#include "ashura/engine/key.h"
#include "ashura/gfx/gfx.h"
#include "ashura/gfx/image.h"
#include "ashura/std/option.h"
#include "ashura/std/types.h"

namespace ash
{

enum class Window : uid
{
  None = UID_INVALID
};

struct WindowSystem
{
  virtual Option<Window> create_window(gfx::InstanceImpl instance,
                                       char const       *title)               = 0;
  virtual void           destroy_window(Window w)                       = 0;
  virtual void           set_title(Window w, char const *title)         = 0;
  virtual char const    *get_title(Window w)                            = 0;
  virtual void           maximize(Window w)                             = 0;
  virtual void           minimize(Window w)                             = 0;
  virtual void           set_size(Window w, Vec2U size)                 = 0;
  virtual void           center(Window w)                               = 0;
  virtual Vec2U          get_size(Window w)                             = 0;
  virtual Vec2U          get_surface_size(Window w)                     = 0;
  virtual void           set_position(Window w, Vec2I pos)              = 0;
  virtual Vec2I          get_position(Window w)                         = 0;
  virtual void           set_min_size(Window w, Vec2U min)              = 0;
  virtual Vec2U          get_min_size(Window w)                         = 0;
  virtual void           set_max_size(Window w, Vec2U max)              = 0;
  virtual Vec2U          get_max_size(Window w)                         = 0;
  virtual void           set_icon(Window w, ImageSpan<u8 const> image)  = 0;
  virtual void           make_bordered(Window w)                        = 0;
  virtual void           make_borderless(Window w)                      = 0;
  virtual void           show(Window w)                                 = 0;
  virtual void           hide(Window w)                                 = 0;
  virtual void           raise(Window w)                                = 0;
  virtual void           restore(Window w)                              = 0;
  virtual void           request_attention(Window w, bool briefly)      = 0;
  virtual void           make_fullscreen(Window w)                      = 0;
  virtual void           make_windowed(Window w)                        = 0;
  virtual void           make_resizable(Window w)                       = 0;
  virtual void           make_unresizable(Window w)                     = 0;
  virtual uid            listen(Window w, WindowEventTypes event_types,
                                Fn<void(WindowEvent const &)> callback) = 0;
  virtual void           unlisten(Window w, uid listener)               = 0;
  virtual gfx::Surface   get_surface(Window w)                          = 0;
  virtual void           poll_events()                                  = 0;
};

WindowSystem *init_sdl_window_system();

}        // namespace ash