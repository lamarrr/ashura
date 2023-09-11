
#pragma once
#include "SDL3/SDL.h"
#include "ashura/primitives.h"
#include "ashura/window.h"
#include "stx/allocator.h"
#include "stx/rc.h"

namespace ash
{

struct WindowManager
{
  stx::Rc<Window *> create_window(char const *title, WindowType type, WindowCreateFlags flags, ash::Extent extent)
  {
    // width and height here refer to the screen coordinates and not the
    // actual pixel coordinates (SEE: Device Pixel Ratio)

    int window_flags = SDL_WINDOW_VULKAN;

    if (type == WindowType::Normal)
    {
    }
    else if (type == WindowType::Popup)
    {
      window_flags |= SDL_WINDOW_POPUP_MENU;
    }
    else if (type == WindowType::Tooltip)
    {
      window_flags |= SDL_WINDOW_TOOLTIP;
    }
    else if (type == WindowType::Utility)
    {
      window_flags |= SDL_WINDOW_UTILITY;
    }

    if ((flags & WindowCreateFlags::Hidden) != WindowCreateFlags::None)
    {
      window_flags |= SDL_WINDOW_HIDDEN;
    }

    window_flags |= SDL_WINDOW_RESIZABLE;

    if ((flags & WindowCreateFlags::NonResizable) != WindowCreateFlags::None)
    {
      window_flags &= ~SDL_WINDOW_RESIZABLE;
    }

    if ((flags & WindowCreateFlags::Borderless) != WindowCreateFlags::None)
    {
      window_flags |= SDL_WINDOW_BORDERLESS;
    }

    if ((flags & WindowCreateFlags::FullScreen) != WindowCreateFlags::None)
    {
      window_flags |= SDL_WINDOW_FULLSCREEN;
    }

    if ((flags & WindowCreateFlags::AlwaysOnTop) != WindowCreateFlags::None)
    {
      window_flags |= SDL_WINDOW_ALWAYS_ON_TOP;
    }

    SDL_Window *window = SDL_CreateWindow(title, AS(i32, extent.width), AS(i32, extent.height), window_flags);

    // window creation shouldn't fail reliably, if it fails, there's no point in the program proceeding
    ASH_SDL_CHECK(window != nullptr, "unable to create window");

    u32 window_id = SDL_GetWindowID(window);
    ASH_SDL_CHECK(window_id != 0);

    stx::Rc w = stx::rc::make_inplace<Window>(stx::os_allocator, window).unwrap();

    SDL_SetWindowData(window, "handle", w.handle);

    return std::move(w);
  }
};

}        // namespace ash
