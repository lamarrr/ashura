#include "ashura/engine/window.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/sparse_vec.h"
#include "ashura/std/vec.h"

namespace ash
{

#define SDL_CHECK_EX(description, ...)                                     \
  if (!(__VA_ARGS__))                                                      \
  {                                                                        \
    (panic_logger)                                                         \
        .panic(description, ", SDL Error: ", SDL_GetError(),               \
               " (expression: " #__VA_ARGS__,                              \
               ") [function: ", ::ash::SourceLocation::current().function, \
               ", file: ", ::ash::SourceLocation::current().file, ":",     \
               ::ash::SourceLocation::current().line, ":",                 \
               ::ash::SourceLocation::current().column, "]");              \
  }

#define SDL_CHECK(...) SDL_CHECK_EX("", __VA_ARGS__)

#define CHECK_SDL_ERRC(...) SDL_CHECK_EX("", !(__VA_ARGS__))

struct SDLWinEventListener
{
  Fn<void(WindowEvent const &)> callback = {};
  WindowEventTypes              types    = WindowEventTypes::None;
};

struct SDLWin
{
  SDL_Window              *win        = nullptr;
  gfx::Surface             surface    = nullptr;
  uid32                    backend_id = UID32_INVALID;
  Vec<SDLWinEventListener> listeners;
  SparseVec<u32>           listeners_id_map;
};

struct SDLWinSystem final : public WindowSystem
{
  Vec<SDLWin>    windows;
  SparseVec<u32> id_map;

  SDL_Window *hnd(uid32 id)
  {
    return windows[id_map[id]].win;
  }

  SDLWin *win(uid32 id)
  {
    return &windows[id_map[id]];
  }

  void init() override
  {
    SDL_Init(SDL_INIT_EVERYTHING);
  }

  void uninit() override
  {
  }

  Span<char const *const> get_required_vulkan_instance_extensions() override
  {
    u32                num_extensions = 0;
    char const *const *extensions =
        SDL_Vulkan_GetInstanceExtensions(&num_extensions);
    return Span{extensions, num_extensions};
  }

  Option<uid32> create_window(gfx::InstanceImpl instance,
                              char const       *title) override
  {
    SDL_Window *window = SDL_CreateWindow(
        title, 1920, 1080, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    uid32 backend_id = SDL_GetWindowID(window);
    CHECK(window != nullptr);

    CHECK(instance.interface->get_backend(instance.self) ==
          gfx::Backend::Vulkan);

    vk::Instance *vk_instance = (vk::Instance *) instance.self;
    VkSurfaceKHR  surface;

    CHECK_SDL_ERRC(SDL_Vulkan_CreateSurface(window, vk_instance->vk_instance,
                                            nullptr, &surface));

    uid32 out_id;
    CHECK(id_map.push(
        [&](uid32 id, u32 index) {
          out_id = id;
          CHECK(windows.push(SDLWin{
              .win = window, .surface = surface, .backend_id = backend_id}));
        },
        windows));

    return Some{out_id};
  }

  void set_title(uid32 window, char const *title) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowTitle(hnd(window), title));
  }

  char const *get_title(uid32 window) override
  {
    char const *title = SDL_GetWindowTitle(hnd(window));
    SDL_CHECK(title != nullptr);
    return title;
  }

  void maximize(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_MaximizeWindow(hnd(window)));
  }

  void minimize(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_MinimizeWindow(hnd(window)));
  }

  void set_size(uid32 window, Vec2U size) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowSize(hnd(window), static_cast<int>(size.x),
                                     static_cast<int>(size.y)) == 0);
  }

  void center(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowPosition(hnd(window), SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED) == 0);
  }

  Vec2U get_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowSize(hnd(window), &w, &h) == 0);
    return Vec2U{static_cast<u32>(w), static_cast<u32>(h)};
  }

  Vec2U get_surface_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowSizeInPixels(hnd(window), &w, &h) == 0);
    return Vec2U{static_cast<u32>(w), static_cast<u32>(h)};
  }

  void set_position(uid32 window, Vec2I pos) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowPosition(hnd(window), pos.x, pos.y) == 0);
  }

  Vec2I get_position(uid32 window) override
  {
    int x, y;
    CHECK_SDL_ERRC(SDL_GetWindowPosition(hnd(window), &x, &y) == 0);
    return Vec2I{x, y};
  }

  void set_min_size(uid32 window, Vec2U min) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowMinimumSize(hnd(window),
                                            static_cast<int>(min.x),
                                            static_cast<int>(min.y)) == 0);
  }

  Vec2U get_min_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowMinimumSize(hnd(window), &w, &h) == 0);
    return Vec2U{static_cast<u32>(w), static_cast<u32>(h)};
  }

  void set_max_size(uid32 window, Vec2U max) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowMaximumSize(hnd(window),
                                            static_cast<int>(max.x),
                                            static_cast<int>(max.y)) == 0);
  }

  Vec2U get_max_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowMaximumSize(hnd(window), &w, &h) == 0);
    return Vec2U{static_cast<u32>(w), static_cast<u32>(h)};
  }

  void set_icon(uid32 window, ImageSpan<u8 const> image) override
  {
    SDL_PixelFormatEnum fmt = SDL_PIXELFORMAT_RGBA8888;

    switch (image.format)
    {
      case gfx::Format::R8G8B8A8_UNORM:
        fmt = SDL_PIXELFORMAT_RGBA8888;
        break;
      case gfx::Format::B8G8R8A8_UNORM:
        fmt = SDL_PIXELFORMAT_BGRA8888;
        break;
      default:
        return;
    }

    SDL_Surface *icon = SDL_CreateSurfaceFrom(
        (void *) image.span.data(), static_cast<int>(image.width),
        static_cast<int>(image.height), static_cast<int>(image.pitch), fmt);
    SDL_CHECK(icon != nullptr);
    CHECK_SDL_ERRC(SDL_SetWindowIcon(hnd(window), icon) == 0);
    SDL_DestroySurface(icon);
  }

  void make_bordered(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowBordered(hnd(window), SDL_TRUE) == 0);
  }

  void make_borderless(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowBordered(hnd(window), SDL_FALSE) == 0);
  }

  void show(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_ShowWindow(hnd(window)) == 0);
  }

  void hide(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_HideWindow(hnd(window)) == 0);
  }

  void raise(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_RaiseWindow(hnd(window)) == 0);
  }

  void restore(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_RestoreWindow(hnd(window)) == 0);
  }

  void request_attention(uid32 window, bool briefly) override
  {
    CHECK_SDL_ERRC(
        SDL_FlashWindow(hnd(window), briefly ? SDL_FLASH_BRIEFLY :
                                               SDL_FLASH_UNTIL_FOCUSED) == 0);
  }

  void make_fullscreen(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowFullscreen(hnd(window), SDL_TRUE) == 0);
  }

  void make_windowed(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowFullscreen(hnd(window), SDL_FALSE) == 0);
  }

  void make_resizable(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowResizable(hnd(window), SDL_TRUE) == 0);
  }

  void make_unresizable(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowResizable(hnd(window), SDL_FALSE) == 0);
  }

  uid32 listen(uid32 window, WindowEventTypes event_types,
               Fn<void(WindowEvent const &)> callback) override
  {
    uid32   out_id;
    SDLWin *pwin = win(window);
    CHECK(pwin->listeners_id_map.push(
        [&](uid32 id, u32) {
          out_id = id;
          CHECK(pwin->listeners.push(callback, event_types));
        },
        pwin->listeners));
    return out_id;
  }

  void unlisten(uid32 window, uid32 listener) override
  {
    SDLWin *pwin = win(window);
    pwin->listeners_id_map.erase(listener, pwin->listeners);
  }

  gfx::Surface get_surface(uid32 window) override
  {
    SDLWin *pwin = win(window);
    return pwin->surface;
  }

  void publish_event(uid32 backend_id, WindowEvent const &event)
  {
    for (SDLWin const &win : windows)
    {
      if (win.backend_id == backend_id)
      {
        for (SDLWinEventListener const &listener : win.listeners)
        {
          if (has_bits(listener.types, event.type))
          {
            listener.callback(event);
          }
        }
        return;
      }
    }
  }

  void poll_events() override
  {
    SDL_Event event;

    if (SDL_PollEvent(&event) == SDL_TRUE)
    {
      switch (event.type)
      {
        case SDL_EVENT_WINDOW_SHOWN:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Shown});
          return;
        case SDL_EVENT_WINDOW_HIDDEN:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Hidden});
          return;
        case SDL_EVENT_WINDOW_EXPOSED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Exposed});
          return;
        case SDL_EVENT_WINDOW_MOVED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Moved});
          return;
        case SDL_EVENT_WINDOW_RESIZED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Resized});
          return;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::SurfaceResized});
          return;
        case SDL_EVENT_WINDOW_MINIMIZED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Minimized});
          return;
        case SDL_EVENT_WINDOW_MAXIMIZED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Maximized});
          return;
        case SDL_EVENT_WINDOW_RESTORED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Restored});
          return;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::MouseEnter});
          return;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::MouseLeave});
          return;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::FocusGained});
          return;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::FocusLost});
          return;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::CloseRequested});
          return;
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::TakeFocus});
          return;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
          MouseClickEvent mouse_event{.mouse_id = event.button.which,
                                      .position =
                                          Vec2{event.button.x, event.button.y},
                                      .clicks = event.button.clicks};
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT:
              mouse_event.button = MouseButtons::Primary;
              break;
            case SDL_BUTTON_RIGHT:
              mouse_event.button = MouseButtons::Secondary;
              break;
            case SDL_BUTTON_MIDDLE:
              mouse_event.button = MouseButtons::Middle;
              break;
            case SDL_BUTTON_X1:
              mouse_event.button = MouseButtons::A1;
              break;
            case SDL_BUTTON_X2:
              mouse_event.button = MouseButtons::A2;
              break;
            default:
              CHECK(false);
          }

          switch (event.type)
          {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
              mouse_event.action = KeyAction::Press;
              break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
              mouse_event.action = KeyAction::Release;
              break;

            default:
              return;
          }

          publish_event(event.button.windowID,
                        WindowEvent{.mouse_click = mouse_event,
                                    .type = WindowEventTypes::MouseClick});
          return;
        }

        case SDL_EVENT_MOUSE_MOTION:
          publish_event(
              event.motion.windowID,
              WindowEvent{
                  .mouse_motion =
                      MouseMotionEvent{
                          .mouse_id = event.motion.which,
                          .position = Vec2{event.motion.x, event.motion.y},
                          .translation =
                              Vec2{event.motion.xrel, event.motion.yrel}},
                  .type = WindowEventTypes::MouseMotion});
          return;

        case SDL_EVENT_MOUSE_WHEEL:
          publish_event(
              event.wheel.windowID,
              WindowEvent{

                  .mouse_wheel =
                      MouseWheelEvent{
                          .mouse_id = event.wheel.which,
                          .position =
                              Vec2{event.wheel.mouseX, event.wheel.mouseY},
                          .translation = Vec2{event.wheel.x, event.wheel.y}},
                  .type = WindowEventTypes::MouseWheel});
          return;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
          publish_event(
              event.key.windowID,
              WindowEvent{
                  .key  = KeyEvent{.key       = (Key) event.key.keysym.sym,
                                   .modifiers = static_cast<KeyModifiers>(
                                      event.key.keysym.mod),
                                   .action = event.type == SDL_EVENT_KEY_DOWN ?
                                                 KeyAction::Press :
                                                 KeyAction::Release},
                  .type = WindowEventTypes::Key});
          return;

        case SDL_EVENT_WINDOW_DESTROYED:
          publish_event(event.window.windowID,
                        WindowEvent{.type = WindowEventTypes::Destroyed});
          return;

        case SDL_EVENT_SYSTEM_THEME_CHANGED:
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
        case SDL_EVENT_DROP_BEGIN:
        case SDL_EVENT_DROP_COMPLETE:
        case SDL_EVENT_DROP_FILE:
        case SDL_EVENT_DROP_POSITION:
        case SDL_EVENT_DROP_TEXT:
        case SDL_EVENT_TEXT_EDITING:
        case SDL_EVENT_TEXT_INPUT:
        case SDL_EVENT_KEYMAP_CHANGED:
        case SDL_EVENT_AUDIO_DEVICE_ADDED:
        case SDL_EVENT_AUDIO_DEVICE_REMOVED:
        case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
        case SDL_EVENT_DISPLAY_ORIENTATION:
        case SDL_EVENT_DISPLAY_ADDED:
        case SDL_EVENT_DISPLAY_REMOVED:
        case SDL_EVENT_DISPLAY_MOVED:
          return;

        default:
          return;
      }
    }
  }
};

SDLWinSystem  sdl_window_system_impl;
WindowSystem *sdl_window_system = &sdl_window_system_impl;

}        // namespace ash