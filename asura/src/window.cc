
#include "asura/ui/window.h"

#include "asura/ui/sdl_utils.h"
#include "asura/ui/window_api_handle.h"
#include "asura/ui/window_handle.h"

namespace asr {
namespace ui {

Window Window::create(WindowApi const& api, WindowCfg cfg) {
  stx::Rc<WindowHandle*> handle =
      stx::rc::make_inplace<WindowHandle>(stx::os_allocator).unwrap();

  // width and height here refer to the screen coordinates and not the
  // actual pixel coordinates (cc: Device Pixel Ratio)

  auto window_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;

  if (cfg.hidden) {
    window_flags |= SDL_WINDOW_HIDDEN;
  } else {
    window_flags |= SDL_WINDOW_SHOWN;
  }

  if (cfg.resizable) {
    window_flags |= SDL_WINDOW_RESIZABLE;
  }

  if (cfg.borderless) {
    window_flags |= SDL_WINDOW_BORDERLESS;
  }

  if (cfg.fullscreen) {
    window_flags |= SDL_WINDOW_FULLSCREEN;
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
  ASR_SDL_ENSURE(window != nullptr, "Unable to create window");

  cfg.min_extent.copy().match(
      [&](Extent min_extent) {
        SDL_SetWindowMinimumSize(window, i32_clamp(min_extent.width),
                                 i32_clamp(min_extent.height));
      },
      []() {});

  cfg.max_extent.copy().match(
      [&](Extent max_extent) {
        SDL_SetWindowMaximumSize(window, i32_clamp(max_extent.width),
                                 i32_clamp(max_extent.height));
      },
      []() {});

  if (cfg.enable_hit_testing) {
    //[[maybe_unused]] int result = SDL_SetWindowHitTest();
  }

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  handle.handle->window = window;
  handle.handle->id = WindowID{SDL_GetWindowID(window)};
  handle.handle->api = api;
  handle.handle->cfg = std::move(cfg);

  WindowInfo info{};
  info.queue = &handle.handle->event_queue;
  api.handle->add_window_info(handle.handle->id, info);

  return Window{std::move(handle)};
}

void Window::attach_surface(vk::Instance const& instance) const {
  WindowSurface surface;
  surface.handle =
      std::shared_ptr<WindowSurfaceHandle>(new WindowSurfaceHandle{});

  ASR_SDL_ENSURE(
      SDL_Vulkan_CreateSurface(handle.handle->window, instance.handle->instance,
                               &surface.handle->surface) == SDL_TRUE,
      "Unable to create surface for window");

  ASR_ENSURE(surface.handle->surface != nullptr);

  surface.handle->instance = instance;

  handle.handle->surface = std::move(surface);
}

}  // namespace ui
}  // namespace asr
