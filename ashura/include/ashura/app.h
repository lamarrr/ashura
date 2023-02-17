#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include "ashura/engine.h"

namespace ash {

struct App {
  STX_MAKE_PINNED(App)

  explicit App(AppConfig icfg) : cfg{icfg.copy()}, engine{icfg} {}

  void tick(std::chrono::nanoseconds interval);

  ~App() {}

  AppConfig cfg;
  Engine engine;
};

}  // namespace ash
