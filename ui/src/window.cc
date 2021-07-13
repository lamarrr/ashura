
#include "vlk/ui/window.h"

#include "vlk/ui/sdl_utils.h"
#include "vlk/ui/window_api_handle.h"
#include "vlk/ui/window_handle.h"

namespace vlk {
namespace ui {

Window Window::create(WindowApi const& api, WindowCfg const& cfg) {
  std::shared_ptr<WindowHandle> handle{new WindowHandle{}};

  // width and height here refer to the screen coordinates and not the
  // actual pixel coordinates (cc: Device Pixel Ratio)

  auto window_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;

  if (cfg.borderless) {
    window_flags |= SDL_WINDOW_BORDERLESS;
  }

  if (cfg.fullscreen) {
    window_flags |= SDL_WINDOW_FULLSCREEN;
  }

  if (cfg.resizable) {
    window_flags |= SDL_WINDOW_RESIZABLE;
  }

  if (cfg.hidden) {
    window_flags |= SDL_WINDOW_HIDDEN;
  } else {
    window_flags |= SDL_WINDOW_SHOWN;
  }

  if (cfg.resizable) {
    window_flags |= SDL_WINDOW_RESIZABLE;
  }

  if (cfg.state == WindowState::Normal) {
  } else if (cfg.state == WindowState::Minimized) {
    window_flags |= SDL_WINDOW_MINIMIZED;
  } else if (cfg.state == WindowState::Maximized) {
    window_flags |= SDL_WINDOW_MAXIMIZED;
  }

  if (cfg.type_hint == WindowTypeHint::Normal) {
  } else if (cfg.type_hint == WindowTypeHint::Popup) {
    window_flags |= SDL_WINDOW_POPUP_MENU;
  } else if (cfg.type_hint == WindowTypeHint::Tooltip) {
    window_flags |= SDL_WINDOW_TOOLTIP;
  } else if (cfg.type_hint == WindowTypeHint::Utility) {
    window_flags |= SDL_WINDOW_UTILITY;
  }

  SDL_Window* window =
      SDL_CreateWindow(cfg.title.c_str(), 0, 0, i32_clamp(cfg.extent.width),
                       i32_clamp(cfg.extent.height), window_flags);

  // window creation shouldn't fail reliably, if it fails,
  // there's no point in the program proceeding
  VLK_SDL_ENSURE(window != nullptr, "Unable to create window");

  cfg.min_extent.clone().match(
      [&](Extent min_extent) {
        SDL_SetWindowMinimumSize(window, i32_clamp(min_extent.width),
                                 i32_clamp(min_extent.height));
      },
      []() {});

  cfg.max_extent.clone().match(
      [&](Extent max_extent) {
        SDL_SetWindowMaximumSize(window, i32_clamp(max_extent.width),
                                 i32_clamp(max_extent.height));
      },
      []() {});

  if (cfg.enable_hit_testing) {
    //[[maybe_unused]] int result = SDL_SetWindowHitTest();
  }

  if (true) {
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
  }

  handle->window = window;
  handle->id = WindowID{SDL_GetWindowID(window)};
  handle->api = api;
  handle->cfg = cfg;

  WindowInfo info{};
  info.queue = &handle->event_queue;
  api.handle->add_window_info(handle->id, info);

  return Window{std::move(handle)};
}

void Window::attach_surface(vk::Instance const& instance) const {
  WindowSurface surface;
  surface.handle =
      std::shared_ptr<WindowSurfaceHandle>(new WindowSurfaceHandle{});

  VLK_SDL_ENSURE(
      SDL_Vulkan_CreateSurface(handle->window, instance.handle->instance,
                               &surface.handle->surface) == SDL_TRUE,
      "Unable to create surface for window");

  VLK_ENSURE(surface.handle->surface != nullptr);

  surface.handle->instance = instance;

  handle->surface = std::move(surface);
}

}  // namespace ui
}  // namespace vlk
