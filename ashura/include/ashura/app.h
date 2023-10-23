#pragma once

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include "ashura/engine.h"

namespace ash
{

struct App
{
  STX_MAKE_PINNED(App)

  template <Impl<Widget> DerivedWidget>
  explicit App(AppConfig icfg, DerivedWidget &&widget) : cfg{icfg}, engine{icfg, std::move(widget)}
  {
  }

  void tick(std::chrono::nanoseconds interval);

  ~App()
  {
  }

  AppConfig cfg;
  Engine    engine;
};

}        // namespace ash
