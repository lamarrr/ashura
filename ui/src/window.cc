
#include "vlk/ui/window.h"

#include "vlk/ui/sdl_util.h"
#include "vlk/ui/window_api_handle.h"
#include "vlk/ui/window_handle.h"

namespace vlk {
namespace ui {

Window Window::create(WindowApi const& api, WindowCfg const& cfg) {
  std::shared_ptr<WindowHandle> handle{new WindowHandle{}};

  // width and height here refer to the screen coordinates and not the
  // actual pixel coordinates (cc: Device Pixel Ratio)
  SDL_Window* window = SDL_CreateWindow(
      cfg.title.c_str(), 0, 0,
      static_cast<int>(std::clamp<uint32_t>(cfg.extent.width, 0, i32_max)),
      static_cast<int>(std::clamp<uint32_t>(cfg.extent.height, 0, i32_max)),
      SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN |
          (cfg.resizable ? SDL_WINDOW_RESIZABLE : 0));

  // TODO(lamarrr): window creation shouldn't fail reliably
  VLK_SDL_ENSURE(window != nullptr, "Unable to create window");

  handle->window = window;
  handle->id = WindowID{SDL_GetWindowID(window)};
  handle->api = api;
  handle->cfg = cfg;

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
