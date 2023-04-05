
#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/backend_window.h"
#include "ashura/primitives.h"
#include "ashura/window.h"
#include <map>

namespace ash
{

struct WindowManager
{
  static stx::Rc<BackendWindow *> create(char const *title, WindowType type, WindowCreateFlags flags, ash::extent extent)
  {
    // width and height here refer to the screen coordinates and not the
    // actual pixel coordinates (SEE: Device Pixel Ratio)

    int window_flags = SDL_WINDOW_VULKAN;

    if (type == WindowType::Normal)
    {
      //
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

    stx::Rc w = stx::rc::make_inplace<BackendWindow>(stx::os_allocator, window).unwrap();

    SDL_SetWindowData(w->window, "impl", w.handle);

    return std::move(w);
  }

  // virtual void on_full_screen_change();

  static constexpr WindowEvents to_window_event(SDL_EventType type)
  {
    switch (type)
    {
      case SDL_EVENT_FIRST:
        return WindowEvents::None;
      case SDL_EVENT_WINDOW_SHOWN:
        return WindowEvents::Shown;
      case SDL_EVENT_WINDOW_HIDDEN:
        return WindowEvents::Hidden;
      case SDL_EVENT_WINDOW_EXPOSED:
        return WindowEvents::Exposed;
      case SDL_EVENT_WINDOW_MOVED:
        return WindowEvents::Moved;
      case SDL_EVENT_WINDOW_RESIZED:
        return WindowEvents::Resized;
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        return WindowEvents::PixelSizeChanged;
      case SDL_EVENT_WINDOW_MINIMIZED:
        return WindowEvents::Minimized;
      case SDL_EVENT_WINDOW_MAXIMIZED:
        return WindowEvents::Maximized;
      case SDL_EVENT_WINDOW_RESTORED:
        return WindowEvents::Restored;
      case SDL_EVENT_WINDOW_MOUSE_ENTER:
        return WindowEvents::MouseEnter;
      case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        return WindowEvents::MouseLeave;
      case SDL_EVENT_WINDOW_FOCUS_GAINED:
        return WindowEvents::FocusGained;
      case SDL_EVENT_WINDOW_FOCUS_LOST:
        return WindowEvents::FocusLost;
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return WindowEvents::CloseRequested;
      case SDL_EVENT_WINDOW_TAKE_FOCUS:
        return WindowEvents::TakeFocus;
      default:
        return WindowEvents::None;
    }
  }

  static BackendWindow *get_window(u32 id)
  {
    SDL_Window *win = SDL_GetWindowFromID(id);
    ASH_SDL_CHECK(win != nullptr);
    BackendWindow *bwin = AS(BackendWindow *, SDL_GetWindowData(win, "impl"));
    ASH_SDL_CHECK(bwin != nullptr);
    return bwin;
  }

  // polls for events, returns true if an event occured, otherwise false
  static bool poll_events()
  {
    SDL_Event event;

    if (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
        {
          WindowEvents win_event = to_window_event((SDL_EventType) event.window.type);

          for (auto const &listener : get_window(event.window.windowID)->event_listeners.general)
          {
            spdlog::info("win event: {}", (int) event.type);
            if ((listener.first & win_event) != WindowEvents::None)
            {
              listener.second.handle(win_event);
            }
          }
          return true;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
          MouseClickEvent mouse_event{.mouse_id = MouseID{event.button.which},
                                      .position = vec2{AS(f32, event.button.x), AS(f32, event.button.y)},
                                      .clicks   = event.button.clicks};

          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT:
              mouse_event.button = MouseButton::Primary;
              break;
            case SDL_BUTTON_RIGHT:
              mouse_event.button = MouseButton::Secondary;
              break;
            case SDL_BUTTON_MIDDLE:
              mouse_event.button = MouseButton::Middle;
              break;
            case SDL_BUTTON_X1:
              mouse_event.button = MouseButton::A1;
              break;
            case SDL_BUTTON_X2:
              mouse_event.button = MouseButton::A2;
              break;
            default:
              return true;
          }

          switch (event.type)
          {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
              mouse_event.action = MouseAction::Press;
              break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
              mouse_event.action = MouseAction::Release;
              break;

            default:
              return true;
          }

          for (auto &listener : get_window(event.button.windowID)->event_listeners.mouse_click)
          {
            listener.handle(mouse_event);
          }
          return true;
        }

        case SDL_EVENT_MOUSE_MOTION:
        {
          for (auto &listener : get_window(event.motion.windowID)->event_listeners.mouse_motion)
          {
            listener.handle(MouseMotionEvent{.mouse_id    = MouseID{event.motion.which},
                                             .position    = vec2{AS(f32, event.motion.x), AS(f32, event.motion.y)},
                                             .translation = vec2{AS(f32, event.motion.xrel), AS(f32, event.motion.yrel)}});
          }
          return true;
        }

        case SDL_EVENT_MOUSE_WHEEL:
        {
          for (auto &listener : get_window(event.wheel.windowID)->event_listeners.mouse_wheel)
          {
            listener.handle(MouseWheelEvent{.mouse_id    = MouseID{event.wheel.which},
                                            .position    = vec2{AS(f32, event.wheel.mouseX), AS(f32, event.wheel.mouseY)},
                                            .translation = vec2{event.wheel.x, event.wheel.y}});
          }
          return true;
        }

        case SDL_EVENT_KEY_DOWN:
        {
          for (auto &listener : get_window(event.key.windowID)->event_listeners.key_down)
          {
            listener.handle(event.key.keysym.sym, AS(KeyModifiers, event.key.keysym.mod));
          }
          return true;
        }

        case SDL_EVENT_KEY_UP:
        {
          for (auto &listener : get_window(event.key.windowID)->event_listeners.key_up)
          {
            listener.handle(event.key.keysym.sym, AS(KeyModifiers, event.key.keysym.mod));
          }
          return true;
        }

          /* Touch events */
          /* Gesture events */

          // TODO(lamarrr): forward other events
          // case SDL_CLIPBOARDUPDATE

          // case SDL_DROPFILE
          // case SDL_DROPFILE
          // case SDL_DROPBEGIN
          // case SDL_DROPCOMPLETE

          // SDL_TEXTEDITING,
          // SDL_TEXTINPUT,
          // SDL_KEYMAPCHANGED,

          // SDL_CONTROLLERAXISMOTION
          // SDL_CONTROLLERBUTTONDOWN
          // SDL_CONTROLLERBUTTONUP
          // SDL_CONTROLLERDEVICEADDED
          // SDL_CONTROLLERDEVICEREMOVED
          // SDL_CONTROLLERDEVICEREMAPPED
          // SDL_CONTROLLERTOUCHPADDOWN
          // SDL_CONTROLLERTOUCHPADMOTION
          // SDL_CONTROLLERTOUCHPADUP
          // SDL_CONTROLLERSENSORUPDATE

          SDL_EVENT_AUDIO_DEVICE_ADDED;
          SDL_EVENT_AUDIO_DEVICE_REMOVED;
          SDL_EVENT_SYSTEM_THEME_CHANGED;
          SDL_EVENT_DISPLAY_ORIENTATION;          // Display orientation has changed to data1
          SDL_EVENT_DISPLAY_CONNECTED;            // Display has been added to the system
          SDL_EVENT_DISPLAY_DISCONNECTED;         // Display has been removed from the system
          SDL_EVENT_DISPLAY_MOVED;                // Display has changed position
          SDL_EVENT_DISPLAY_SCALE_CHANGED;        // Display has changed desktop display scale

        default:
        {
          return true;
        }
      }
    }
    else
    {
      return false;
    }
  }
};
}        // namespace ash
