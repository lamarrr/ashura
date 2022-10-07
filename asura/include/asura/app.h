#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "asura/primitives.h"
#include "asura/utils.h"
#include "asura/window.h"
#include "asura/window_api.h"
#include "spdlog/logger.h"
#include "stx/string.h"

namespace spdlog {

namespace asr {
namespace ui {

// initial window config
struct AppCfg {
  // refresh rate and various other settings
  stx::String name;
  // WindowCfg root_window_cfg;
  bool enable_validation_layers = false;
};

struct EngineCfg {
  stx::String name = stx::string::make_static("Valkyrie Engine");
  Version version = Version{0, 0, 1};
};

struct Version {
  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t patch = 0;
};

struct App {
  STX_MAKE_PINNED(App)

  App(AppCfg app_cfg) : cfg{std::move(app_cfg)}, root_widget{widget} { init(); }

  void tick();

  ~App();

private:
  // TODO(lamarrr): move this logic into a separate static factory function
  void init();

  std::unique_ptr<WindowApi> window_api;
  std::unique_ptr<Window> window;
  bool window_extent_changed = true;
  bool should_quit = false;
  // used for rendering and presentation
  std::unique_ptr<spdlog::logger> logger;
  uint32_t present_refresh_rate_hz = 0;
  AppCfg cfg;
  EngineCfg engine_cfg;
};

} // namespace ui
} // namespace asr
