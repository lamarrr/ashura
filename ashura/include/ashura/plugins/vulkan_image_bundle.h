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
  VulkanImageBundle(AssetBundle<stx::Rc<vk::ImageResource *>> &ibundle,
                    vk::UploadContext                         &iupload_context) :
      bundle{&ibundle}, upload_context{&iupload_context}
  {}

  virtual constexpr void on_startup() override
  {}

  virtual constexpr void tick(std::chrono::nanoseconds interval) override
  {}

  virtual constexpr void on_exit() override
  {}

  virtual constexpr ~VulkanImageBundle() override
  {}

  virtual gfx::image add(ImageView view) override
  {
    return bundle->add(upload_context->upload_image(view));
  }

  virtual stx::Result<stx::Void, AssetBundleError> remove(gfx::image image) override
  {
    return bundle->remove(image);
  }

  AssetBundle<stx::Rc<vk::ImageResource *>> *bundle         = nullptr;
  vk::UploadContext                         *upload_context = nullptr;
};

}        // namespace ash
