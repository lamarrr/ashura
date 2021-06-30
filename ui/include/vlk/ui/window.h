#pragma once

#include <memory>
#include <string>

#include "vlk/ui/primitives.h"
#include "vlk/ui/window_api.h"

namespace vlk {

namespace vk {
struct Instance;
};

namespace ui {

// TODO(lamarrr): log selections

struct WindowCfg {
  Extent extent = Extent{1920, 1080};
  std::string title;
  bool resizable = true;
  bool maximized = false;
  // borderless
  // fullscreen
  // minimized
};

struct WindowHandle;

// TODO(lamarrr): window should be destructed on the same thread that created
// it, and it should call its api so it's info and poll info can be removed

struct Window {
  // creates a window without a surface
  static Window create(WindowApi const& api, WindowCfg const& cfg);

  // attach surface to window for presentation
  void attach_surface(vk::Instance const& instance) const;

  // we need sksurface or skpicture or skimage backbuffer here
  // dirtiness detection
  std::shared_ptr<WindowHandle> handle;
};

}  // namespace ui
}  // namespace vlk
