#pragma once

#include <memory>
#include <string>
#include <thread>
#include <variant>

#include "SDL.h"
#include "asura/primitives.h"
#include "asura/vulkan.h"
#include "asura/window.h"
#include "asura/window_api.h"
#include "asura/window_event_queue.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/string.h"

namespace asr {

enum class WindowTypeHint : uint8_t { Normal, Utility, Tooltip, Popup };

enum class WindowPosition : uint8_t { Centered };

struct WindowConfig {
  stx::String title = stx::string::make_static("Asura");
  Extent extent{1920, 1080};
  stx::Option<Extent> min_extent;
  stx::Option<Extent> max_extent;
  std::variant<WindowPosition, IOffset> position = WindowPosition::Centered;
  WindowTypeHint type_hint = WindowTypeHint::Normal;
  bool hidden = false;
  bool resizable = true;
  bool borderless = false;
  bool fullscreen = false;
  // needed for borderless windows
  // bool enable_hit_testing = false;
};

// TODO(lamarrr): ensure render context is not copied from just anywhere and
// they use references
struct Window {
  ASR_MAKE_HANDLE(Window)

  ~Window() {
    if (window_ != nullptr) {
      // window should be destructed on the same thread that created it
      ASR_ENSURE(init_thread_id_ == std::this_thread::get_id());
      // delete window
      SDL_DestroyWindow(window_);
    }
  }

  stx::Vec<char const*> get_required_instance_extensions() const;

  // attach surface to window for presentation
  void attach_surface(vk::Instance const& instance) const;

  // this functionality should be exposed via the windowcontroller subsystem
  // fullscreen(true)
  //
  //

  // void set_title(stx::String const& title);
  // void position(IOffset pos);
  // void set_icon(uint8_t *rgba_pixels, Extent extent);
  // void make_bordered();
  // void make_borderless();
  // void show();
  // void hide();
  // void raise();
  // void maximize();
  // void minimize();
  // void restore();
  // void make_fullscreen();
  // void make_nonfullscreen();
  // void enable_hit_testing();
  // void make_resizable();
  // void make_unresizable();
  // void center();
  // void resize();
  // void constrain_max(stx::Option<int> width, stx::Option<int> height);
  // void constrain_min(stx::Option<int> width, stx::Option<int> height);

  stx::Option<stx::Rc<WindowApi*>> api_;
  SDL_Window* window_ = nullptr;
  WindowID id_ = WindowID{0};
  Extent extent_;
  WindowEventQueue event_queue_;
  WindowConfig cfg_;
  std::thread::id init_thread_id_;
  WindowSurface surface_;
  Extent surface_extent_;
};

// creates a window without a surface. TODO(lamarrr): can we create surface
// immediately?
stx::Rc<Window*> create_window(stx::Rc<WindowApi*> api, WindowConfig cfg);

}  // namespace asr
