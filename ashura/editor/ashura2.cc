#include "ashura/passes/bloom.h"
#include "ashura/passes/blur.h"
#include "ashura/passes/chromatic_aberration.h"
#include "ashura/passes/fxaa.h"
#include "ashura/passes/pbr.h"
#include "ashura/passes/rrect.h"
#include "ashura/renderer.h"
#include "ashura/span.h"
#include "ashura/storage.h"
#include "ashura/string.h"
#include "ashura/vulkan_gfx.h"

int main()
{
  using namespace ash;

  Storage<Logger> storage;
  Logger         *logger = storage;
  StdioSink       sink;
  LogSinkImpl     sink_impl{.self      = (LogSink) &sink,
                            .interface = &stdio_sink_interface};

  if (!create_logger(logger, {&sink_impl, 1}, default_allocator))
  {
    abort();
  }

  gfx::InstanceImpl instance =
      vk::instance_interface.create(default_allocator, logger, true).unwrap();

  gfx::DeviceType pref[] = {gfx::DeviceType::DiscreteGpu, gfx::DeviceType::Cpu,
                            gfx::DeviceType::IntegratedGpu,
                            gfx::DeviceType::VirtualGpu,
                            gfx::DeviceType::Other};
  gfx::DeviceImpl device =
      instance->create_device(instance.self, to_span(pref), {}, default_allocator)
          .unwrap();

  gfx::Buffer buffer =
      device
          ->create_buffer(
              device.self,
              gfx::BufferDesc{.label       = "STAGING_BUFFER_0",
                              .size        = 1024,
                              .host_mapped = true,
                              .usage       = gfx::BufferUsage::TransferDst |
                                       gfx::BufferUsage::TransferSrc |
                                       gfx::BufferUsage::UniformTexelBuffer})
          .unwrap();

  void *map = device->get_buffer_memory_map(device.self, buffer).unwrap();

  ((char *) map)[0] = 0;

  device->flush_buffer_memory_map(device.self, buffer, {0, gfx::WHOLE_SIZE})
      .unwrap();
  device
      ->invalidate_buffer_memory_map(device.self, buffer, {0, gfx::WHOLE_SIZE})
      .unwrap();

  gfx::Image image =
      device
          ->create_image(
              device.self,
              gfx::ImageDesc{.label  = "ATTACHMENT_0",
                             .type   = gfx::ImageType::Type2D,
                             .format = gfx::Format::R8G8B8A8_UNORM,
                             .usage  = gfx::ImageUsage::TransferDst |
                                      gfx::ImageUsage::Sampled |
                                      gfx::ImageUsage::ColorAttachment,
                             .aspects      = gfx::ImageAspects::Color,
                             .extent       = gfx::Extent3D{200, 200, 1},
                             .mip_levels   = 1,
                             .array_layers = 2})
          .unwrap();

  gfx::ImageView view =
      device
          ->create_image_view(
              device.self,
              gfx::ImageViewDesc{.label           = "ATTACHMENT_VIEW_0",
                                 .image           = image,
                                 .view_type       = gfx::ImageViewType::Type2D,
                                 .view_format     = gfx::Format::R8G8B8A8_UNORM,
                                 .mapping         = gfx::ComponentMapping{},
                                 .aspects         = gfx::ImageAspects::Color,
                                 .first_mip_level = 0,
                                 .num_mip_levels  = 1,
                                 .first_array_layer = 1,
                                 .num_array_layers  = 1})
          .unwrap();

  gfx::BufferView buffer_view =
      device
          ->create_buffer_view(
              device.self,
              gfx::BufferViewDesc{.label  = "BUFFER_VIEW_0",
                                  .buffer = buffer,
                                  .format = gfx::Format::R8G8B8A8_UNORM,
                                  .offset = 0,
                                  .size   = 1024})
          .unwrap();

  gfx::RenderPass render_pass =
      device
          ->create_render_pass(
              device.self,
              gfx::RenderPassDesc{
                  .label             = "RENDER_PASS_0",
                  .color_attachments = to_span({gfx::RenderPassAttachment{
                      .format = gfx::Format::R8G8B8A8_UNORM}}),
                  .input_attachments = {},
                  .depth_stencil_attachment = {}})
          .unwrap();

  gfx::Framebuffer framebuffer =
      device
          ->create_framebuffer(
              device.self,
              gfx::FramebufferDesc{.label                    = "FRAMEBUFFER_0",
                                   .render_pass              = render_pass,
                                   .extent                   = {200, 200},
                                   .color_attachments        = to_span({view}),
                                   .depth_stencil_attachment = nullptr,
                                   .layers                   = 1})
          .unwrap();

  gfx::DescriptorSetLayout set_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gfx::DescriptorSetLayoutDesc{
                  .label = "main set layout",
                  .bindings =
                      to_span({gfx::DescriptorBindingDesc{
                                   .type  = gfx::DescriptorType::SampledImage,
                                   .count = 2},
                               gfx::DescriptorBindingDesc{
                                   .type  = gfx::DescriptorType::StorageImage,
                                   .count = 4},
                               gfx::DescriptorBindingDesc{
                                   .type = gfx::DescriptorType::InputAttachment,
                                   .count = 8}})})
          .unwrap();

  gfx::DescriptorHeapImpl descriptor_heap =
      device
          ->create_descriptor_heap(device.self, to_span({set_layout}), 200,
                                   default_allocator)
          .unwrap();

  u32 group   = descriptor_heap->add_group(descriptor_heap.self, 0).unwrap();
  u32 group_1 = descriptor_heap->add_group(descriptor_heap.self, 0).unwrap();

  for (u32 i = 0; i < 1000; i++)
  {
    descriptor_heap->add_group(descriptor_heap.self, 0).unwrap();
  }

  descriptor_heap->release(descriptor_heap.self, group_1);

  descriptor_heap->sampled_image(descriptor_heap.self, group, 0, 0,
                                 to_span({gfx::SampledImageBinding{view},
                                          gfx::SampledImageBinding{view}}));

  gfx::DescriptorHeapStats stats =
      descriptor_heap->get_stats(descriptor_heap.self);

  gfx::FrameContext frame_ctx =
      device
          ->create_frame_context(device.self, 4,
                                 to_span({default_allocator, default_allocator,
                                          default_allocator, default_allocator}))
          .unwrap();

  gfx::FrameInfo frame_info = device->get_frame_info(device.self, frame_ctx);

  gfx::CommandEncoderImpl command_encoder =
      frame_info.command_encoders[frame_info.current_command_encoder];

  command_encoder->begin(command_encoder.self);
  command_encoder->begin_debug_marker(command_encoder.self, "intialization",
                                      {1, 1, 1, 1});
  command_encoder->fill_buffer(command_encoder.self, buffer, 0, gfx::WHOLE_SIZE,
                               0);
  command_encoder->clear_color_image(
      command_encoder.self, image, {},
      to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                          .first_mip_level   = 0,
                                          .num_mip_levels    = 1,
                                          .first_array_layer = 0,
                                          .num_array_layers  = 1}}));
  command_encoder->end_debug_marker(command_encoder.self);
  command_encoder->end(command_encoder.self).unwrap();
  command_encoder->reset(command_encoder.self);

  gfx::Shader compute_shader =
      device
          ->create_shader(
              device.self,
              gfx::ShaderDesc{
                  .label      = "SHADER_0",
                  .spirv_code = to_span(
                      {0x07230203U, 0x00010000U, 0x0008000bU, 0x00000006U,
                       0x00000000U, 0x00020011U, 0x00000001U, 0x0006000bU,
                       0x00000001U, 0x4c534c47U, 0x6474732eU, 0x3035342eU,
                       0x00000000U, 0x0003000eU, 0x00000000U, 0x00000001U,
                       0x0005000fU, 0x00000005U, 0x00000004U, 0x6e69616dU,
                       0x00000000U, 0x00060010U, 0x00000004U, 0x00000011U,
                       0x00000001U, 0x00000001U, 0x00000001U, 0x00030003U,
                       0x00000002U, 0x000001c2U, 0x00040005U, 0x00000004U,
                       0x6e69616dU, 0x00000000U, 0x00020013U, 0x00000002U,
                       0x00030021U, 0x00000003U, 0x00000002U, 0x00050036U,
                       0x00000002U, 0x00000004U, 0x00000000U, 0x00000003U,
                       0x000200f8U, 0x00000005U, 0x000100fdU, 0x00010038U})})
          .unwrap();

  gfx::PipelineCache cache =
      device
          ->create_pipeline_cache(
              device.self, gfx::PipelineCacheDesc{.label = "PIPELINE_CACHE_0",
                                                  .initial_data = {}})
          .unwrap();

  gfx::ComputePipeline compute_pipeline =
      device
          ->create_compute_pipeline(
              device.self,
              gfx::ComputePipelineDesc{
                  .label = "COMPUTE_PIPELINE_0",
                  .compute_shader =
                      gfx::ShaderStageDesc{.shader      = compute_shader,
                                           .entry_point = "main",
                                           .specialization_constants_data = {},
                                           .specialization_constants      = {}},
                  .push_constant_size     = 128,
                  .descriptor_set_layouts = to_span({set_layout}),
                  .cache                  = cache})
          .unwrap();

  logger->info("Num Pools: ", stats.num_pools,
               ", Num allocated groups: ", stats.num_allocated_groups,
               ", Num free groups: ", stats.num_free_groups,
               ", Num released: ", stats.num_released_groups);

  device->unref_shader(device.self, compute_shader);
  device->unref_compute_pipeline(device.self, compute_pipeline);
  device->unref_pipeline_cache(device.self, cache);
  device->unref_frame_context(device.self, frame_ctx);
  device->unref_descriptor_heap(device.self, descriptor_heap);
  device->unref_descriptor_set_layout(device.self, set_layout);
  device->unref_framebuffer(device.self, framebuffer);
  device->unref_render_pass(device.self, render_pass);
  device->unref_buffer_view(device.self, buffer_view);
  device->unref_image_view(device.self, view);
  device->unref_image(device.self, image);
  device->unref_buffer(device.self, buffer);
  instance->unref_device(instance.self, device.self);
  instance->unref(instance.self);

  logger->debug("Here");
  logger->trace("Here");
  logger->info("Here");
  logger->warn("Here");
  logger->error("Here");
  logger->fatal("Here");
  logger->info("Exiting");

  destroy_logger(logger);
}
