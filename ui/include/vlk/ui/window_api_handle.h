#pragma once

#include <cinttypes>
#include <map>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "vlk/ui/sdl_utils.h"
#include "vlk/ui/window_event_queue.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

struct WindowInfo {
  // queue to receive events from
  WindowEventQueue* queue = nullptr;
};

enum class WindowID : uint32_t {};

inline WindowEvent sdl_window_event_to_vlk(uint8_t win_event_type) {
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

struct WindowApiHandle {
  bool is_initialized = false;

  VLK_MAKE_HANDLE(WindowApiHandle)

  void init() {
    VLK_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0,
                   "Unable to initialize SDL");
    is_initialized = true;
  }

  ~WindowApiHandle() {
    if (is_initialized) {
      SDL_Quit();
    }
  }

  // this implies that we need to detach the handle from here first
  //
  // also ensure all api calls occur on the main thread.
  std::map<WindowID, WindowInfo> windows_info;

  void add_window_info(WindowID id, WindowInfo info) {
    windows_info.emplace(id, info);
  }

  WindowInfo get_window(WindowID id) {
    auto window_entry_pos = windows_info.find(id);
    VLK_ENSURE(window_entry_pos != windows_info.end());

    return window_entry_pos->second;
  }

  // must be a valid window registered with this api and added with the
  // 'add_window_info' method
  void remove_window_info(WindowID id) {
    auto pos = windows_info.find(id);
    VLK_ENSURE(pos != windows_info.end());
    windows_info.erase(pos);
  }

  // TODO(lamarrr): keyboard events must be handled as combinations
  bool poll_events() {
    SDL_Event event{};

    if (SDL_PollEvent(&event) == 1) {
      switch (event.type) {
        case SDL_WINDOWEVENT: {
          get_window(WindowID{event.window.windowID})
              .queue->add_raw(sdl_window_event_to_vlk(event.window.event));
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
          // SDL_StartTextInput()
          VLK_LOG("timestamp: {}", event.button.timestamp);
          get_window(WindowID{event.button.windowID})
              .queue->add_raw(mouse_event);
        };

        default: {
          return true;
        }
      }

    } else {
      return false;
    }
  }
};

}  // namespace ui
}  // namespace vlk
