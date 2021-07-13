#pragma once

#include <memory>
#include <string>
#include <variant>

#include "vlk/ui/primitives.h"
#include "vlk/ui/window_api.h"

namespace vlk {

namespace vk {
struct Instance;
}

namespace ui {

// TODO(lamarrr): log selections

enum class WindowState : uint8_t { Normal, Maximized, Minimized };

enum class WindowTypeHint : uint8_t { Normal, Utility, Tooltip, Popup };

enum class WindowPosition : uint8_t { Centered };

struct WindowCfg {
  Extent extent{1920, 1080};
  stx::Option<Extent> min_extent;
  stx::Option<Extent> max_extent;
  std::string title;
  WindowState state = WindowState::Normal;
  WindowTypeHint type_hint = WindowTypeHint::Normal;
  bool resizable = true;
  bool fullscreen = false;
  bool borderless = false;
  bool hidden = false;
  bool enable_hit_testing = false;  // needed for borderless windows
  std::variant<WindowPosition, IOffset> position;
};

struct WindowHandle;

// TODO(lamarrr): window should be destructed on the same thread that created
// it, and it should call its api so it's info and poll info can be removed

// TODO(lamarrr): ensure render context is not copied from just anywhere and
// they use references
struct Window {
  // creates a window without a surface
  static Window create(WindowApi const& api, WindowCfg const& cfg);

  // attach surface to window for presentation
  void attach_surface(vk::Instance const& instance) const;

  // this functionality should be exposed via the windowcontroller subsystem
  // fullscreen(true)
  //
  //

  void set_title(std::string const& title);
  void show();
  void hide();
  void maximize();
  void minimize();
  // raise this window above others
  void raise();
  void restore();
  void make_fullscreen();
  void make_nonfullscreen();
  void set_icon();
  void make_bordered();
  void make_borderless();
  // TODO(lamarrr): how will this work?
  void enable_hit_testing();
  void make_resizable();
  void make_unresizable();
  void center();
  void position(IOffset const&);
  void resize();
  void constrain_max();
  void constrain_min();

  // we need sksurface or skpicture or skimage backbuffer here
  // dirtiness detection
  std::shared_ptr<WindowHandle> handle;
};

}  // namespace ui
}  // namespace vlk
