
#include "ashura/window_api.h"

#include <map>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "ashura/event.h"
#include "ashura/primitives.h"
#include "ashura/sdl_utils.h"
#include "ashura/utils.h"
#include "ashura/window.h"

namespace ash {

namespace impl {
constexpr WindowEvents sdl_window_event_to_ash(u8 type) {
  switch (type) {
    case SDL_WINDOWEVENT_NONE:
      return WindowEvents::None;
    case SDL_WINDOWEVENT_SHOWN:
      return WindowEvents::Shown;
    case SDL_WINDOWEVENT_HIDDEN:
      return WindowEvents::Hidden;
    case SDL_WINDOWEVENT_EXPOSED:
      return WindowEvents::Exposed;
    case SDL_WINDOWEVENT_MOVED:
      return WindowEvents::Moved;
    case SDL_WINDOWEVENT_RESIZED:
      return WindowEvents::Resized;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      return WindowEvents::SizeChanged;
    case SDL_WINDOWEVENT_MINIMIZED:
      return WindowEvents::Minimized;
    case SDL_WINDOWEVENT_MAXIMIZED:
      return WindowEvents::Maximized;
    case SDL_WINDOWEVENT_RESTORED:
      return WindowEvents::Restored;
    case SDL_WINDOWEVENT_ENTER:
      return WindowEvents::Enter;
    case SDL_WINDOWEVENT_LEAVE:
      return WindowEvents::Leave;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
      return WindowEvents::FocusGained;
    case SDL_WINDOWEVENT_FOCUS_LOST:
      return WindowEvents::FocusLost;
    case SDL_WINDOWEVENT_CLOSE:
      return WindowEvents::Close;
    case SDL_WINDOWEVENT_TAKE_FOCUS:
      return WindowEvents::TakeFocus;
    default:
      return WindowEvents::None;
  }
}
}  // namespace impl

WindowApi::WindowApi() {
  ASH_SDL_CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0,
                "Unable to initialize SDL");
}

WindowApi::~WindowApi() { SDL_Quit(); }

bool WindowApi::poll_events() {
  SDL_Event event;

  if (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_WINDOWEVENT: {
        WindowEvents win_event =
            impl::sdl_window_event_to_ash(event.window.event);

        for (auto const& listener :
             get_window_info(WindowID{event.window.windowID})
                 ->event_listeners) {
          if ((listener.first & win_event) != WindowEvents::None) {
            listener.second.handle(win_event);
          }
        }
        return true;
      }

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
        MouseClickEvent mouse_event{
            .mouse_id = MouseID{event.button.which},
            .position = vec2{AS(f32, event.button.x), AS(f32, event.button.y)},
            .clicks = event.button.clicks};

        switch (event.button.button) {
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

        switch (event.type) {
          case SDL_MOUSEBUTTONDOWN:
            mouse_event.action = MouseAction::Press;
            break;

          case SDL_MOUSEBUTTONUP:
            mouse_event.action = MouseAction::Release;
            break;

          default:
            return true;
        }

        for (auto& listener : get_window_info(WindowID{event.button.windowID})
                                  ->mouse_click_listeners) {
          listener.handle(mouse_event);
        }
        return true;
      }

      case SDL_MOUSEMOTION: {
        for (auto& listener : get_window_info(WindowID{event.motion.windowID})
                                  ->mouse_motion_listeners) {
          listener.handle(MouseMotionEvent{
              .mouse_id = MouseID{event.motion.which},
              .position =
                  vec2{AS(f32, event.motion.x), AS(f32, event.motion.y)},
              .translation = vec2{AS(f32, event.motion.xrel),
                                  AS(f32, event.motion.yrel)}});
        }
        return true;
      }

      case SDL_MOUSEWHEEL: {
        for (auto& listener : get_window_info(WindowID{event.wheel.windowID})
                                  ->mouse_wheel_listeners) {
          listener.handle(MouseWheelEvent{
              .mouse_id = MouseID{event.wheel.which},
              .position = vec2{AS(f32, event.wheel.mouseX),
                               AS(f32, event.wheel.mouseY)},
              .translation = vec2{event.wheel.preciseX, event.wheel.preciseY}});
        }
        return true;
      }

      case SDL_KEYDOWN: {
        for (auto& listener : get_window_info(WindowID{event.key.windowID})
                                  ->key_down_listeners) {
          listener.handle(event.key.keysym.sym,
                          AS(KeyModifiers, event.key.keysym.mod));
        }
        return true;
      }

      case SDL_KEYUP: {
        for (auto& listener :
             get_window_info(WindowID{event.key.windowID})->key_up_listeners) {
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

        // TODO(lamarrr): Consider:
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

      default: {
        return true;
      }
    }
  } else {
    return false;
  }
}

}  // namespace ash
