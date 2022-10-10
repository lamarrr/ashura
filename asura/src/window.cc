
#include "asura/window.h"

#include <thread>

#include "asura/sdl_utils.h"
#include "asura/window_handle.h"

namespace asr {

stx::Vec<char const*> Window::get_required_instance_extensions() const {
  uint32_t ext_count = 0;
  stx::Vec<char const*> required_instance_extensions;

  ASR_SDL_ENSURE(
      SDL_Vulkan_GetInstanceExtensions(window_, &ext_count, nullptr) ==
          SDL_TRUE,
      "Unable to get number of window's required Vulkan instance extensions");

  required_instance_extensions.resize(ext_count).unwrap();

  ASR_SDL_ENSURE(
      SDL_Vulkan_GetInstanceExtensions(
          window_, &ext_count, required_instance_extensions.data()) == SDL_TRUE,
      "Unable to get window's required Vulkan instance extensions");

  return required_instance_extensions;
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

stx::Rc<Window*> create_window(stx::Rc<WindowApi*> api, WindowConfig cfg) {
  // width and height here refer to the screen coordinates and not the
  // actual pixel coordinates (cc: Device Pixel Ratio)

  auto window_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;

  if (cfg.type_hint == WindowTypeHint::Normal) {
  } else if (cfg.type_hint == WindowTypeHint::Popup) {
    window_flags |= SDL_WINDOW_POPUP_MENU;
  } else if (cfg.type_hint == WindowTypeHint::Tooltip) {
    window_flags |= SDL_WINDOW_TOOLTIP;
  } else if (cfg.type_hint == WindowTypeHint::Utility) {
    window_flags |= SDL_WINDOW_UTILITY;
  }

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

  // if (cfg.enable_hit_testing) {
  //[[maybe_unused]] int result = SDL_SetWindowHitTest();
  // }

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  stx::Rc<Window*> win =
      stx::rc::make_inplace<Window>(stx::os_allocator).unwrap();

  win.handle->api_ = stx::Some(std::move(api));
  win.handle->window_ = window;
  win.handle->id_ = WindowID{SDL_GetWindowID(window)};
  // swin.handle->surface_
  win.handle->extent_ = cfg.extent;
  win.handle->surface_extent_ = cfg.extent;
  win.handle->cfg_ = std::move(cfg);
  win.handle->init_thread_id_ = std::this_thread::get_id();

  WindowApi::WindowInfo info;
  info.queue = &win.handle->event_queue_;
  api.handle->add_window_info(win.handle->id_, info);

  return std::move(win);
}

}  // namespace asr
