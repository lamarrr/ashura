#include "ashura/engine/passes/root.h"

namespace ash
{

void RootPass::acquire_view(Pass self, RenderServer *server, uid32 view_id)
{
  View              *view   = server->get_view(view_id).unwrap();
  gfx::DeviceImpl    device = server->device;
  gfx::SwapchainInfo swapchain_info =
      device->get_swapchain_info(device.self, server->swapchain).unwrap();

  device
      ->create_image(device.self,
                     gfx::ImageDesc{.label   = nullptr,
                                    .type    = gfx::ImageType::Type2D,
                                    .format  = gfx::Format::B8G8R8A8_UNORM,
                                    .usage   = gfx::ImageUsage::ColorAttachment,
                                    .aspects = gfx::ImageAspects::Color,
                                    .extent  = {0x00, 0x00},
                                    .mip_levels   = 1,
                                    .array_layers = 1})
      .unwrap();
      
  view->resources.emplace("Root:COLOR_IMAGE"_span,   );
}

}        // namespace ash