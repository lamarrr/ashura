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
}

void PBRPass::begin(Pass self, RenderServer *server, PassBeginInfo const *info)
{
  View              *view   = server->get_view(view_id).unwrap();
  gfx::DeviceImpl    device = server->device;
  gfx::SwapchainInfo swapchain_info =
      device->get_swapchain_info(device.self, server->swapchain).unwrap();
  Attachment **color_attachment =
      (Attachment **) view->resources["COLOR_ATTACHMENT"_span];
  Attachment **depth_attachment =
      (Attachment **) view->resources["DEPTH_STENCIL_ATTACHMENT"_span];
  bool use_hdr = view->resources["USE_HDR"_span] == nullptr;

  if (color_attachment == nullptr ||
      (color_attachment != nullptr &&
       ((*color_attachment)->extent != view->viewport_extent)))
  {
    if (color_attachment != nullptr)
    {
      //   color_attachment->image;        // queue for deletion
      //   color_attachment->view;         // queue for deletion
    }

    gfx::Format format = gfx::Format::B8G8R8A8_UNORM;

    if (use_hdr)
    {
      if (has_any_bit(device
                          ->get_format_properties(
                              device.self, gfx::Format::R32G32B32A32_SFLOAT)
                          .unwrap()
                          .optimal_tiling_features,
                      gfx::FormatFeatures::ColorAttachment |
                          gfx::FormatFeatures::ColorAttachmentBlend))
      {
        format = gfx::Format::R32G32B32A32_SFLOAT;
      }
      else
      {
        if (has_any_bit(device
                            ->get_format_properties(
                                device.self, gfx::Format::R32G32B32_SFLOAT)
                            .unwrap()
                            .optimal_tiling_features,
                        gfx::FormatFeatures::ColorAttachment |
                            gfx::FormatFeatures::ColorAttachmentBlend))
        {
          format = gfx::Format::R32G32B32_SFLOAT;
        }
      }
    }

    // TODO(lamarrr): double buffering
    gfx::Image image =
        device
            ->create_image(
                device.self,
                gfx::ImageDesc{.label  = nullptr,
                               .type   = gfx::ImageType::Type2D,
                               .format = format,
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
                                   .view_format = format,
                                   .mapping     = {},
                                   .aspects     = gfx::ImageAspects::Color,
                                   .first_mip_level   = 0,
                                   .num_mip_levels    = 1,
                                   .first_array_layer = 0,
                                   .num_array_layers  = 1})
            .unwrap();

    Attachment *attachment = server->allocator.allocate_typed<Attachment>(1);
    new (attachment) Attachment{.image  = image,
                                .view   = image_view,
                                .format = format,
                                .extent = view->viewport_extent};

    // deletion is incorrect
    view->resources.emplace("COLOR_ATTACHMENT"_span, attachment);
  }

  if (depth_attachment == nullptr ||
      (depth_attachment != nullptr &&
       (*depth_attachment)->extent != view->viewport_extent))
  {
    if (depth_attachment != nullptr)
    {
      //   depth_attachment->image;        // queue for deletion
      //   depth_attachment->view;         // queue for deletion
    }

    gfx::Image image =
        device
            ->create_image(
                device.self,
                gfx::ImageDesc{.label  = nullptr,
                               .type   = gfx::ImageType::Type2D,
                               .format = gfx::Format::D16_UNORM_S8_UINT,
                               .usage =
                                   gfx::ImageUsage::DepthStencilAttachment |
                                   gfx::ImageUsage::Sampled |
                                   gfx::ImageUsage::TransferSrc,
                               .aspects = gfx::ImageAspects::Depth |
                                          gfx::ImageAspects::Stencil,
                               .extent       = {view->viewport_extent.x,
                                                view->viewport_extent.y, 1},
                               .mip_levels   = 1,
                               .array_layers = 1})
            .unwrap();

    gfx::ImageView image_view =
        device
            ->create_image_view(
                device.self,
                gfx::ImageViewDesc{.label     = nullptr,
                                   .image     = nullptr,
                                   .view_type = gfx::ImageViewType::Type2D,
                                   .view_format =
                                       gfx::Format::D16_UNORM_S8_UINT,
                                   .mapping = {},
                                   .aspects = gfx::ImageAspects::Depth |
                                              gfx::ImageAspects::Stencil,
                                   .first_mip_level   = 0,
                                   .num_mip_levels    = 1,
                                   .first_array_layer = 0,
                                   .num_array_layers  = 1})
            .unwrap();

    Attachment *attachment = server->allocator.allocate_typed<Attachment>(1);
    new (attachment) Attachment{.image  = image,
                                .view   = image_view,
                                .format = gfx::Format::D16_UNORM_S8_UINT,
                                .extent = view->viewport_extent};

    view->resources.emplace("DEPTH_STENCIL_ATTACHMENT"_span, attachment);
  }

  // clear view
  // TODO(lamarrr): who clears the attachments
  // SYNCING RENDERPASS AND images, and framebuffers
  //
  (*encoder)->clear_color_image(encoder->self, nullptr, {}, {});
  (*encoder)->clear_depth_stencil_image(encoder->self, nullptr, {}, {});

  self->pipeline =
      device
          ->create_graphics_pipeline(
              device.self,
              gfx::GraphicsPipelineDesc{
                  .label = "PBR Graphics Pipeline",
                  .vertex_shader =
                      gfx::ShaderStageDesc{
                          .shader = server->get_shader("PBR_VERTEX_SHADER"_span)
                                        .unwrap(),
                          .entry_point                   = "vs_main",
                          .specialization_constants_data = {},
                          .specialization_constants      = {}},
                  .fragment_shader =
                      gfx::ShaderStageDesc{
                          .shader =
                              server->get_shader("PBR_FRAGMENT_SHADER"_span)
                                  .unwrap(),
                          .entry_point                   = "fs_main",
                          .specialization_constants_data = {},
                          .specialization_constants      = {}},
                  .render_pass           = gfx::RenderPass{},
                  .vertex_input_bindings = to_span<gfx::VertexInputBinding>(
                      {{.binding    = 0,
                        .stride     = sizeof(PBRVertex),
                        .input_rate = gfx::InputRate::Vertex}}),
                  .vertex_attributes = to_span<gfx::VertexAttribute>(
                      {{.binding  = 0,
                        .location = 0,
                        .format   = gfx::Format::R32G32B32_SFLOAT,
                        .offset   = offsetof(PBRVertex, x)},
                       {.binding  = 0,
                        .location = 1,
                        .format   = gfx::Format::R32G32_SFLOAT,
                        .offset   = offsetof(PBRVertex, u)}}),
                  .push_constant_size     = gfx::MAX_PUSH_CONSTANT_SIZE,
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

  gfx::RenderPass render_pass =
      server
          ->get_render_pass(gfx::RenderPassDesc{
              .label             = "PBR RenderPass",
              .color_attachments = to_span<gfx::RenderPassAttachment>(
                  {{.format           = (*color_attachment)->format,
                    .load_op          = gfx::LoadOp::Load,
                    .store_op         = gfx::StoreOp::Store,
                    .stencil_load_op  = gfx::LoadOp::DontCare,
                    .stencil_store_op = gfx::StoreOp::DontCare}}),
              .depth_stencil_attachment =
                  gfx::RenderPassAttachment{
                      .format           = (*depth_stencil_attachment)->format,
                      .load_op          = gfx::LoadOp::Load,
                      .store_op         = gfx::StoreOp::Store,
                      .stencil_load_op  = gfx::LoadOp::Load,
                      .stencil_store_op = gfx::StoreOp::Store},
              .input_attachments = {}})
          .unwrap();

  gfx::Framebuffer framebuffer =
      device
          ->create_framebuffer(
              device.self,
              gfx::FramebufferDesc{
                  .label                    = "PBR Framebuffer",
                  .render_pass              = render_pass,
                  .extent                   = (*color_attachment)->extent,
                  .color_attachments        = {&(*color_attachment)->view, 1},
                  .depth_stencil_attachment = (*depth_stencil_attachment)->view,
                  .layers                   = 1})
          .unwrap();
}

void PBRPass::encode(Pass self_, RenderServer *server,
                     PassEncodeInfo const *info)
{
  PBRPass                *self   = (PBRPass *) self_;
  gfx::DeviceImpl         device = server->device;
  View                   *view   = server->get_view(view_id).unwrap();
  gfx::CommandEncoderImpl enc    = info->command_encoder;
  Attachment            **color_attachment =
      (Attachment **) view->resources["COLOR_ATTACHMENT"_span];
  Attachment **depth_stencil_attachment =
      (Attachment **) view->resources["DEPTH_STENCIL_ATTACHMENT"_span];

  enc->begin_render_pass(enc.self, framebuffer, render_pass, {0, 0},
                         (*color_attachment)->extent, to_span({gfx::Color{}}),
                         gfx::DepthStencil{});
  enc->bind_graphics_pipeline(enc.self, self->pipeline);

  for (usize i = 0; i < info->indices.size(); i++)
  {
    PBRMesh   mesh;
    PBRObject object;
    enc->bind_vertex_buffers(enc.self, {&mesh.vertex_buffer, 1},
                             {&mesh.vertex_buffer_offset, 0});
    enc->bind_index_buffer(enc.self, mesh.index_buffer, mesh.first_index,
                           mesh.index_type);
    enc->bind_descriptor_sets(enc.self, {&self->descriptor_heap.self, 1},
                              {&object.descriptor_heap_group, 1},
                              to_span<u32>({0, 1, 2, 3}), {});
    enc->draw(enc.self, mesh.first_index, mesh.num_indices, 0, 0, 1);
  }

  enc->end_render_pass(enc.self);
}

}        // namespace ash