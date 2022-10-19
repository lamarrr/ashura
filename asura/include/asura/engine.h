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

  stx::Option<stx::Rc<spdlog::logger*>> logger_;
  stx::Option<stx::Rc<WindowApi*>> window_api_;
  stx::Option<stx::Rc<Window*>> root_window_;
  // asset manager
  // plugins & systems

  void tick(std::chrono::nanoseconds interval);
};

}  // namespace asr