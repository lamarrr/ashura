#include "ashura/span.h"
#include "ashura/vulkan_gfx.h"

using namespace ash;

int main()
{
  Logger      logger;
  StdioSink   sink;
  LogSinkImpl sink_impl{.self      = (LogSink) &sink,
                        .interface = &stdio_sink_interface};

  if (!create_logger(&logger, {&sink_impl, 1}, heap_allocator))
  {
    abort();
  }
  gfx::InstanceImpl instance =
      vk::instance_interface.create(heap_allocator, &logger, true).unwrap();

  gfx::DeviceType pref[] = {gfx::DeviceType::DiscreteGpu,
                            gfx::DeviceType::VirtualGpu,
                            gfx::DeviceType::IntegratedGpu,
                            gfx::DeviceType::Cpu, gfx::DeviceType::Other};
  gfx::DeviceImpl device =  instance.interface->create_device(instance.self, span::from_array(pref),
                                    {}, heap_allocator).unwrap();

  gfx::Buffer buffer = device.interface->create_buffer(  device.self, gfx::BufferDesc{
                       .label      = "STAGING_BUFFER",
                       .host_mapped = true,
                       .size  = 1024,
                       .usage = gfx::BufferUsage::TransferSrc|gfx::BufferUsage::UniformTexelBuffer}).unwrap();

  void *map =
      device.interface->get_buffer_memory_map(device.self, buffer).unwrap();

  ((char *) map)[0] = 0;

  device.interface->flush_buffer_memory_map(device.self, buffer,
                                            {0, gfx::WHOLE_SIZE});
  device.interface->invalidate_buffer_memory_map(device.self, buffer,
                                                 {0, gfx::WHOLE_SIZE});

  gfx::Image image = device.interface->create_image(
      device.self, gfx::ImageDesc{.label        = "ATTACHMENT_0",
                          .type         = gfx::ImageType::Type2D,
                          .format       = gfx::Format::R8G8B8A8_UNORM,
                          .usage        = gfx::ImageUsage::ColorAttachment,
                          .aspects      = gfx::ImageAspects::Color,
                          .extent       = gfx::Extent3D{200, 200, 1},
                          .mip_levels   = 1,
                          .array_layers = 2}).unwrap();

  gfx::ImageView view =               device.interface->create_image_view(   device.self,   gfx::ImageViewDesc{
                                .label = "ATTACHMENT_0_VIEW",
                                .image = image,
                                .view_type = gfx::ImageViewType::Type2D,
                                .view_format = gfx::Format::R8G8B8A8_UNORM,
                                .mapping = gfx::ComponentMapping{},
                                .aspects = gfx::ImageAspects::Color,
                                .first_mip_level = 0,
                                .num_mip_levels = 1,
                                .first_array_layer = 1,
                                .num_array_layers = 1
                            }
                          ).unwrap();

  device.interface->create_buffer_view(
      device.self, gfx::BufferViewDesc{.label  = "BUFFER_VIEW_0",
                                       .buffer = buffer,
                                       .format = gfx::Format::R8G8B8A8_UNORM,
                                       .offset = 0,
                                       .size   = 1024});

  gfx::RenderPass render_pass = device.interface->create_render_pass(   device.self, gfx::RenderPassDesc{
        .label = "RENDER_PASS_0",
        .color_attachments = span::from_initializer_list({
            gfx::RenderPassAttachment{
                .format = gfx::Format::B8G8R8A8_UNORM
            }
        }),
        .input_attachments = {},
        .depth_stencil_attachment={}
    } ).unwrap();

  gfx::DescriptorSetLayout set_layout =  device.interface->create_descriptor_set_layout(  device.self,  gfx::DescriptorSetLayoutDesc{ .label = "main set layout",
.bindings = span::from_initializer_list({
    gfx::DescriptorBindingDesc{
        .type = gfx::DescriptorType::CombinedImageSampler,
        .count = 2
    },
    gfx::DescriptorBindingDesc{
         .type = gfx::DescriptorType::StorageImage,
        .count = 4
    },
    gfx::DescriptorBindingDesc{
        .type = gfx::DescriptorType::InputAttachment,
        .count = 8
    }})}).unwrap();

  gfx::DescriptorHeapImpl descriptor_heap =    device.interface->create_descriptor_heap(device.self,
   span::from_initializer_list({set_layout}), 200, heap_allocator ).unwrap();

  u32 group =
      descriptor_heap.interface->add_group(descriptor_heap.self, 0).unwrap();

  gfx::DescriptorHeapStats stats =
      descriptor_heap.interface->get_stats(descriptor_heap.self);

  logger.info("Num Pools: ", stats.num_pools,
              ", Num allocated groups: ", stats.num_allocated_groups,
              ", Num free groups: ", stats.num_free_groups,
              ", Num released: ", stats.num_released_groups);

  logger.info("Exiting");
}
