
#include "asura/window_api.h"

#include <map>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "asura/sdl_utils.h"
#include "asura/utils.h"

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
        get_window_info(WindowID{event.window.windowID})
            .queue->add_raw(impl::sdl_window_event_to_asr(event.window.event));
        return true;
      }

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP: {
        MouseButtonEvent mouse_event;
        mouse_event.mouse_id = MouseID{event.button.which};
        mouse_event.offset = IOffset{event.button.x, event.button.y};
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
            .queue->add_raw(mouse_event);
      };

        // TODO(lamarrr): forward other events

      default: {
        return true;
      }
    }

  } else {
    return false;
  }
}

}  // namespace asr
