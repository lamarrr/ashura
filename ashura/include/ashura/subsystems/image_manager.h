#pragma once
#include <chrono>
#include <string_view>

#include "ashura/image.h"
#include "ashura/subsystem.h"
#include "ashura/primitives.h"
#include "stx/span.h"
#include "stx/void.h"

namespace ash
{

// TODO(lamarrr): we need widget context for getting other subsystems
struct ImageManager : public Subsystem
{
  virtual constexpr void on_startup(Context &ctx) override
  {}

  virtual constexpr void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {}

  virtual constexpr void on_exit(Context &ctx) override
  {}

  virtual constexpr std::string_view get_name() override
  {
    return "ImageManager";
  }

  virtual constexpr ~ImageManager() override
  {}

  virtual gfx::image add(ImageView view, bool is_real_time)
  {
    return 0;
  }

  virtual void update(gfx::image image, ImageView view)
  {
  }

  virtual void remove(gfx::image image)
  {
  }
};

}        // namespace ash
