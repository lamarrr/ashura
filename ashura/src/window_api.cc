
#include "ashura/window_api.h"

#include <map>

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/event.h"
#include "ashura/primitives.h"
#include "ashura/sdl_utils.h"
#include "ashura/utils.h"
#include "ashura/window.h"

namespace ash
{

namespace impl
{
constexpr WindowEvents sdl_window_event_to_ash(u32 type)
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
}        // namespace impl

WindowApi::WindowApi()
{
  ASH_SDL_CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0,
                "Unable to initialize SDL");
}

WindowApi::~WindowApi()
{
  SDL_Quit();
}

bool WindowApi::poll_events()
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
        WindowEvents win_event =
            impl::sdl_window_event_to_ash(event.window.type);

        for (auto const &listener :
             get_window_info(WindowID{event.window.windowID})
                 ->event_listeners)
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
        MouseClickEvent mouse_event{
            .mouse_id = MouseID{event.button.which},
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

        for (auto &listener : get_window_info(WindowID{event.button.windowID})
                                  ->mouse_click_listeners)
        {
          listener.handle(mouse_event);
        }
        return true;
      }

      case SDL_EVENT_MOUSE_MOTION:
      {
        for (auto &listener : get_window_info(WindowID{event.motion.windowID})
                                  ->mouse_motion_listeners)
        {
          listener.handle(MouseMotionEvent{
              .mouse_id = MouseID{event.motion.which},
              .position =
                  vec2{AS(f32, event.motion.x), AS(f32, event.motion.y)},
              .translation = vec2{AS(f32, event.motion.xrel),
                                  AS(f32, event.motion.yrel)}});
        }
        return true;
      }

      case SDL_EVENT_MOUSE_WHEEL:
      {
        for (auto &listener : get_window_info(WindowID{event.wheel.windowID})
                                  ->mouse_wheel_listeners)
        {
          listener.handle(MouseWheelEvent{
              .mouse_id    = MouseID{event.wheel.which},
              .position    = vec2{AS(f32, event.wheel.mouseX),
                               AS(f32, event.wheel.mouseY)},
              .translation = vec2{event.wheel.x, event.wheel.y}});
        }
        return true;
      }

      case SDL_EVENT_KEY_DOWN:
      {
        for (auto &listener : get_window_info(WindowID{event.key.windowID})
                                  ->key_down_listeners)
        {
          listener.handle(event.key.keysym.sym,
                          AS(KeyModifiers, event.key.keysym.mod));
        }
        return true;
      }

      case SDL_EVENT_KEY_UP:
      {
        for (auto &listener :
             get_window_info(WindowID{event.key.windowID})->key_up_listeners)
        {
          listener.handle(event.key.keysym.sym,
                          AS(KeyModifiers, event.key.keysym.mod));
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

stx::Vec<char const *> WindowApi::get_required_instance_extensions() const
{
  u32 ext_count;

  ASH_SDL_CHECK(
      SDL_Vulkan_GetInstanceExtensions(&ext_count, nullptr) == SDL_TRUE,
      "unable to get number of window's required Vulkan instance extensions");

  stx::Vec<char const *> required_instance_extensions{stx::os_allocator};

  required_instance_extensions.resize(ext_count).unwrap();

  ASH_SDL_CHECK(
      SDL_Vulkan_GetInstanceExtensions(
          &ext_count, required_instance_extensions.data()) == SDL_TRUE,
      "unable to get window's required Vulkan instance extensions");

  return required_instance_extensions;
}

}        // namespace ash
