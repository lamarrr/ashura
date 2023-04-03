#pragma once
#include <chrono>
#include <string_view>

#include "ashura/asset_bundle.h"
#include "ashura/image.h"
#include "ashura/plugin.h"
#include "ashura/plugins/image_bundle.h"
#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_context.h"
#include "stx/rc.h"
#include "stx/span.h"
#include "stx/void.h"

namespace ash
{

struct VulkanImageBundle : public ImageBundle
{
  VulkanImageBundle(vk::RenderResourceManager &imgr) :
      mgr{&imgr}
  {}

  virtual constexpr void on_startup() override
  {}

  virtual constexpr void tick(std::chrono::nanoseconds interval) override
  {}

  virtual constexpr void on_exit() override
  {}

  virtual constexpr ~VulkanImageBundle() override
  {}

  virtual gfx::image add(ImageView view, bool is_real_time) override
  {
    return mgr->add(view, is_real_time);
  }

  virtual void update(gfx::image image, ImageView view)
  {
    mgr->update(image, view);
  }

  virtual void remove(gfx::image image) override
  {
    mgr->remove(image);
  }

  vk::RenderResourceManager *mgr = nullptr;
};

}        // namespace ash
