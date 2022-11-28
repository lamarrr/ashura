#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <variant>

#include "SDL.h"
#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "ashura/window.h"
#include "ashura/window_api.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/string.h"

namespace asr {
using namespace stx::literals;

enum class WindowTypeHint : u8 { Normal, Utility, Tooltip, Popup };

enum class WindowPosition : u8 { Centered };

struct WindowConfig {
  stx::String title = "ashura"_str;
  Extent extent{1920, 1080};  // TODO(lamarrr): GetDisplaySize
  stx::Option<Extent> min_extent;
  stx::Option<Extent> max_extent;
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

// TODO(lamarrr): ensure render context is not copied from just anywhere and
// they use references
// TODO(lamarrr): we don't need vecs in the parameter,only an allocator
struct Window {
  Window(stx::Rc<WindowApi*> api, SDL_Window* window, WindowID id,
         Extent extent, Extent surface_extent, WindowConfig cfg,
         std::thread::id init_thread_id)
      : api_{std::move(api)},
        window_{window},
        id_{id},
        window_extent_{extent},
        surface_extent_{surface_extent},
        cfg_{std::move(cfg)},
        init_thread_id_{init_thread_id} {}

  STX_MAKE_PINNED(Window)

  ~Window() {
    // window should be destructed on the same thread that created it
    ASR_ENSURE(init_thread_id_ == std::this_thread::get_id());
    api_.handle->remove_window_info(id_);
    // delete window
    SDL_DestroyWindow(window_);
  }

  void set_title(stx::String const& title) const {
    SDL_SetWindowTitle(window_, title.c_str());
  }

  stx::String get_title() const {
    return stx::string::make(stx::os_allocator, SDL_GetWindowTitle(window_))
        .unwrap();
  }

  void set_position(OffsetI pos) const;

  void center();

  // variant, or centered?
  OffsetI get_position() const;

  void set_icon(stx::Span<uint8_t const> rgba_pixels, Extent extent);

  void make_bordered() const { SDL_SetWindowBordered(window_, SDL_TRUE); }

  void make_borderless() const { SDL_SetWindowBordered(window_, SDL_FALSE); }

  void show() const { SDL_ShowWindow(window_); }

  void hide() const { SDL_HideWindow(window_); }

  void raise() const { SDL_RaiseWindow(window_); }

  void maximize() const { SDL_MaximizeWindow(window_); }

  void minimize() const { SDL_MinimizeWindow(window_); }

  void restore() const { SDL_RestoreWindow(window_); }

  void flash();

  void make_fullscreen() const {
    ASR_ENSURE(
        SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP) == 0);
  }

  void make_nonfullscreen_exclusive() const {
    ASR_ENSURE(SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN) == 0);
  }

  void make_windowed() const {
    ASR_ENSURE(SDL_SetWindowFullscreen(window_, 0) == 0);
  }

  // void enable_hit_testing();

  void make_resizable() const { SDL_SetWindowResizable(window_, SDL_TRUE); }

  void make_unresizable() const { SDL_SetWindowResizable(window_, SDL_FALSE); }

  void resize();

  void constrain(stx::Option<int> min_width, stx::Option<int> min_height,
                 stx::Option<int> max_width, stx::Option<int> max_height);

  stx::Vec<char const*> get_required_instance_extensions() const;

  // attach surface to window for presentation
  void attach_surface(stx::Rc<vk::Instance*> const& instance);

  void recreate_swapchain(stx::Rc<vk::CommandQueue*> const& queue);

  std::pair<WindowSwapchainDiff, u32> acquire_image();

  WindowSwapchainDiff present(u32 next_swapchain_image_index);

  void on(WindowEvent event, stx::UniqueFn<void()> callback) {
    window_event_listeners.emplace(event, std::move(callback));
  }

  // use SDL_EVENT_DISPLAY
  void on_refresh_rate_changed();
  // keyboard
  // mouse
  // controller

  void tick(std::chrono::nanoseconds) {
    SDL_DisplayMode display_mode;
    ASR_ENSURE(SDL_GetWindowDisplayMode(window_, &display_mode) == 0,
               "Unable to get window display mode");
    refresh_rate_ = AS_U32(display_mode.refresh_rate);
    // forward event if refresh rate changed
  }

  stx::Rc<WindowApi*> api_;
  SDL_Window* window_ = nullptr;
  WindowID id_ = WindowID{0};
  Extent window_extent_;
  Extent surface_extent_;
  WindowConfig cfg_;
  std::thread::id init_thread_id_;
  stx::Option<stx::Unique<vk::Surface*>> surface_;
  u32 refresh_rate_ = 1;
  std::map<WindowEvent, stx::UniqueFn<void()>> window_event_listeners;
  stx::UniqueFn<void(MouseClickEvent const&)> mouse_click_listener =
      stx::fn::rc::make_unique_static([](MouseClickEvent const&) {});
  stx::UniqueFn<void(MouseMotionEvent const&)> mouse_motion_listener =
      stx::fn::rc::make_unique_static([](MouseMotionEvent const&) {});
  stx::UniqueFn<void()> quit_listener =
      stx::fn::rc::make_unique_static([]() {});
};

stx::Rc<Window*> create_window(stx::Rc<WindowApi*> api, WindowConfig cfg);

}  // namespace asr
