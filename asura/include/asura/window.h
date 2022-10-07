#pragma once

#include <memory>
#include <string>
#include <variant>

#include "asura/primitives.h"
#include "asura/window_api.h"
#include "stx/rc.h"
#include "stx/string.h"

namespace asr {

namespace vk {
struct Instance;
}

namespace ui {

enum class WindowTypeHint : uint8_t { Normal, Utility, Tooltip, Popup };

enum class WindowPosition : uint8_t { Centered };

struct WindowCfg {
  // TODO(lamarrr): remove this constant
  Extent extent{1920, 1080};
  stx::Option<Extent> min_extent;
  stx::Option<Extent> max_extent;
  stx::String title;
  WindowTypeHint type_hint = WindowTypeHint::Normal;
  bool resizable = true;
  bool fullscreen = false;
  bool borderless = false;
  bool hidden = false;
  bool enable_hit_testing =
      false; // TODO(lamarrr) needed for borderless windows
  std::variant<WindowPosition, IOffset> position;
};

struct WindowHandle;

// TODO(lamarrr): window should be destructed on the same thread that created
// it, and it should call its api so it's info and poll info can be removed

// TODO(lamarrr): ensure render context is not copied from just anywhere and
// they use references
struct Window {
  // creates a window without a surface
  static Window create(WindowApi const &api, WindowCfg cfg);

  // attach surface to window for presentation
  void attach_surface(vk::Instance const &instance) const;

  // this functionality should be exposed via the windowcontroller subsystem
  // fullscreen(true)
  //
  //

  void set_title(stx::String const &title);
  void position(IOffset pos);
  void set_icon(uint8_t *rgba_pixels, Extent extent);
  void make_bordered();
  void make_borderless();
  void show();
  void hide();
  void raise();
  void maximize();
  void minimize();
  void restore();
  void make_fullscreen();
  void make_nonfullscreen();
  void enable_hit_testing();
  void make_resizable();
  void make_unresizable();
  void center();
  void resize();
  void constrain_max(stx::Option<int> width, stx::Option<int> height);
  void constrain_min(stx::Option<int> width, stx::Option<int> height);

  // we need sksurface or skpicture or skimage backbuffer here
  // dirtiness detection
  stx::Rc<WindowHandle *> handle;
};

} // namespace ui
} // namespace asr
