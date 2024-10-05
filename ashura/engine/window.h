/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/input.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/image.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct Window_T *Window;

struct WindowSystem
{
  virtual void           init()                                           = 0;
  virtual void           uninit()                                         = 0;
  virtual Option<Window> create_window(gfx::InstanceImpl instance,
                                       Span<char const>  title)            = 0;
  virtual void           destroy_window(Window window)                    = 0;
  virtual void           set_title(Window window, Span<char const> title) = 0;
  virtual char const    *get_title(Window window)                         = 0;
  virtual void           maximize(Window window)                          = 0;
  virtual void           minimize(Window window)                          = 0;
  virtual void           set_size(Window window, Vec2U size)              = 0;
  virtual void           center(Window window)                            = 0;
  virtual Vec2U          get_size(Window window)                          = 0;
  virtual Vec2U          get_surface_size(Window window)                  = 0;
  virtual void           set_position(Window window, Vec2I pos)           = 0;
  virtual Vec2I          get_position(Window window)                      = 0;
  virtual void           set_min_size(Window window, Vec2U min)           = 0;
  virtual Vec2U          get_min_size(Window window)                      = 0;
  virtual void           set_max_size(Window window, Vec2U max)           = 0;
  virtual Vec2U          get_max_size(Window window)                      = 0;
  virtual void           set_icon(Window window, ImageSpan<u8 const, 4> image,
                                  gfx::Format format)                     = 0;
  virtual void           make_bordered(Window window)                     = 0;
  virtual void           make_borderless(Window window)                   = 0;
  virtual void           show(Window window)                              = 0;
  virtual void           hide(Window window)                              = 0;
  virtual void           raise(Window window)                             = 0;
  virtual void           restore(Window window)                           = 0;
  virtual void           request_attention(Window window, bool briefly)   = 0;
  virtual void           make_fullscreen(Window window)                   = 0;
  virtual void           make_windowed(Window window)                     = 0;
  virtual void           make_resizable(Window window)                    = 0;
  virtual void           make_unresizable(Window window)                  = 0;
  virtual u64            listen(Window window, WindowEventTypes event_types,
                                Fn<void(WindowEvent const &)> callback)   = 0;
  virtual void           unlisten(Window window, u64 listener)            = 0;
  virtual Result<Void, Void> set_hit_test(Window                  window,
                                          Fn<WindowRegion(Vec2U)> hit)    = 0;
  virtual gfx::Surface       get_surface(Window window)                   = 0;
  virtual void               poll_events()                                = 0;
};

extern WindowSystem *sdl_window_system;

}        // namespace ash