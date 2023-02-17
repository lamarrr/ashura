#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include "ashura/engine.h"

namespace ash {

struct App {
  STX_MAKE_PINNED(App)

  explicit App(AppConfig cfg) : cfg_{std::move(cfg)}, engine_{cfg} {}

  void tick(std::chrono::nanoseconds interval);

  ~App() {}

  AppConfig cfg_;
  Engine engine_;
};

}  // namespace ash
