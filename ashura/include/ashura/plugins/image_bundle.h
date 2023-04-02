#pragma once
#include <chrono>
#include <string_view>

#include "ashura/image.h"
#include "ashura/plugin.h"
#include "ashura/primitives.h"
#include "stx/span.h"
#include "stx/void.h"

namespace ash
{

// TODO(lamarrr): we need widget context for getting other plugins
struct ImageBundle : public Plugin
{
  virtual constexpr void on_startup() override
  {}

  virtual constexpr void tick(std::chrono::nanoseconds interval) override
  {}

  virtual constexpr void on_exit() override
  {}

  virtual constexpr std::string_view get_id() override
  {
    return "ImageBundle";
  }

  virtual gfx::image add(ImageView view)
  {
    return 0;
  }

  virtual void remove(gfx::image image)
  {
  }

  virtual constexpr ~ImageBundle() override
  {}
};

}        // namespace ash
