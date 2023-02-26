#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <variant>

#include "SDL3/SDL.h"
#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "ashura/window.h"
#include "ashura/window_api.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/string.h"

namespace ash {

enum class WindowTypeHint : u8 { Normal, Utility, Tooltip, Popup };

enum class WindowPosition : u8 { Centered };

struct WindowConfig {
  stx::String title = stx::string::make_static("Ashura");
  ash::extent extent{1920, 1080};
  stx::Option<ash::extent> min_extent;
  stx::Option<ash::extent> max_extent;
  WindowTypeHint type_hint = WindowTypeHint::Normal;
  bool hidden = false;
  bool resizable = true;
  bool borderless = false;
  bool fullscreen = false;
  bool always_on_top = false;
  // needed for borderless windows
  // bool enable_hit_testing = false;
  // std::variant<WindowPosition, IOffset> position = WindowPosition::Centered;

  WindowConfig copy() const {
    return WindowConfig{stx::string::make(stx::os_allocator, title).unwrap(),
                        extent,
                        min_extent.copy(),
                        max_extent.copy(),
                        type_hint,
                        hidden,
                        resizable,
                        borderless,
                        fullscreen,
                        always_on_top};
  }
};

enum class WindowSwapchainDiff : u8 {
  None = 0,
  // the window's extent and surface (framebuffer) extent has changed
  Extent = 1,
  // the window swapchain can still be used for presentation but is not optimal
  // for presentation in its present state
  Suboptimal = 2,
  // the window swapchain is now out of date and needs to be changed
  OutOfDate = 4,
  All = Extent | Suboptimal | OutOfDate
};

STX_DEFINE_ENUM_BIT_OPS(WindowSwapchainDiff)

struct Window {
  Window(stx::Rc<WindowApi*> api, SDL_Window* window, WindowID id,
         extent extent, ash::extent surface_extent, WindowConfig cfg,
         std::thread::id init_thread_id)
      : api{std::move(api)},
        window{window},
        id{id},
        window_extent{extent},
        surface_extent{surface_extent},
        cfg{std::move(cfg)},
        init_thread_id{init_thread_id} {}

  STX_MAKE_PINNED(Window)

  ~Window() {
    // window should be destructed on the same thread that created it
    ASH_CHECK(init_thread_id == std::this_thread::get_id());

    if (surface.is_some()) {
      surface.value()->destroy();
    }

    api->remove_window_info(id);

    SDL_DestroyWindow(window);
  }

  void set_title(stx::String const& title) const {
    SDL_SetWindowTitle(window, title.c_str());
  }

  stx::String get_title() const {
    return stx::string::make(stx::os_allocator, SDL_GetWindowTitle(window))
        .unwrap();
  }

  void set_position(offseti pos) const;

  void center();

  // TODO(lamarrr): variant, or centered?
  offseti get_position() const;

  void set_icon(stx::Span<u8 const> rgba_pixels, extent extent);

  void make_bordered() const { SDL_SetWindowBordered(window, SDL_TRUE); }

  void make_borderless() const { SDL_SetWindowBordered(window, SDL_FALSE); }

  void show() const { SDL_ShowWindow(window); }

  void hide() const { SDL_HideWindow(window); }

  void raise() const { SDL_RaiseWindow(window); }

  void maximize() const { SDL_MaximizeWindow(window); }

  void minimize() const { SDL_MinimizeWindow(window); }

  void restore() const { SDL_RestoreWindow(window); }

  void flash();

  void make_fullscreen() const {
    // SDL_SetWindowFullscreenMode()
    ASH_CHECK(SDL_SetWindowFullscreen(window, SDL_TRUE) == 0);
  }

  void make_windowed() const {
    ASH_CHECK(SDL_SetWindowFullscreen(window, SDL_FALSE) == 0);
  }

  // void enable_hit_testing();

  void make_resizable() const { SDL_SetWindowResizable(window, SDL_TRUE); }

  void make_unresizable() const { SDL_SetWindowResizable(window, SDL_FALSE); }

  void resize();

  void constrain(stx::Option<int> min_width, stx::Option<int> min_height,
                 stx::Option<int> max_width, stx::Option<int> max_height);

  // attach surface to window for presentation
  void attach_surface(stx::Rc<vk::Instance*> const& instance);

  void recreate_swapchain(stx::Rc<vk::CommandQueue*> const& queue,
                          spdlog::logger& logger);

  std::pair<WindowSwapchainDiff, u32> acquire_image();

  WindowSwapchainDiff present(VkQueue queue, u32 swapchain_image_index);

  void on(WindowEvents event, stx::UniqueFn<void(WindowEvents)> action) {
    event_listeners.push(std::make_pair(event, std::move(action))).unwrap();
  }

  void on_key_down(stx::UniqueFn<void(Key, KeyModifiers)>);

  void on_key_up(stx::UniqueFn<void(Key, KeyModifiers)>);

  // on keypressed

  /* TODO(lamarrr): repeat all widget events */

  // global: virtual void on_copy();
  // global: virtual void on_cut();
  // global: virtual void on_paste();
  // global: virtual void on_full_screen_change();

  void tick(std::chrono::nanoseconds) {}

  stx::Rc<WindowApi*> api;
  SDL_Window* window = nullptr;
  WindowID id{0};
  extent window_extent;
  extent surface_extent;
  WindowConfig cfg;
  std::thread::id init_thread_id;
  stx::Option<stx::Unique<vk::Surface*>> surface;
  stx::Option<stx::Rc<vk::Instance*>> instance;
  stx::Vec<std::pair<WindowEvents, stx::UniqueFn<void(WindowEvents)>>>
      event_listeners{stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(MouseClickEvent)>> mouse_click_listeners{
      stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(MouseMotionEvent)>> mouse_motion_listeners{
      stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(MouseWheelEvent)>> mouse_wheel_listeners{
      stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(Key, KeyModifiers)>> key_down_listeners{
      stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(Key, KeyModifiers)>> key_up_listeners{
      stx::os_allocator};
};

stx::Rc<Window*> create_window(stx::Rc<WindowApi*> api, WindowConfig cfg);

}  // namespace ash
