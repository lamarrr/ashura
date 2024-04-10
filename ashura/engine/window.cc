#include "ashura/engine/window.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/sparse_vec.h"
#include "ashura/std/vec.h"

namespace ash
{

#define SDL_CHECK_EX(description, ...)                                      \
  if (!(__VA_ARGS__))                                                       \
  {                                                                         \
    (default_logger)                                                        \
        ->panic(description, ", SDL Error: ", SDL_GetError(),               \
                " (expression: " #__VA_ARGS__,                              \
                ") [function: ", ::ash::SourceLocation::current().function, \
                ", file: ", ::ash::SourceLocation::current().file, ":",     \
                ::ash::SourceLocation::current().line, ":",                 \
                ::ash::SourceLocation::current().column, "]");              \
  }

#define SDL_CHECK(...) SDL_CHECK_EX("", __VA_ARGS__)

#define CHECK_SDL_ERRC(...) SDL_CHECK_EX("", !(__VA_ARGS__))

namespace sdl
{
struct WindowEventListener
{
  Fn<void(WindowEvent const &)> callback = to_fn([](WindowEvent const &) {});
  WindowEventTypes              types    = WindowEventTypes::None;
};

struct Window
{
  SDL_Window              *win              = nullptr;
  gfx::Surface             surface          = nullptr;
  uid32                    backend_id       = UID32_INVALID;
  Vec<WindowEventListener> listeners        = {};
  SparseVec<u32>           listeners_id_map = {};
};

struct WindowSystemImpl final : public WindowSystem
{
  Vec<Window>    windows;
  SparseVec<u32> id_map;

  SDL_Window *hnd(uid32 id)
  {
    return windows[id_map[id]].win;
  }

  Window *win(uid32 id)
  {
    return &windows[id_map[id]];
  }

  void init()
  {
    CHECK_SDL_ERRC(SDL_Init(SDL_INIT_VIDEO));
  }

  Option<uid32> create_window(gfx::InstanceImpl instance,
                              char const       *title) override
  {
    SDL_Window *window = SDL_CreateWindow(
        title, 1920, 1080, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_CHECK(window != nullptr);
    uid32 backend_id = SDL_GetWindowID(window);
    SDL_CHECK(backend_id != 0);

    CHECK(instance.interface->get_backend(instance.self) ==
          gfx::Backend::Vulkan);

    vk::Instance *vk_instance = (vk::Instance *) instance.self;
    VkSurfaceKHR  surface;

    SDL_CHECK(SDL_Vulkan_CreateSurface(window, vk_instance->vk_instance,
                                       nullptr, &surface) == SDL_TRUE);

    uid32 out_id;
    CHECK(id_map.push(
        [&](uid32 id, u32) {
          out_id = id;
          CHECK(windows.push(Window{
              .win = window, .surface = surface, .backend_id = backend_id}));
        },
        windows));

    return Some{out_id};
  }

  void destroy_window(uid32 window) override
  {
    if (window != UID32_INVALID)
    {
      CHECK(false);
    }
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
                                     static_cast<int>(size.y)));
  }

  void center(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowPosition(hnd(window), SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED));
  }

  Vec2U get_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowSize(hnd(window), &w, &h));
    return Vec2U{static_cast<u32>(w), static_cast<u32>(h)};
  }

  Vec2U get_surface_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowSizeInPixels(hnd(window), &w, &h));
    return Vec2U{static_cast<u32>(w), static_cast<u32>(h)};
  }

  void set_position(uid32 window, Vec2I pos) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowPosition(hnd(window), pos.x, pos.y));
  }

  Vec2I get_position(uid32 window) override
  {
    int x, y;
    CHECK_SDL_ERRC(SDL_GetWindowPosition(hnd(window), &x, &y));
    return Vec2I{x, y};
  }

  void set_min_size(uid32 window, Vec2U min) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowMinimumSize(
        hnd(window), static_cast<int>(min.x), static_cast<int>(min.y)));
  }

  Vec2U get_min_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowMinimumSize(hnd(window), &w, &h));
    return Vec2U{static_cast<u32>(w), static_cast<u32>(h)};
  }

  void set_max_size(uid32 window, Vec2U max) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowMaximumSize(
        hnd(window), static_cast<int>(max.x), static_cast<int>(max.y)));
  }

  Vec2U get_max_size(uid32 window) override
  {
    int w, h;
    CHECK_SDL_ERRC(SDL_GetWindowMaximumSize(hnd(window), &w, &h));
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
        CHECK(false);
    }

    SDL_Surface *icon = SDL_CreateSurfaceFrom(
        (void *) image.span.data(), static_cast<int>(image.width),
        static_cast<int>(image.height), static_cast<int>(image.pitch), fmt);
    SDL_CHECK(icon != nullptr);
    CHECK_SDL_ERRC(SDL_SetWindowIcon(hnd(window), icon));
    SDL_DestroySurface(icon);
  }

  void make_bordered(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowBordered(hnd(window), SDL_TRUE));
  }

  void make_borderless(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowBordered(hnd(window), SDL_FALSE));
  }

  void show(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_ShowWindow(hnd(window)));
  }

  void hide(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_HideWindow(hnd(window)));
  }

  void raise(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_RaiseWindow(hnd(window)));
  }

  void restore(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_RestoreWindow(hnd(window)));
  }

  void request_attention(uid32 window, bool briefly) override
  {
    CHECK_SDL_ERRC(SDL_FlashWindow(
        hnd(window), briefly ? SDL_FLASH_BRIEFLY : SDL_FLASH_UNTIL_FOCUSED));
  }

  void make_fullscreen(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowFullscreen(hnd(window), SDL_TRUE));
  }

  void make_windowed(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowFullscreen(hnd(window), SDL_FALSE));
  }

  void make_resizable(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowResizable(hnd(window), SDL_TRUE));
  }

  void make_unresizable(uid32 window) override
  {
    CHECK_SDL_ERRC(SDL_SetWindowResizable(hnd(window), SDL_FALSE));
  }

  uid32 listen(uid32 window, WindowEventTypes event_types,
               Fn<void(WindowEvent const &)> callback) override
  {
    uid32   out_id;
    Window *pwin = win(window);
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
    Window *pwin = win(window);
    pwin->listeners_id_map.erase(listener, pwin->listeners);
  }

  gfx::Surface get_surface(uid32 window) override
  {
    Window *pwin = win(window);
    return pwin->surface;
  }

  void publish_event(uid32 backend_id, WindowEvent const &event)
  {
    for (Window const &win : windows)
    {
      if (win.backend_id == backend_id)
      {
        for (WindowEventListener const &listener : win.listeners)
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
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Shown});
          return;
        case SDL_EVENT_WINDOW_HIDDEN:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Hidden});
          return;
        case SDL_EVENT_WINDOW_EXPOSED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Exposed});
          return;
        case SDL_EVENT_WINDOW_MOVED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Moved});
          return;
        case SDL_EVENT_WINDOW_RESIZED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Resized});
          return;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
          publish_event(event.window.windowID,
                        WindowEvent{.none_ = 0,
                                    .type  = WindowEventTypes::SurfaceResized});
          return;
        case SDL_EVENT_WINDOW_MINIMIZED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Minimized});
          return;
        case SDL_EVENT_WINDOW_MAXIMIZED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Maximized});
          return;
        case SDL_EVENT_WINDOW_RESTORED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Restored});
          return;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::MouseEnter});
          return;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::MouseLeave});
          return;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::FocusGained});
          return;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::FocusLost});
          return;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          publish_event(event.window.windowID,
                        WindowEvent{.none_ = 0,
                                    .type  = WindowEventTypes::CloseRequested});
          return;
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::TakeFocus});
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
                              Vec2{event.wheel.mouse_x, event.wheel.mouse_y},
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
          publish_event(
              event.window.windowID,
              WindowEvent{.none_ = 0, .type = WindowEventTypes::Destroyed});
          return;

        case SDL_EVENT_SYSTEM_THEME_CHANGED:
          return;
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
          return;
        case SDL_EVENT_DROP_BEGIN:
        case SDL_EVENT_DROP_COMPLETE:
        case SDL_EVENT_DROP_FILE:
        case SDL_EVENT_DROP_POSITION:
        case SDL_EVENT_DROP_TEXT:
          return;
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

/*
  // void enable_hit_testing();
  //
  // SDL_HITTEST_NORMAL
  // region is normal and has no special properties
  // SDL_HITTEST_DRAGGABLE
  // region can drag entire window
  // SDL_HITTEST_RESIZE_TOPLEFT
  // region can resize top left window
  // SDL_HITTEST_RESIZE_TOP
  // region can resize top window
  // SDL_HITTEST_RESIZE_TOPRIGHT
  // region can resize top right window
  // SDL_HITTEST_RESIZE_RIGHT
  // region can resize right window
  // SDL_HITTEST_RESIZE_BOTTOMRIGHT
  // region can resize bottom right window
  // SDL_HITTEST_RESIZE_BOTTOM
  // region can resize bottom window
  // SDL_HITTEST_RESIZE_BOTTOMLEFT
  // region can resize bottom left window
  // SDL_HITTEST_RESIZE_LEFT
  // region can resize left window
  //

enum class SwapChainState : u8
{
  Ok            = 0,
  ExtentChanged = 1,        // the window's extent and surface (framebuffer)
                            // extent has changed
  Suboptimal =
      2,        // the window swapchain can still be used for presentation but
                // is not optimal for presentation in its present state
  OutOfDate = 4,        // the window swapchain is now out of date and needs to
                        // be changed
  All = 7
};

*/

}        // namespace sdl

WindowSystem *init_sdl_window_system()
{
  static sdl::WindowSystemImpl impl;
  impl.init();
  return &impl;
}
}        // namespace ash