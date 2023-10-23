#pragma once
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string_view>

#include "ashura/image.h"
#include "ashura/image_decoder.h"
#include "ashura/primitives.h"
#include "ashura/subsystem.h"
#include "ashura/subsystems/image_manager.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_context.h"
#include "stx/memory.h"
#include "stx/rc.h"
#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/schedule.h"
#include "stx/span.h"
#include "stx/void.h"

namespace ash
{

struct VulkanImageManager : public ImageManager
{
  VulkanImageManager(vk::RenderResourceManager &imgr) : mgr{&imgr}
  {
  }

  virtual constexpr void on_startup(Context &ctx) override
  {
  }

  virtual constexpr void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
  }

  virtual constexpr void on_exit(Context &ctx) override
  {
  }

  virtual constexpr ~VulkanImageManager() override
  {
  }

  virtual gfx::image add(ImageView<u8 const> view, bool is_real_time) override
  {
    return mgr->add_image(view, is_real_time);
  }

  virtual void update(gfx::image image, ImageView<u8 const> view)
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
