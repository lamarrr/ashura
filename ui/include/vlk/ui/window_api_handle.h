

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

struct WindowApiHandle {
  WindowApiHandle() {
    VLK_SDL_ENSURE(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0,
                   "Unable to initialize SDL");
  }

  VLK_MAKE_HANDLE(WindowApiHandle)

  ~WindowApiHandle() { SDL_Quit(); }

  // this implies that we need to detach the handle from here first
  //
  // also ensure all api calls occur on the main thread.
  std::map<WindowID, WindowInfo> windows_info;

  void add_window_info(WindowID id, WindowInfo info) {
    windows_info.emplace(id, info);
  }

  // must be a valid window registered with this api and added with the
  // 'add_window_info' method
  void remove_window_info(WindowID id) {
    auto pos = windows_info.find(id);
    VLK_ENSURE(pos != windows_info.end());
    windows_info.erase(pos);
  }

  void poll_events() {
    SDL_Event event{};
    if (SDL_PollEvent(&event) == 1) {
      switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
          MouseButtonEvent mouse_event;
          mouse_event.mouse_id = MouseID{event.button.which};
          mouse_event.offset = IOffset{event.button.x, event.button.y};
          mouse_event.clicks = event.button.clicks;

          switch (event.button.button) {
            case SDL_BUTTON_LEFT:
              mouse_event.button = MouseButton::Primary;
            case SDL_BUTTON_RIGHT:
              mouse_event.button = MouseButton::Secondary;
            case SDL_BUTTON_MIDDLE:
              mouse_event.button = MouseButton::Middle;
          }

          switch (event.button.state) {
            case SDL_PRESSED:
              mouse_event.action = MouseAction::Press;
            case SDL_RELEASED:
              mouse_event.action = MouseAction::Release;
          }

          auto window_entry_pos =
              windows_info.find(WindowID{event.button.windowID});
          VLK_ENSURE(window_entry_pos != windows_info.end());

          window_entry_pos->second.queue->add_raw(mouse_event);

          return;
        };

        default:
          return;
      }
    } else {
      return;
    }
  }
};

}  // namespace ui
}  // namespace vlk
