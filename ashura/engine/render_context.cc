#include "ashura/engine/render_context.h"

namespace ash
{

void RenderContext::init(gfx::DeviceImpl p_device, bool p_use_hdr,
                         u32 p_buffering, gfx::Extent p_initial_extent,
                         StrHashMap<gfx::Shader> p_shader_map)
{
  CHECK(p_buffering <= gfx::MAX_FRAME_BUFFERING && p_buffering > 0);
  CHECK(p_initial_extent.x > 0 && p_initial_extent.y > 0);
  device = p_device;

  gfx::Format sel_color_format         = gfx::Format::Undefined;
  gfx::Format sel_depth_stencil_format = gfx::Format::Undefined;

  if (p_use_hdr)
  {
    gfx::FormatProperties properties =
        device
            ->get_format_properties(device.self,
                                    gfx::Format::R16G16B16A16_SFLOAT)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
    {
      sel_color_format = gfx::Format::R16G16B16A16_SFLOAT;
    }
    else
    {
      default_logger->warn("HDR mode requested but Device does not support "
                           "HDR render target, trying UNORM color");
    }
  }

  if (sel_color_format == gfx::Format::Undefined)
  {
    gfx::FormatProperties properties =
        device->get_format_properties(device.self, gfx::Format::B8G8R8A8_UNORM)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
    {
      sel_color_format = gfx::Format::B8G8R8A8_UNORM;
    }
  }

  if (sel_color_format == gfx::Format::Undefined)
  {
    gfx::FormatProperties properties =
        device->get_format_properties(device.self, gfx::Format::R8G8B8A8_UNORM)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
    {
      sel_color_format = gfx::Format::R8G8B8A8_UNORM;
    }
  }

  {
    gfx::FormatProperties properties =
        device
            ->get_format_properties(device.self, gfx::Format::D16_UNORM_S8_UINT)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, DEPTH_STENCIL_FEATURES))
    {
      sel_depth_stencil_format = gfx::Format::D16_UNORM_S8_UINT;
    }
  }

  if (sel_depth_stencil_format == gfx::Format::Undefined)
  {
    gfx::FormatProperties properties =
        device
            ->get_format_properties(device.self, gfx::Format::D24_UNORM_S8_UINT)
            .unwrap();
    if (has_bits(properties.optimal_tiling_features, DEPTH_STENCIL_FEATURES))
    {
      sel_depth_stencil_format = gfx::Format::D24_UNORM_S8_UINT;
    }
  }

  CHECK_DESC(sel_color_format != gfx::Format::Undefined,
             "Device doesn't support any known color format");
  CHECK_DESC(sel_depth_stencil_format != gfx::Format::Undefined,
             "Device doesn't support any depth stencil format");

  pipeline_cache       = nullptr;
  buffering            = p_buffering;
  shader_map           = p_shader_map;
  color_format         = sel_color_format;
  depth_stencil_format = sel_depth_stencil_format;

  ssbo_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gfx::DescriptorSetLayoutDesc{
                  .label    = "SSBO"_span,
                  .bindings = to_span({gfx::DescriptorBindingDesc{
                      .type  = gfx::DescriptorType::DynamicStorageBuffer,
                      .count = 1,
                      .is_variable_length = false}})})
          .unwrap();

  textures_layout = device
                        ->create_descriptor_set_layout(
                            device.self,
                            gfx::DescriptorSetLayoutDesc{
                                .label    = "Texture Pack"_span,
                                .bindings = to_span({gfx::DescriptorBindingDesc{
                                    .type  = gfx::DescriptorType::SampledImage,
                                    .count = NUM_TEXTURE_SLOTS,
                                    .is_variable_length = true}})})
                        .unwrap();

  sampler_layout = device
                       ->create_descriptor_set_layout(
                           device.self,
                           gfx::DescriptorSetLayoutDesc{
                               .label    = "Sampler"_span,
                               .bindings = to_span({gfx::DescriptorBindingDesc{
                                   .type  = gfx::DescriptorType::Sampler,
                                   .count = 1,
                                   .is_variable_length = false}})})
                       .unwrap();

  texture_views = device
                      ->create_descriptor_set(device.self, textures_layout,
                                              to_span<u32>({NUM_TEXTURE_SLOTS}))
                      .unwrap();

  recreate_framebuffers(p_initial_extent);
}

static void recreate_framebuffer(RenderContext &ctx, Framebuffer &fb,
                                 gfx::Extent new_extent)
{
  ctx.release(fb);

  fb.color.desc = gfx::ImageDesc{
      .label  = "Framebuffer Color Image"_span,
      .type   = gfx::ImageType::Type2D,
      .format = ctx.color_format,
      .usage  = gfx::ImageUsage::ColorAttachment | gfx::ImageUsage::Sampled |
               gfx::ImageUsage::Storage | gfx::ImageUsage::TransferDst |
               gfx::ImageUsage::TransferSrc,
      .aspects      = gfx::ImageAspects::Color,
      .extent       = gfx::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gfx::SampleCount::Count1};

  fb.color.image =
      ctx.device->create_image(ctx.device.self, fb.color.desc).unwrap();

  fb.color.view_desc =
      gfx::ImageViewDesc{.label           = "Framebuffer Color Image View"_span,
                         .image           = fb.color.image,
                         .view_type       = gfx::ImageViewType::Type2D,
                         .view_format     = fb.color.desc.format,
                         .mapping         = {},
                         .aspects         = gfx::ImageAspects::Color,
                         .first_mip_level = 0,
                         .num_mip_levels  = 1,
                         .first_array_layer = 0,
                         .num_array_layers  = 1};
  fb.color.view =
      ctx.device->create_image_view(ctx.device.self, fb.color.view_desc)
          .unwrap();

  fb.color.texture =
      ctx.device
          ->create_descriptor_set(ctx.device.self, ctx.textures_layout,
                                  to_span<u32>({1}))
          .unwrap();

  ctx.device->update_descriptor_set(
      ctx.device.self,
      gfx::DescriptorSetUpdate{
          .set     = fb.color.texture,
          .binding = 0,
          .element = 0,
          .images = to_span({gfx::ImageBinding{.image_view = fb.color.view}})});

  fb.depth_stencil.desc = gfx::ImageDesc{
      .label  = "Framebuffer Depth Stencil Image"_span,
      .type   = gfx::ImageType::Type2D,
      .format = ctx.depth_stencil_format,
      .usage  = gfx::ImageUsage::DepthStencilAttachment |
               gfx::ImageUsage::Sampled | gfx::ImageUsage::TransferDst |
               gfx::ImageUsage::TransferSrc,
      .aspects      = gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
      .extent       = gfx::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gfx::SampleCount::Count1};

  fb.depth_stencil.image =
      ctx.device->create_image(ctx.device.self, fb.depth_stencil.desc).unwrap();

  fb.depth_stencil.view_desc = gfx::ImageViewDesc{
      .label           = "Framebuffer Depth Stencil Image View"_span,
      .image           = fb.depth_stencil.image,
      .view_type       = gfx::ImageViewType::Type2D,
      .view_format     = fb.depth_stencil.desc.format,
      .mapping         = {},
      .aspects         = gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
      .first_mip_level = 0,
      .num_mip_levels  = 1,
      .first_array_layer = 0,
      .num_array_layers  = 1};

  fb.depth_stencil.view =
      ctx.device->create_image_view(ctx.device.self, fb.depth_stencil.view_desc)
          .unwrap();

  fb.depth_stencil.texture =
      ctx.device
          ->create_descriptor_set(ctx.device.self, ctx.textures_layout,
                                  to_span<u32>({1}))
          .unwrap();

  ctx.device->update_descriptor_set(
      ctx.device.self,
      gfx::DescriptorSetUpdate{.set     = fb.depth_stencil.texture,
                               .binding = 0,
                               .element = 0,
                               .images  = to_span({gfx::ImageBinding{
                                    .image_view = fb.depth_stencil.view}})});

  fb.extent = new_extent;
}

void RenderContext::uninit()
{
  device->destroy_pipeline_cache(device.self, pipeline_cache);
  device->destroy_descriptor_set(device.self, texture_views);
  device->destroy_descriptor_set_layout(device.self, ssbo_layout);
  device->destroy_descriptor_set_layout(device.self, textures_layout);
  release(screen_fb);
  release(scratch_fb);
  idle_reclaim();
  for (u32 i = 0; i < buffering; i++)
  {
    released_objects[i].reset();
  }
  shader_map.for_each([&](Span<char const>, gfx::Shader shader) {
    device->destroy_shader(device.self, shader);
  });
  shader_map.reset();
}

void RenderContext::recreate_framebuffers(gfx::Extent new_extent)
{
  recreate_framebuffer(*this, screen_fb, new_extent);
  recreate_framebuffer(*this, scratch_fb, new_extent);
}

gfx::CommandEncoderImpl RenderContext::encoder()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.encoders[ctx.ring_index];
}

u32 RenderContext::ring_index()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.ring_index;
}

gfx::FrameId RenderContext::frame_id()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.current;
}

gfx::FrameId RenderContext::tail_frame_id()
{
  gfx::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.tail;
}

Option<gfx::Shader> RenderContext::get_shader(Span<char const> name)
{
  gfx::Shader *shader = shader_map[name];
  if (shader == nullptr)
  {
    return None;
  }
  return Some{*shader};
}

void RenderContext::release(gfx::Image image)
{
  if (image == nullptr)
  {
    return;
  }
  CHECK(released_objects[ring_index()].push(
      gfx::Object{.image = image, .type = gfx::ObjectType::Image}));
}

void RenderContext::release(gfx::ImageView view)
{
  if (view == nullptr)
  {
    return;
  }
  CHECK(released_objects[ring_index()].push(
      gfx::Object{.image_view = view, .type = gfx::ObjectType::ImageView}));
}

void RenderContext::release(gfx::Buffer buffer)
{
  if (buffer == nullptr)
  {
    return;
  }
  CHECK(released_objects[ring_index()].push(
      gfx::Object{.buffer = buffer, .type = gfx::ObjectType::Buffer}));
}

void RenderContext::release(gfx::BufferView view)
{
  if (view == nullptr)
  {
    return;
  }
  CHECK(released_objects[ring_index()].push(
      gfx::Object{.buffer_view = view, .type = gfx::ObjectType::BufferView}));
}

void RenderContext::release(gfx::DescriptorSet set)
{
  if (set == nullptr)
  {
    return;
  }
  CHECK(released_objects[ring_index()].push(gfx::Object{
      .descriptor_set = set, .type = gfx::ObjectType::DescriptorSet}));
}

void RenderContext::release(gfx::Sampler sampler)
{
  if (sampler == nullptr)
  {
    return;
  }
  CHECK(released_objects[ring_index()].push(
      gfx::Object{.sampler = sampler, .type = gfx::ObjectType::Sampler}));
}

static void destroy_objects(gfx::DeviceImpl const  &d,
                            Span<gfx::Object const> objects)
{
  for (u32 i = 0; i < (u32) objects.size(); i++)
  {
    gfx::Object obj = objects[i];
    switch (obj.type)
    {
      case gfx::ObjectType::Image:
        d->destroy_image(d.self, obj.image);
        break;
      case gfx::ObjectType::ImageView:
        d->destroy_image_view(d.self, obj.image_view);
        break;
      case gfx::ObjectType::Buffer:
        d->destroy_buffer(d.self, obj.buffer);
        break;
      case gfx::ObjectType::BufferView:
        d->destroy_buffer_view(d.self, obj.buffer_view);
        break;
      case gfx::ObjectType::Sampler:
        d->destroy_sampler(d.self, obj.sampler);
        break;
      case gfx::ObjectType::DescriptorSet:
        d->destroy_descriptor_set(d.self, obj.descriptor_set);
        break;
      default:
        UNREACHABLE();
    }
  }
}

void RenderContext::idle_reclaim()
{
  device->wait_idle(device.self).unwrap();
  for (u32 i = 0; i < buffering; i++)
  {
    destroy_objects(device, to_span(released_objects[i]));
  }
}

void RenderContext::begin_frame(gfx::Swapchain swapchain)
{
  device->begin_frame(device.self, swapchain).unwrap();
  destroy_objects(device, to_span(released_objects[ring_index()]));
  released_objects[ring_index()].clear();

  gfx::CommandEncoderImpl enc = encoder();

  enc->clear_color_image(
      enc.self, screen_fb.color.image, gfx::Color{.float32 = {0, 0, 0, 0}},
      to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                          .first_mip_level   = 0,
                                          .num_mip_levels    = 1,
                                          .first_array_layer = 0,
                                          .num_array_layers  = 1}}));

  enc->clear_color_image(
      enc.self, scratch_fb.color.image, gfx::Color{.float32 = {0, 0, 0, 0}},
      to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                          .first_mip_level   = 0,
                                          .num_mip_levels    = 1,
                                          .first_array_layer = 0,
                                          .num_array_layers  = 1}}));

  enc->clear_depth_stencil_image(
      enc.self, screen_fb.depth_stencil.image,
      gfx::DepthStencil{.depth = 0, .stencil = 0},
      to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Depth |
                                                     gfx::ImageAspects::Stencil,
                                          .first_mip_level   = 0,
                                          .num_mip_levels    = 1,
                                          .first_array_layer = 0,
                                          .num_array_layers  = 1}}));

  enc->clear_depth_stencil_image(
      enc.self, scratch_fb.depth_stencil.image,
      gfx::DepthStencil{.depth = 0, .stencil = 0},
      to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Depth |
                                                     gfx::ImageAspects::Stencil,
                                          .first_mip_level   = 0,
                                          .num_mip_levels    = 1,
                                          .first_array_layer = 0,
                                          .num_array_layers  = 1}}));
}

void RenderContext::end_frame(gfx::Swapchain swapchain)
{
  gfx::CommandEncoderImpl enc = encoder();
  if (swapchain != nullptr)
  {
    gfx::SwapchainState swapchain_state =
        device->get_swapchain_state(device.self, swapchain).unwrap();

    if (swapchain_state.current_image.is_some())
    {
      enc->blit_image(
          enc.self, screen_fb.color.image,
          swapchain_state.images[swapchain_state.current_image.unwrap()],
          to_span({gfx::ImageBlit{
              .src_layers  = {.aspects           = gfx::ImageAspects::Color,
                              .mip_level         = 0,
                              .first_array_layer = 0,
                              .num_array_layers  = 1},
              .src_offsets = {{0, 0, 0},
                              {screen_fb.extent.x, screen_fb.extent.y, 1}},
              .dst_layers  = {.aspects           = gfx::ImageAspects::Color,
                              .mip_level         = 0,
                              .first_array_layer = 0,
                              .num_array_layers  = 1},
              .dst_offsets = {{0, 0, 0},
                              {swapchain_state.extent.x,
                               swapchain_state.extent.y, 1}}}}),
          gfx::Filter::Linear);
    }
  }
  device->submit_frame(device.self, swapchain).unwrap();
}

}        // namespace ash