
#include "asura/window_api.h"

#include <map>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "asura/event.h"
#include "asura/sdl_utils.h"
#include "asura/utils.h"
#include "asura/window.h"

namespace asr {

namespace impl {
inline WindowEvent sdl_window_event_to_asr(uint8_t win_event_type) {
  switch (win_event_type) {
    case SDL_WINDOWEVENT_NONE:
      return WindowEvent::None;
    case SDL_WINDOWEVENT_SHOWN:
      return WindowEvent::Shown;
    case SDL_WINDOWEVENT_HIDDEN:
      return WindowEvent::Hidden;
    case SDL_WINDOWEVENT_EXPOSED:
      return WindowEvent::Exposed;
    case SDL_WINDOWEVENT_MOVED:
      return WindowEvent::Moved;
    case SDL_WINDOWEVENT_RESIZED:
      return WindowEvent::Resized;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      return WindowEvent::SizeChanged;
    case SDL_WINDOWEVENT_MINIMIZED:
      return WindowEvent::Minimized;
    case SDL_WINDOWEVENT_MAXIMIZED:
      return WindowEvent::Maximized;
    case SDL_WINDOWEVENT_RESTORED:
      return WindowEvent::Restored;
    case SDL_WINDOWEVENT_ENTER:
      return WindowEvent::Enter;
    case SDL_WINDOWEVENT_LEAVE:
      return WindowEvent::Leave;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
      return WindowEvent::FocusGained;
    case SDL_WINDOWEVENT_FOCUS_LOST:
      return WindowEvent::FocusLost;
    case SDL_WINDOWEVENT_CLOSE:
      return WindowEvent::Close;
    case SDL_WINDOWEVENT_TAKE_FOCUS:
      return WindowEvent::TakeFocus;
    default:
      return WindowEvent::None;
  }
}
}  // namespace impl

WindowApi::WindowApi() {
  ASR_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0,
                 "Unable to initialize SDL");
}

WindowApi::~WindowApi() { SDL_Quit(); }

bool WindowApi::poll_events() {
  SDL_Event event{};

  if (SDL_PollEvent(&event) == 1) {
    switch (event.type) {
      case SDL_WINDOWEVENT: {
        Window* win = get_window_info(WindowID{event.window.windowID});

        auto listener = win->window_event_listeners.find(
            impl::sdl_window_event_to_asr(event.window.event));

        if (listener != win->window_event_listeners.end()) {
          listener->second.handle();
        }

        return true;
      }

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
        MouseClickEvent mouse_event;
        mouse_event.mouse_id = MouseID{event.button.which};
        mouse_event.offset = OffsetI{event.button.x, event.button.y};
        mouse_event.clicks = event.button.clicks;

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

        get_window_info(WindowID{event.button.windowID})
            ->mouse_click_listener.handle(mouse_event);

        return true;
      }

      case SDL_MOUSEMOTION: {
        MouseMotionEvent mouse_event;

        mouse_event.mouse_id = MouseID{event.motion.which};
        mouse_event.offset.x = event.motion.x;
        mouse_event.offset.y = event.motion.y;
        mouse_event.translation.x = event.motion.xrel;
        mouse_event.translation.y = event.motion.yrel;

        get_window_info(WindowID{event.button.windowID})
            ->mouse_motion_listener.handle(mouse_event);

        return true;
      }

      case SDL_MOUSEWHEEL: {
        return true;
      }

      case SDL_QUIT: {
        get_window_info(WindowID{event.button.windowID})
            ->quit_listener.handle();

        return true;
      }

        // TODO(lamarrr): forward other events
        // case SDL_CLIPBOARDUPDATE

        // case SDL_DROPFILE
        // case SDL_DROPFILE
        // case SDL_DROPBEGIN
        // case SDL_DROPCOMPLETE

        // TODO(lamarrr): add requestkeyoardinput
        // SDL_KEYDOWN
        // SDL_KEYUP,
        // SDL_TEXTEDITING,
        // SDL_TEXTINPUT,
        // SDL_KEYMAPCHANGED,

        // Future
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

}  // namespace asr
