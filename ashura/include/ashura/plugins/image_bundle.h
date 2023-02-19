#pragma once
#include <chrono>
#include <string_view>

#include "ashura/image.h"
#include "ashura/plugin.h"
#include "ashura/primitives.h"
#include "stx/span.h"

namespace ash {

enum class ImageBundleError { None, InvalidId };

struct ImageBundle : public Plugin {
  virtual constexpr void on_startup() override {}

  virtual constexpr void tick(std::chrono::nanoseconds interval) override {}

  virtual constexpr void on_exit() override {}

  virtual constexpr std::string_view get_id() override { return "ImageBundle"; }

  virtual gfx::image add(stx::Span<u8 const> pixels, extent extent);

  virtual ImageBundleError remove(gfx::image image);

  virtual constexpr ~ImageBundle() override {}
};

}  // namespace ash
