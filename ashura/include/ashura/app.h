#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include "ashura/engine.h"

namespace asr {

struct App {
  STX_MAKE_PINNED(App)

  App(AppConfig cfg) : cfg_{std::move(cfg)}, engine_{cfg} {}

  void tick(std::chrono::nanoseconds interval);

  ~App();

  AppConfig cfg_;
  Engine engine_;
};

}  // namespace asr
