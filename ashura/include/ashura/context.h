#pragma once

#include <map>
#include <string>
#include <string_view>

#include "SDL3/SDL.h"
#include "ashura/event.h"
#include "ashura/loggers.h"
#include "ashura/plugin.h"
#include "ashura/primitives.h"
#include "ashura/window.h"
#include "ashura/window_manager.h"
#include "fmt/format.h"
#include "stx/option.h"
#include "stx/scheduler.h"

namespace ash
{

struct WindowManager;
struct ClipBoard;

enum class SystemTheme
{
  Unknown = SDL_SYSTEM_THEME_UNKNOWN,
  Light   = SDL_SYSTEM_THEME_LIGHT,
  Dark    = SDL_SYSTEM_THEME_DARK
};

// add window controller class
// not thread-safe! ensure all api calls occur on the main thread.
struct Context
{
  stx::Vec<Plugin *>  plugins{stx::os_allocator};
  stx::TaskScheduler *task_scheduler = nullptr;
  ClipBoard          *clipboard      = nullptr;
  WindowManager      *window_manager = nullptr;
  SystemTheme         theme          = SystemTheme::Unknown;
  // TODO(lamarrr): expose current window here

  Context()
  {}

  STX_MAKE_PINNED(Context)

  ~Context()
  {
    for (Plugin *plugin : plugins)
    {
      ASH_LOG_INFO(Context, "Destroying plugin: {} (type: {})", plugin->get_name(), typeid(*plugin).name());
      delete plugin;
    }
  }

  void register_plugin(Plugin *plugin)
  {
    plugins.push_inplace(plugin).unwrap();
    ASH_LOG_INFO(Context, "Registered plugin: {} (type: {})", plugin->get_name(), typeid(*plugin).name());
  }

  template <typename T>
  stx::Option<T *> get_plugin(std::string_view name) const
  {
    stx::Span plugin = plugins.span().which([name](Plugin *plugin) { return plugin->get_name() == name; });
    if (!plugin.is_empty())
    {
      return stx::Some(plugin[0]->template as<T>());
    }
    else
    {
      return stx::None;
    }
  }

  // TODO(lamarrr): on_EXIT

  void tick(std::chrono::nanoseconds interval)
  {
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

  static Window *get_window(u32 id)
  {
    // TODO(lamarrr): allow other window types without handles
    SDL_Window *win = SDL_GetWindowFromID(id);
    if (win == nullptr)
    {
      return nullptr;
    }
    Window *bwin = AS(Window *, SDL_GetWindowData(win, "handle"));
    return bwin;
  }

  // polls for events, returns true if an event occured, otherwise false
  bool poll_events()
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
          Window      *win       = get_window(event.window.windowID);
          if (win == nullptr)
          {
            return true;
          }
          for (auto const &listener : win->event_listeners.general)
          {
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

          Window *win = get_window(event.button.windowID);
          if (win == nullptr)
          {
            return true;
          }
          for (auto &listener : win->event_listeners.mouse_click)
          {
            listener.handle(mouse_event);
          }
          return true;
        }

        case SDL_EVENT_MOUSE_MOTION:
        {
          Window *win = get_window(event.motion.windowID);
          if (win == nullptr)
          {
            return true;
          }
          for (auto &listener : win->event_listeners.mouse_motion)
          {
            listener.handle(MouseMotionEvent{.mouse_id    = MouseID{event.motion.which},
                                             .position    = vec2{AS(f32, event.motion.x), AS(f32, event.motion.y)},
                                             .translation = vec2{AS(f32, event.motion.xrel), AS(f32, event.motion.yrel)}});
          }
          return true;
        }

        case SDL_EVENT_MOUSE_WHEEL:
        {
          Window *win = get_window(event.wheel.windowID);
          if (win == nullptr)
          {
            return true;
          }
          for (auto &listener : win->event_listeners.mouse_wheel)
          {
            listener.handle(MouseWheelEvent{.mouse_id    = MouseID{event.wheel.which},
                                            .position    = vec2{AS(f32, event.wheel.mouseX), AS(f32, event.wheel.mouseY)},
                                            .translation = vec2{event.wheel.x, event.wheel.y}});
          }
          return true;
        }

        case SDL_EVENT_KEY_DOWN:
        {
          Window *win = get_window(event.key.windowID);
          if (win == nullptr)
          {
            return true;
          }
          for (auto &listener : win->event_listeners.key_down)
          {
            listener.handle(event.key.keysym.sym, AS(KeyModifiers, event.key.keysym.mod));
          }
          return true;
        }

        case SDL_EVENT_KEY_UP:
        {
          Window *win = get_window(event.key.windowID);
          if (win == nullptr)
          {
            return true;
          }
          for (auto &listener : win->event_listeners.key_up)
          {
            listener.handle(event.key.keysym.sym, AS(KeyModifiers, event.key.keysym.mod));
          }
          return true;
        }

        case SDL_EVENT_SYSTEM_THEME_CHANGED:
        {
          theme = AS(SystemTheme, SDL_GetSystemTheme());
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
