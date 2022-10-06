#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "stx/string.h"
#include "vlk/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/window.h"
#include "vlk/ui/window_api.h"
#include "vlk/utils.h"

namespace spdlog {
class logger;
}

namespace vlk {

namespace vk {
struct Instance;
struct CommandQueue;
}  // namespace vk

namespace ui {

// forward declarations here

//
//
// Issues: direct context order with app, also given we can't create vulkan
// instance without creating video first
//
//

// initial window config
struct AppCfg {
  // refresh rate and various other settings
  stx::String name;
  WindowCfg window_cfg;
  bool enable_validation_layers = false;
};

struct Pipeline;
struct VkRenderContext;

struct Version {
  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t patch = 0;
};

struct EngineCfg {
  stx::String name = stx::string::make_static("Valkyrie Engine");
  Version version = Version{0, 0, 1};
};

struct App {
  STX_MAKE_PINNED(App)

  App(Widget* widget, AppCfg app_cfg)
      : cfg{std::move(app_cfg)}, root_widget{widget} {
    init();
  }

  virtual void tick();

  ~App();

 private:
  // TODO(lamarrr): move this logic into a separate static factory function
  void init();

  std::unique_ptr<WindowApi> window_api;
  std::unique_ptr<Window> window;
  bool window_extent_changed = true;
  bool should_quit = false;
  // used for rendering and presentation
  std::shared_ptr<VkRenderContext> vk_render_context;
  std::unique_ptr<Pipeline> pipeline;
  std::unique_ptr<Widget> root_widget;
  std::unique_ptr<spdlog::logger> logger;
  uint32_t present_refresh_rate_hz = 0;

  AppCfg cfg;

  EngineCfg engine_cfg{};
};

}  // namespace ui
}  // namespace vlk
