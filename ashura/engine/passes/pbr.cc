#include "ashura/engine/passes/pbr.h"

namespace ash
{

struct PBRBinding
{
  gfx::RenderPass       render_pass = nullptr;
  gfx::Framebuffer      framebuffer = nullptr;
  gfx::GraphicsPipeline pipeline    = nullptr;
};

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
}

void PBRPass::begin(Pass self_, RenderServer *server, PassBeginInfo const *info)
{
  gfx::DeviceImpl device = server->device;
  View           *view   = server->get_view(info->view).unwrap();
  PBRPass        *self   = (PBRPass *) self_;

  // clear view
  // TODO(lamarrr): when and who clears the attachments
  // SYNCING RENDERPASS AND images, and framebuffers
  //
  //   (*encoder)->clear_color_image(encoder->self, nullptr, {}, {});
  //   (*encoder)->clear_depth_stencil_image(encoder->self, nullptr, {}, {});

  PBRBinding *binding = server->allocator.allocate_typed<PBRBinding>(1);
  // TODO: check allocation
  Attachment color_attachment = info->attachments->color_attachment.value();
  Attachment depth_stencil_attachment =
      info->attachments->depth_stencil_attachment.value();

  binding->render_pass =
      server
          ->get_render_pass(gfx::RenderPassDesc{
              .label             = "PBR RenderPass",
              .color_attachments = to_span<gfx::RenderPassAttachment>(
                  {{.format           = color_attachment.desc.format,
                    .load_op          = gfx::LoadOp::Load,
                    .store_op         = gfx::StoreOp::Store,
                    .stencil_load_op  = gfx::LoadOp::DontCare,
                    .stencil_store_op = gfx::StoreOp::DontCare}}),
              .depth_stencil_attachment =
                  gfx::RenderPassAttachment{
                      .format           = depth_stencil_attachment.desc.format,
                      .load_op          = gfx::LoadOp::Load,
                      .store_op         = gfx::StoreOp::Store,
                      .stencil_load_op  = gfx::LoadOp::Load,
                      .stencil_store_op = gfx::StoreOp::Store},
              .input_attachments = {}})
          .unwrap();

  gfx::Shader vertex_shader =
      server->get_shader("PBR_VERTEX_SHADER"_span).unwrap();
  gfx::Shader fragment_shader =
      server->get_shader("PBR_FRAGMENT_SHADER"_span).unwrap();

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label         = "PBR Graphics Pipeline",
      .vertex_shader = gfx::ShaderStageDesc{.shader      = vertex_shader,
                                            .entry_point = "vs_main",
                                            .specialization_constants_data = {},
                                            .specialization_constants = {}},
      .fragment_shader =
          gfx::ShaderStageDesc{.shader                        = fragment_shader,
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
      .primitive_topology     = gfx::PrimitiveTopology::TriangleList,
      .rasterization_state =
          gfx::PipelineRasterizationState{
              .depth_clamp_enable         = false,
              .polygon_mode               = gfx::PolygonMode::Fill,
              .cull_mode                  = gfx::CullMode::None,
              .front_face                 = gfx::FrontFace::CounterClockWise,
              .depth_bias_enable          = false,
              .depth_bias_constant_factor = 0,
              .depth_bias_clamp           = 0,
              .depth_bias_slope_factor    = 0},
      .depth_stencil_state =
          gfx::PipelineDepthStencilState{.depth_test_enable  = true,
                                         .depth_write_enable = true,
                                         .depth_compare_op =
                                             gfx::CompareOp::Greater,
                                         .depth_bounds_test_enable = false,
                                         .stencil_test_enable      = false,
                                         .front_stencil = gfx::StencilOpState{},
                                         .back_stencil  = gfx::StencilOpState{},
                                         .min_depth_bounds = 0,
                                         .max_depth_bounds = 0},
      .color_blend_state =
          gfx::PipelineColorBlendState{
              .logic_op_enable = true,
              .logic_op        = gfx::LogicOp::Set,
              .attachments = to_span<gfx::PipelineColorBlendAttachmentState>(
                  {{.blend_enable           = false,
                    .src_color_blend_factor = gfx::BlendFactor::Zero,
                    .dst_color_blend_factor = gfx::BlendFactor::Zero,
                    .color_blend_op         = gfx::BlendOp::Add,
                    .src_alpha_blend_factor = gfx::BlendFactor::Zero,
                    .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
                    .alpha_blend_op         = gfx::BlendOp::Add,
                    .color_write_mask       = gfx::ColorComponents::All}}),
              .blend_constant = {1, 1, 1, 1}},
      .cache = server->pipeline_cache};

  binding->pipeline =
      device->create_graphics_pipeline(device.self, pipeline_desc).unwrap();

  gfx::Framebuffer framebuffer =
      device
          ->create_framebuffer(
              device.self,
              gfx::FramebufferDesc{
                  .label                    = "PBR Framebuffer",
                  .render_pass              = binding->render_pass,
                  .extent                   = view->config.extent,
                  .color_attachments        = {&color_attachment.view, 1},
                  .depth_stencil_attachment = depth_stencil_attachment.view,
                  .layers                   = 1})
          .unwrap();

  *info->binding = (PassBinding) binding;
}

void PBRPass::end(Pass self_, RenderServer *server, PassEndInfo const *info)
{
  gfx::DeviceImpl device  = server->device;
  PBRBinding     *binding = (PBRBinding *) info->binding;
  // TODO(lamarrr): use cache instead, and never delete immediately, add
  // deletion queue
  device->unref_render_pass(device.self, binding->render_pass);
  device->unref_graphics_pipeline(device.self, binding->pipeline);
  device->unref_framebuffer(device.self, binding->framebuffer);
}

void PBRPass::encode(Pass self_, RenderServer *server,
                     PassEncodeInfo const *info)
{
  PBRPass                *self    = (PBRPass *) self_;
  gfx::DeviceImpl         device  = server->device;
  View                   *view    = server->get_view(info->view).unwrap();
  gfx::CommandEncoderImpl enc     = info->encoder;
  PBRBinding             *binding = (PBRBinding *) info->binding;

  enc->begin_render_pass(enc.self, binding->framebuffer, binding->render_pass,
                         {0, 0}, view->config.extent, to_span({gfx::Color{}}),
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