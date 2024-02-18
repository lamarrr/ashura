#include "ashura/engine/passes/pbr.h"

namespace ash
{

void PBRPass::init(Pass self_, RenderServer *server, uid32 id)
{
  PBRPass               *self   = (PBRPass *) self_;
  gfx::DeviceImpl const &device = server->device;
  self->descriptor_set_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gfx::DescriptorSetLayoutDesc{
                  .label    = "PBR Descriptor Layout",
                  .bindings = to_span<gfx::DescriptorBindingDesc>(
                      {{.type = gfx::DescriptorType::Sampler, .count = 1},
                       {.type = gfx::DescriptorType::SampledImage, .count = 6},
                       {.type  = gfx::DescriptorType::UniformBuffer,
                        .count = 1}})})
          .unwrap();

  self->descriptor_heap =
      device
          ->create_descriptor_heap(device.self,
                                   {&self->descriptor_set_layout, 1}, 256,
                                   default_allocator)
          .unwrap();

  self->sampler =
      device
          ->create_sampler(
              device.self,
              gfx::SamplerDesc{
                  .label             = "PBR Sampler",
                  .mag_filter        = gfx::Filter::Linear,
                  .min_filter        = gfx::Filter::Linear,
                  .mip_map_mode      = gfx::SamplerMipMapMode::Linear,
                  .address_mode_u    = gfx::SamplerAddressMode::ClampToEdge,
                  .address_mode_v    = gfx::SamplerAddressMode::ClampToEdge,
                  .address_mode_w    = gfx::SamplerAddressMode::ClampToEdge,
                  .mip_lod_bias      = 0,
                  .anisotropy_enable = false,
                  .max_anisotropy    = 0,
                  .compare_enable    = false,
                  .compare_op        = gfx::CompareOp::Never,
                  .min_lod           = 0,
                  .max_lod           = 1,
                  .border_color      = gfx::BorderColor::FloatTransparentBlack,
                  .unnormalized_coordinates = false})
          .unwrap();

  // get shader from server
  // get or create render pass
  // get or create frame buffer for each view on view added
  self->pipeline =
      device
          ->create_graphics_pipeline(
              device.self,
              gfx::GraphicsPipelineDesc{
                  .label = "PBR Graphics Pipeline",
                  .vertex_shader =
                      gfx::ShaderStageDesc{.shader      = nullptr,
                                           .entry_point = "vs_main",
                                           .specialization_constants_data = {},
                                           .specialization_constants      = {}},
                  .fragment_shader =
                      gfx::ShaderStageDesc{.shader      = nullptr,
                                           .entry_point = "fs_main",
                                           .specialization_constants_data = {},
                                           .specialization_constants      = {}},
                  .render_pass           = gfx::RenderPass{},
                  .vertex_input_bindings = to_span<gfx::VertexInputBinding>({}),
                  .vertex_attributes     = to_span<gfx::VertexAttribute>({}),
                  .push_constant_size    = gfx::MAX_PUSH_CONSTANT_SIZE,
                  .descriptor_set_layouts = {&self->descriptor_set_layout, 1},
                  .primitive_topology = gfx::PrimitiveTopology::TriangleList,
                  .rasterization_state =
                      gfx::PipelineRasterizationState{
                          .depth_clamp_enable = false,
                          .polygon_mode       = gfx::PolygonMode::Fill,
                          .cull_mode          = gfx::CullMode::None,
                          .front_face        = gfx::FrontFace::CounterClockWise,
                          .depth_bias_enable = false,
                          .depth_bias_constant_factor = 0,
                          .depth_bias_clamp           = 0,
                          .depth_bias_slope_factor    = 0},
                  .depth_stencil_state =
                      gfx::PipelineDepthStencilState{
                          .depth_test_enable        = true,
                          .depth_write_enable       = true,
                          .depth_compare_op         = gfx::CompareOp::Greater,
                          .depth_bounds_test_enable = false,
                          .stencil_test_enable      = false,
                          .front_stencil            = gfx::StencilOpState{},
                          .back_stencil             = gfx::StencilOpState{},
                          .min_depth_bounds         = 0,
                          .max_depth_bounds         = 0},
                  .color_blend_state =
                      gfx::PipelineColorBlendState{
                          .logic_op_enable = true,
                          .logic_op        = gfx::LogicOp::Set,
                          .attachments =
                              to_span<gfx::PipelineColorBlendAttachmentState>(
                                  {{.blend_enable = false,
                                    .src_color_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .dst_color_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .color_blend_op = gfx::BlendOp::Add,
                                    .src_alpha_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .dst_alpha_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .alpha_blend_op = gfx::BlendOp::Add,
                                    .color_write_mask =
                                        gfx::ColorComponents::All}}),
                          .blend_constant = {1, 1, 1, 1}},
                  .cache = server->pipeline_cache})
          .unwrap();
}

void PBRPass::begin(Pass self, RenderServer *server, uid32 view_id,
                    gfx::CommandEncoderImpl const *encoder)
{
  View              *view   = server->get_view(view_id).unwrap();
  gfx::DeviceImpl    device = server->device;
  gfx::SwapchainInfo swapchain_info =
      device->get_swapchain_info(device.self, server->swapchain).unwrap();
  ViewResource *color_resource = view->resources["COLOR_ATTACHMENT"_span];
  ViewResource *depth_stencil_resource =
      view->resources["DEPTH_STENCIL_ATTACHMENT"_span];

  // get_per_frame_image/per_frame_framebuffer?
  if (color_resource == nullptr)
  {
    gfx::Image image =
        device
            ->create_image(
                device.self,
                gfx::ImageDesc{.label  = nullptr,
                               .type   = gfx::ImageType::Type2D,
                               .format = gfx::Format::B8G8R8A8_UNORM,
                               .usage  = gfx::ImageUsage::ColorAttachment |
                                        gfx::ImageUsage::Sampled |
                                        gfx::ImageUsage::TransferSrc,
                               .aspects      = gfx::ImageAspects::Color,
                               .extent       = {view->viewport_extent.x,
                                                view->viewport_extent.y, 1},
                               .mip_levels   = 1,
                               .array_layers = 1})
            .unwrap();

    gfx::ImageView image_view =
        device
            ->create_image_view(
                device.self,
                gfx::ImageViewDesc{.label       = nullptr,
                                   .image       = nullptr,
                                   .view_type   = gfx::ImageViewType::Type2D,
                                   .view_format = gfx::Format::B8G8R8A8_UNORM,
                                   .mapping     = {},
                                   .aspects     = gfx::ImageAspects::Color,
                                   .first_mip_level   = 0,
                                   .num_mip_levels    = 1,
                                   .first_array_layer = 0,
                                   .num_array_layers  = 1})
            .unwrap();

    view->resources.emplace(
        "COLOR_ATTACHMENT"_span,
        ViewResource{.pass = 0, .resource = image, .tag = {}});
  }
  else
  {
    Attachment *color_attachment = (Attachment *) color_resource->resource;

    if (color_attachment->extent != view->viewport_extent)
    {
      // queue for deletion
      // create new image
      // create new image view
    }
  }

  if (depth_stencil_resource == nullptr)
  {
    device
        ->create_image(
            device.self,
            gfx::ImageDesc{
                .label  = nullptr,
                .type   = gfx::ImageType::Type2D,
                .format = gfx::Format::D16_UNORM_S8_UINT,
                .usage  = gfx::ImageUsage::DepthStencilAttachment |
                         gfx::ImageUsage::Sampled |
                         gfx::ImageUsage::TransferSrc,
                .aspects =
                    gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
                .extent = {view->viewport_extent.x, view->viewport_extent.y, 1},
                .mip_levels   = 1,
                .array_layers = 1})
        .unwrap();

    view->resources.emplace("DEPTH_STENCIL_ATTACHMENT"_span, ViewResource{});
  }
  else
  {
    Attachment *depth_stencil_attachment =
        (Attachment *) depth_stencil_resource->resource;

    if (depth_stencil_attachment->extent != view->viewport_extent)
    {
      // queue for deletion
      // create new image
      // create new image view
    }
  }

  device->create_framebuffer(device.self, {}).unwrap();

  // clear view

  // get_view_resource().or_else( create_and_attach_view_resource  )
  //
  // get_render_pass/get_framebuffer(
  gfx::RenderPassDesc{
      .label             = "PBR RenderPass",
      .color_attachments = to_span<gfx::RenderPassAttachment>(
          {{.format           = gfx::Format::B8G8R8A8_UNORM,
            .load_op          = gfx::LoadOp::Load,
            .store_op         = gfx::StoreOp::Store,
            .stencil_load_op  = gfx::LoadOp::Load,
            .stencil_store_op = gfx::StoreOp::DontCare}}),
      .depth_stencil_attachment =
          gfx::RenderPassAttachment{.format   = gfx::Format::D16_UNORM_S8_UINT,
                                    .load_op  = gfx::LoadOp::Load,
                                    .store_op = gfx::StoreOp::Store,
                                    .stencil_load_op  = gfx::LoadOp::Load,
                                    .stencil_store_op = gfx::StoreOp::Store},
      .input_attachments = {}};
  // )
}

void PBRPass::encode(Pass self_, RenderServer *server, uid32 view,
                     PassEncodeInfo const *info)
{
  PBRPass                *self = (PBRPass *) self_;
  gfx::RenderPass         render_pass;
  gfx::Framebuffer        framebuffer;
  gfx::Extent             extent;
  gfx::CommandEncoderImpl enc = info->command_encoder;

  enc->begin_render_pass(enc.self, framebuffer, render_pass, {0, 0}, extent,
                         to_span({gfx::Color{}}), gfx::DepthStencil{});
  enc->bind_graphics_pipeline(enc.self, self->pipeline);

  for (usize i = 0; i < info->indices.size(); i++)
  {
    usize batch_end = i;
    while (batch_end < info->indices.size())
    {
      // if materials not same
      batch_end++;
    }
    enc->draw(enc.self, 0, 0, 0, 0, 0);
  }

  enc->end_render_pass(enc.self);
}

}        // namespace ash