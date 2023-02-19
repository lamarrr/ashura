#pragma once
#include <chrono>
#include <string_view>

#include "ashura/asset_bundle.h"
#include "ashura/plugin.h"
#include "ashura/plugins/image_bundle.h"
#include "ashura/vulkan.h"
#include "stx/rc.h"

namespace ash {

struct VulkanImageBundle : public Plugin {
  virtual constexpr void on_startup() override {}

  virtual constexpr void tick(std::chrono::nanoseconds interval) override {}

  virtual constexpr void on_exit() override {}

  virtual constexpr std::string_view get_id() override {
    return "VulkanImageBundle";
  }

  virtual constexpr ~VulkanImageBundle() override {}

  virtual gfx::image add(stx::Span<u8 const> pixels, extent extent);

  virtual ImageBundleError remove(gfx::image image);

  AssetBundle<stx::Rc<vk::ImageResource *>> *bundle = nullptr;
  vk::RecordingContext *recording_context = nullptr;
};

}  // namespace ash
