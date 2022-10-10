#pragma once

#include <chrono>

#include "asura/app_config.h"
#include "asura/version.h"
#include "asura/window.h"
#include "asura/window_api.h"
#include "spdlog/logger.h"
#include "stx/rc.h"
#include "stx/string.h"

namespace asr {

struct Engine {
  Engine(AppConfig const& cfg);

  stx::Option<stx::Unique<spdlog::logger*>> logger_;
  stx::Option<stx::Rc<WindowApi*>> window_api_;
  stx::Option<stx::Unique<Window*>> root_window_;
  // asset manager
  // plugins & systems
  // bool window_extent_changed = true;
  // bool should_quit = false;
  // uint32_t present_refresh_rate_hz = 0;

  void tick(std::chrono::nanoseconds);
};

}  // namespace asr