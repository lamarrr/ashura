/// SPDX-License-Identifier: MIT
#include "ashura/engine/gpu_context.h"

namespace ash
{

GpuContext GpuContext::create(AllocatorImpl allocator, gpu::DeviceImpl device,
                              bool use_hdr, u32 buffering,
                              gpu::Extent initial_extent)
{
  CHECK(buffering <= gpu::MAX_FRAME_BUFFERING);
  CHECK(initial_extent.x > 0 && initial_extent.y > 0);

  u32 sel_hdr_color_format     = 0;
  u32 sel_sdr_color_format     = 0;
  u32 sel_depth_stencil_format = 0;

  if (use_hdr)
  {
    for (; sel_hdr_color_format < size(HDR_COLOR_FORMATS);
         sel_hdr_color_format++)
    {
      gpu::FormatProperties props =
          device
              ->get_format_properties(device.self,
                                      HDR_COLOR_FORMATS[sel_hdr_color_format])
              .unwrap();
      if (has_bits(props.optimal_tiling_features, COLOR_FEATURES))
      {
        break;
      }
    }

    if (sel_hdr_color_format >= size(HDR_COLOR_FORMATS))
    {
      logger->warn("HDR mode requested but Device does not support "
                   "HDR render target, trying UNORM color");
    }
  }

  if (!use_hdr || sel_hdr_color_format >= size(HDR_COLOR_FORMATS))
  {
    for (; sel_sdr_color_format < size(SDR_COLOR_FORMATS);
         sel_sdr_color_format++)
    {
      gpu::FormatProperties props =
          device
              ->get_format_properties(device.self,
                                      SDR_COLOR_FORMATS[sel_sdr_color_format])
              .unwrap();
      if (has_bits(props.optimal_tiling_features, COLOR_FEATURES))
      {
        break;
      }
    }
  }

  for (; sel_depth_stencil_format < size(DEPTH_STENCIL_FORMATS);
       sel_depth_stencil_format++)
  {
    gpu::FormatProperties props =
        device
            ->get_format_properties(
                device.self, DEPTH_STENCIL_FORMATS[sel_depth_stencil_format])
            .unwrap();
    if (has_bits(props.optimal_tiling_features, DEPTH_STENCIL_FEATURES))
    {
      break;
    }
  }

  gpu::Format color_format = gpu::Format::Undefined;

  if (use_hdr)
  {
    CHECK_DESC(sel_sdr_color_format != size(SDR_COLOR_FORMATS) ||
                   sel_hdr_color_format != size(HDR_COLOR_FORMATS),
               "Device doesn't support any known color format");
    if (sel_hdr_color_format != size(HDR_COLOR_FORMATS))
    {
      color_format = HDR_COLOR_FORMATS[sel_hdr_color_format];
    }
    else
    {
      color_format = SDR_COLOR_FORMATS[sel_sdr_color_format];
    }
  }
  else
  {
    CHECK_DESC(sel_sdr_color_format != size(SDR_COLOR_FORMATS),
               "Device doesn't support any known color format");
    color_format = SDR_COLOR_FORMATS[sel_sdr_color_format];
  }

  CHECK_DESC(sel_depth_stencil_format != size(DEPTH_STENCIL_FORMATS),
             "Device doesn't support any known depth stencil format");
  gpu::Format depth_stencil_format =
      DEPTH_STENCIL_FORMATS[sel_depth_stencil_format];

  logger->trace("Selected color format: ", color_format);

  logger->trace("Selected depth stencil format: ", depth_stencil_format);

  gpu::PipelineCache pipeline_cache = nullptr;

  gpu::DescriptorSetLayout ubo_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gpu::DescriptorSetLayoutInfo{
                  .label    = "UBO Layout"_span,
                  .bindings = span({gpu::DescriptorBindingInfo{
                      .type  = gpu::DescriptorType::DynamicUniformBuffer,
                      .count = 1,
                      .is_variable_length = false}})})
          .unwrap();

  gpu::DescriptorSetLayout ssbo_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gpu::DescriptorSetLayoutInfo{
                  .label    = "SSBO Layout"_span,
                  .bindings = span({gpu::DescriptorBindingInfo{
                      .type  = gpu::DescriptorType::DynamicStorageBuffer,
                      .count = 1,
                      .is_variable_length = false}})})
          .unwrap();

  gpu::DescriptorSetLayout textures_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gpu::DescriptorSetLayoutInfo{
                  .label    = "Textures Layout"_span,
                  .bindings = span({gpu::DescriptorBindingInfo{
                      .type               = gpu::DescriptorType::SampledImage,
                      .count              = NUM_TEXTURE_SLOTS,
                      .is_variable_length = true}})})
          .unwrap();

  gpu::DescriptorSetLayout samplers_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gpu::DescriptorSetLayoutInfo{
                  .label    = "Samplers Layout"_span,
                  .bindings = span({gpu::DescriptorBindingInfo{
                      .type               = gpu::DescriptorType::Sampler,
                      .count              = NUM_SAMPLER_SLOTS,
                      .is_variable_length = true}})})
          .unwrap();

  gpu::DescriptorSet texture_views =
      device
          ->create_descriptor_set(device.self, textures_layout,
                                  span<u32>({NUM_TEXTURE_SLOTS}))
          .unwrap();

  gpu::DescriptorSet samplers =
      device
          ->create_descriptor_set(device.self, samplers_layout,
                                  span<u32>({NUM_SAMPLER_SLOTS}))
          .unwrap();

  gpu::Image default_image =
      device
          ->create_image(
              device.self,
              gpu::ImageInfo{.label  = "Default Texture Image"_span,
                             .type   = gpu::ImageType::Type2D,
                             .format = gpu::Format::B8G8R8A8_UNORM,
                             .usage  = gpu::ImageUsage::Sampled |
                                      gpu::ImageUsage::TransferDst |
                                      gpu::ImageUsage::Storage |
                                      gpu::ImageUsage::Storage,
                             .aspects      = gpu::ImageAspects::Color,
                             .extent       = {1, 1, 1},
                             .mip_levels   = 1,
                             .array_layers = 1,
                             .sample_count = gpu::SampleCount::Count1})
          .unwrap();

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views;

  {
    gpu::ComponentMapping mappings[NUM_DEFAULT_TEXTURES] = {};
    mappings[TEXTURE_WHITE]       = {.r = gpu::ComponentSwizzle::One,
                                     .g = gpu::ComponentSwizzle::One,
                                     .b = gpu::ComponentSwizzle::One,
                                     .a = gpu::ComponentSwizzle::One};
    mappings[TEXTURE_BLACK]       = {.r = gpu::ComponentSwizzle::Zero,
                                     .g = gpu::ComponentSwizzle::Zero,
                                     .b = gpu::ComponentSwizzle::Zero,
                                     .a = gpu::ComponentSwizzle::One};
    mappings[TEXTURE_TRANSPARENT] = {.r = gpu::ComponentSwizzle::Zero,
                                     .g = gpu::ComponentSwizzle::Zero,
                                     .b = gpu::ComponentSwizzle::Zero,
                                     .a = gpu::ComponentSwizzle::Zero};
    mappings[TEXTURE_RED]         = {.r = gpu::ComponentSwizzle::One,
                                     .g = gpu::ComponentSwizzle::Zero,
                                     .b = gpu::ComponentSwizzle::Zero,
                                     .a = gpu::ComponentSwizzle::One};
    mappings[TEXTURE_GREEN]       = {.r = gpu::ComponentSwizzle::Zero,
                                     .g = gpu::ComponentSwizzle::One,
                                     .b = gpu::ComponentSwizzle::Zero,
                                     .a = gpu::ComponentSwizzle::One};
    mappings[TEXTURE_BLUE]        = {.r = gpu::ComponentSwizzle::Zero,
                                     .g = gpu::ComponentSwizzle::Zero,
                                     .b = gpu::ComponentSwizzle::One,
                                     .a = gpu::ComponentSwizzle::One};

    for (u32 i = 0; i < NUM_DEFAULT_TEXTURES; i++)
    {
      default_image_views[i] =
          device
              ->create_image_view(
                  device.self,
                  gpu::ImageViewInfo{.label = "Default Texture Image View"_span,
                                     .image = default_image,
                                     .view_type   = gpu::ImageViewType::Type2D,
                                     .view_format = gpu::Format::B8G8R8A8_UNORM,
                                     .mapping     = mappings[i],
                                     .aspects     = gpu::ImageAspects::Color,
                                     .first_mip_level   = 0,
                                     .num_mip_levels    = 1,
                                     .first_array_layer = 0,
                                     .num_array_layers  = 1})
              .unwrap();

      device->update_descriptor_set(
          device.self, gpu::DescriptorSetUpdate{
                           .set     = texture_views,
                           .binding = 0,
                           .element = i,
                           .images  = span({gpu::ImageBinding{
                                .image_view = default_image_views[i]}})});
    }
  }

  InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects;

  for (u32 i = 0; i < buffering; i++)
  {
    released_objects.push(Vec<gpu::Object>{allocator}).unwrap();
  }

  GpuContext ctx{allocator,
                 device,
                 pipeline_cache,
                 buffering,
                 color_format,
                 depth_stencil_format,
                 ubo_layout,
                 ssbo_layout,
                 textures_layout,
                 samplers_layout,
                 texture_views,
                 samplers,
                 default_image,
                 default_image_views,
                 std::move(released_objects)};

  for (u32 i = 0; i < NUM_DEFAULT_TEXTURES; i++)
  {
    u32 slot = ctx.alloc_texture_slot();
    CHECK(slot == i);
  }

  CachedSampler sampler = ctx.create_sampler(
      gpu::SamplerInfo{.label             = "Linear+Repeat Sampler"_span,
                       .mag_filter        = gpu::Filter::Linear,
                       .min_filter        = gpu::Filter::Linear,
                       .mip_map_mode      = gpu::SamplerMipMapMode::Linear,
                       .address_mode_u    = gpu::SamplerAddressMode::Repeat,
                       .address_mode_v    = gpu::SamplerAddressMode::Repeat,
                       .address_mode_w    = gpu::SamplerAddressMode::Repeat,
                       .mip_lod_bias      = 0,
                       .anisotropy_enable = false,
                       .max_anisotropy    = 1.0,
                       .compare_enable    = false,
                       .compare_op        = gpu::CompareOp::Never,
                       .min_lod           = 0,
                       .max_lod           = 0,
                       .border_color = gpu::BorderColor::FloatTransparentBlack,
                       .unnormalized_coordinates = false});

  CHECK(sampler.slot == SAMPLER_LINEAR);

  sampler = ctx.create_sampler(
      gpu::SamplerInfo{.label             = "Nearest+Repeat Sampler"_span,
                       .mag_filter        = gpu::Filter::Nearest,
                       .min_filter        = gpu::Filter::Nearest,
                       .mip_map_mode      = gpu::SamplerMipMapMode::Nearest,
                       .address_mode_u    = gpu::SamplerAddressMode::Repeat,
                       .address_mode_v    = gpu::SamplerAddressMode::Repeat,
                       .address_mode_w    = gpu::SamplerAddressMode::Repeat,
                       .mip_lod_bias      = 0,
                       .anisotropy_enable = false,
                       .max_anisotropy    = 1.0,
                       .compare_enable    = false,
                       .compare_op        = gpu::CompareOp::Never,
                       .min_lod           = 0,
                       .max_lod           = 0,
                       .border_color = gpu::BorderColor::FloatTransparentBlack,
                       .unnormalized_coordinates = false});

  CHECK(sampler.slot == SAMPLER_NEAREST);

  sampler = ctx.create_sampler(
      gpu::SamplerInfo{.label          = "Linear+EdgeClamped Sampler"_span,
                       .mag_filter     = gpu::Filter::Linear,
                       .min_filter     = gpu::Filter::Linear,
                       .mip_map_mode   = gpu::SamplerMipMapMode::Linear,
                       .address_mode_u = gpu::SamplerAddressMode::ClampToEdge,
                       .address_mode_v = gpu::SamplerAddressMode::ClampToEdge,
                       .address_mode_w = gpu::SamplerAddressMode::ClampToEdge,
                       .mip_lod_bias   = 0,
                       .anisotropy_enable = false,
                       .max_anisotropy    = 1.0,
                       .compare_enable    = false,
                       .compare_op        = gpu::CompareOp::Never,
                       .min_lod           = 0,
                       .max_lod           = 0,
                       .border_color = gpu::BorderColor::FloatTransparentBlack,
                       .unnormalized_coordinates = false});

  CHECK(sampler.slot == SAMPLER_LINEAR_CLAMPED);

  sampler = ctx.create_sampler(
      gpu::SamplerInfo{.label          = "Nearest+EdgeClamped Sampler"_span,
                       .mag_filter     = gpu::Filter::Nearest,
                       .min_filter     = gpu::Filter::Nearest,
                       .mip_map_mode   = gpu::SamplerMipMapMode::Nearest,
                       .address_mode_u = gpu::SamplerAddressMode::ClampToEdge,
                       .address_mode_v = gpu::SamplerAddressMode::ClampToEdge,
                       .address_mode_w = gpu::SamplerAddressMode::ClampToEdge,
                       .mip_lod_bias   = 0,
                       .anisotropy_enable = false,
                       .max_anisotropy    = 1.0,
                       .compare_enable    = false,
                       .compare_op        = gpu::CompareOp::Never,
                       .min_lod           = 0,
                       .max_lod           = 0,
                       .border_color = gpu::BorderColor::FloatTransparentBlack,
                       .unnormalized_coordinates = false});

  CHECK(sampler.slot == SAMPLER_NEAREST_CLAMPED);

  ctx.recreate_framebuffers(initial_extent);

  return ctx;
}

static void recreate_framebuffer(GpuContext &ctx, Framebuffer &fb,
                                 gpu::Extent new_extent)
{
  ctx.release(fb);

  fb.color.info = gpu::ImageInfo{
      .label  = "Framebuffer Color Image"_span,
      .type   = gpu::ImageType::Type2D,
      .format = ctx.color_format,
      .usage  = gpu::ImageUsage::ColorAttachment | gpu::ImageUsage::Sampled |
               gpu::ImageUsage::Storage | gpu::ImageUsage::TransferDst |
               gpu::ImageUsage::TransferSrc,
      .aspects      = gpu::ImageAspects::Color,
      .extent       = gpu::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gpu::SampleCount::Count1};

  fb.color.image =
      ctx.device->create_image(ctx.device.self, fb.color.info).unwrap();

  fb.color.view_info =
      gpu::ImageViewInfo{.label           = "Framebuffer Color Image View"_span,
                         .image           = fb.color.image,
                         .view_type       = gpu::ImageViewType::Type2D,
                         .view_format     = fb.color.info.format,
                         .mapping         = {},
                         .aspects         = gpu::ImageAspects::Color,
                         .first_mip_level = 0,
                         .num_mip_levels  = 1,
                         .first_array_layer = 0,
                         .num_array_layers  = 1};
  fb.color.view =
      ctx.device->create_image_view(ctx.device.self, fb.color.view_info)
          .unwrap();

  fb.depth_stencil.info = gpu::ImageInfo{
      .label  = "Framebuffer Depth Stencil Image"_span,
      .type   = gpu::ImageType::Type2D,
      .format = ctx.depth_stencil_format,
      .usage  = gpu::ImageUsage::DepthStencilAttachment |
               gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
               gpu::ImageUsage::TransferSrc,
      .aspects      = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
      .extent       = gpu::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gpu::SampleCount::Count1};

  fb.depth_stencil.image =
      ctx.device->create_image(ctx.device.self, fb.depth_stencil.info).unwrap();

  fb.depth_stencil.view_info = gpu::ImageViewInfo{
      .label           = "Framebuffer Depth Stencil Image View"_span,
      .image           = fb.depth_stencil.image,
      .view_type       = gpu::ImageViewType::Type2D,
      .view_format     = fb.depth_stencil.info.format,
      .mapping         = {},
      .aspects         = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
      .first_mip_level = 0,
      .num_mip_levels  = 1,
      .first_array_layer = 0,
      .num_array_layers  = 1};

  fb.depth_stencil.view =
      ctx.device->create_image_view(ctx.device.self, fb.depth_stencil.view_info)
          .unwrap();

  fb.color_texture =
      ctx.device
          ->create_descriptor_set(ctx.device.self, ctx.textures_layout,
                                  span<u32>({1}))
          .unwrap();

  ctx.device->update_descriptor_set(
      ctx.device.self,
      gpu::DescriptorSetUpdate{
          .set     = fb.color_texture,
          .binding = 0,
          .element = 0,
          .images  = span({gpu::ImageBinding{.image_view = fb.color.view}})});

  fb.extent = new_extent;
}

void GpuContext::recreate_framebuffers(gpu::Extent new_extent)
{
  recreate_framebuffer(*this, screen_fb, new_extent);
  for (Framebuffer &f : scratch_fbs)
  {
    recreate_framebuffer(*this, f, new_extent);
  }
}

gpu::CommandEncoderImpl GpuContext::encoder()
{
  gpu::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.encoders[ctx.ring_index];
}

u32 GpuContext::ring_index()
{
  gpu::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.ring_index;
}

gpu::FrameId GpuContext::frame_id()
{
  gpu::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.current;
}

gpu::FrameId GpuContext::tail_frame_id()
{
  gpu::FrameContext ctx = device->get_frame_context(device.self);
  return ctx.tail;
}

CachedSampler GpuContext::create_sampler(gpu::SamplerInfo const &info)
{
  CachedSampler *cached = sampler_cache.try_get(info);
  if (cached != nullptr)
  {
    return *cached;
  }

  CachedSampler sampler{.sampler =
                            device->create_sampler(device.self, info).unwrap(),
                        .slot = alloc_sampler_slot()};

  device->update_descriptor_set(
      device.self,
      gpu::DescriptorSetUpdate{
          .set     = samplers,
          .binding = 0,
          .element = sampler.slot,
          .images  = span({gpu::ImageBinding{.sampler = sampler.sampler}})});

  bool exists;
  CHECK(sampler_cache.insert(exists, nullptr, info, sampler) && !exists);

  return sampler;
}

u32 GpuContext::alloc_texture_slot()
{
  usize i = find_clear_bit(span(texture_slots));
  CHECK_DESC(i < size_bits(texture_slots), "Out of Texture Slots");
  set_bit(span(texture_slots), i);
  return (u32) i;
}

void GpuContext::release_texture_slot(u32 slot)
{
  clear_bit(span(texture_slots), slot);
}

u32 GpuContext::alloc_sampler_slot()
{
  usize i = find_clear_bit(span(sampler_slots));
  CHECK_DESC(i < size_bits(sampler_slots), "Out of Sampler Slots");
  set_bit(span(sampler_slots), i);
  return (u32) i;
}

void GpuContext::release_sampler_slot(u32 slot)
{
  clear_bit(span(sampler_slots), slot);
}

void GpuContext::release(gpu::Image image)
{
  if (image == nullptr)
  {
    return;
  }
  released_objects[ring_index()]
      .push(gpu::Object{.image = image, .type = gpu::ObjectType::Image})
      .unwrap();
}

void GpuContext::release(gpu::ImageView view)
{
  if (view == nullptr)
  {
    return;
  }
  released_objects[ring_index()]
      .push(gpu::Object{.image_view = view, .type = gpu::ObjectType::ImageView})
      .unwrap();
}

void GpuContext::release(gpu::Buffer buffer)
{
  if (buffer == nullptr)
  {
    return;
  }
  released_objects[ring_index()]
      .push(gpu::Object{.buffer = buffer, .type = gpu::ObjectType::Buffer})
      .unwrap();
}

void GpuContext::release(gpu::BufferView view)
{
  if (view == nullptr)
  {
    return;
  }
  released_objects[ring_index()]
      .push(
          gpu::Object{.buffer_view = view, .type = gpu::ObjectType::BufferView})
      .unwrap();
}

void GpuContext::release(gpu::DescriptorSetLayout layout)
{
  if (layout == nullptr)
  {
    return;
  }
  released_objects[ring_index()]
      .push(gpu::Object{.descriptor_set_layout = layout,
                        .type = gpu::ObjectType::DescriptorSetLayout})
      .unwrap();
}

void GpuContext::release(gpu::DescriptorSet set)
{
  if (set == nullptr)
  {
    return;
  }
  released_objects[ring_index()]
      .push(gpu::Object{.descriptor_set = set,
                        .type           = gpu::ObjectType::DescriptorSet})
      .unwrap();
}

void GpuContext::release(gpu::Sampler sampler)
{
  if (sampler == nullptr)
  {
    return;
  }
  released_objects[ring_index()]
      .push(gpu::Object{.sampler = sampler, .type = gpu::ObjectType::Sampler})
      .unwrap();
}

static void uninit_objects(gpu::DeviceImpl d, Span<gpu::Object const> objects)
{
  for (gpu::Object obj : objects)
  {
    switch (obj.type)
    {
      case gpu::ObjectType::Image:
        d->uninit_image(d.self, obj.image);
        break;
      case gpu::ObjectType::ImageView:
        d->uninit_image_view(d.self, obj.image_view);
        break;
      case gpu::ObjectType::Buffer:
        d->uninit_buffer(d.self, obj.buffer);
        break;
      case gpu::ObjectType::BufferView:
        d->uninit_buffer_view(d.self, obj.buffer_view);
        break;
      case gpu::ObjectType::Sampler:
        d->uninit_sampler(d.self, obj.sampler);
        break;
      case gpu::ObjectType::DescriptorSet:
        d->uninit_descriptor_set(d.self, obj.descriptor_set);
        break;
      case gpu::ObjectType::DescriptorSetLayout:
        d->uninit_descriptor_set_layout(d.self, obj.descriptor_set_layout);
        break;
      default:
        UNREACHABLE();
    }
  }
}

void GpuContext::idle_reclaim()
{
  device->wait_idle(device.self).unwrap();
  for (auto &objects : released_objects)
  {
    uninit_objects(device, span(objects));
    objects.clear();
  }
}

void GpuContext::begin_frame(gpu::Swapchain swapchain)
{
  device->begin_frame(device.self, swapchain).unwrap();
  uninit_objects(device, span(released_objects[ring_index()]));
  released_objects[ring_index()].clear();

  gpu::CommandEncoderImpl enc = encoder();

  enc->clear_color_image(
      enc.self, screen_fb.color.image, gpu::Color{.float32 = {0, 0, 0, 0}},
      span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Color,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = 1,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1}}));

  for (Framebuffer const &f : scratch_fbs)
  {
    enc->clear_color_image(
        enc.self, f.color.image, gpu::Color{.float32 = {0, 0, 0, 0}},
        span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Color,
                                         .first_mip_level   = 0,
                                         .num_mip_levels    = 1,
                                         .first_array_layer = 0,
                                         .num_array_layers  = 1}}));
  }

  enc->clear_depth_stencil_image(
      enc.self, screen_fb.depth_stencil.image,
      gpu::DepthStencil{.depth = 0, .stencil = 0},
      span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Depth |
                                                  gpu::ImageAspects::Stencil,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = 1,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1}}));

  for (Framebuffer const &f : scratch_fbs)
  {
    enc->clear_depth_stencil_image(
        enc.self, f.depth_stencil.image,
        gpu::DepthStencil{.depth = 0, .stencil = 0},
        span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Depth |
                                                    gpu::ImageAspects::Stencil,
                                         .first_mip_level   = 0,
                                         .num_mip_levels    = 1,
                                         .first_array_layer = 0,
                                         .num_array_layers  = 1}}));
  }
}

void GpuContext::end_frame(gpu::Swapchain swapchain)
{
  gpu::CommandEncoderImpl enc = encoder();
  if (swapchain != nullptr)
  {
    gpu::SwapchainState swapchain_state =
        device->get_swapchain_state(device.self, swapchain).unwrap();

    if (swapchain_state.current_image.is_some())
    {
      enc->blit_image(
          enc.self, screen_fb.color.image,
          swapchain_state.images[swapchain_state.current_image.unwrap()],
          span({gpu::ImageBlit{
              .src_layers  = {.aspects           = gpu::ImageAspects::Color,
                              .mip_level         = 0,
                              .first_array_layer = 0,
                              .num_array_layers  = 1},
              .src_offsets = {{0, 0, 0},
                              {screen_fb.extent.x, screen_fb.extent.y, 1}},
              .dst_layers  = {.aspects           = gpu::ImageAspects::Color,
                              .mip_level         = 0,
                              .first_array_layer = 0,
                              .num_array_layers  = 1},
              .dst_offsets = {{0, 0, 0},
                              {swapchain_state.extent.x,
                               swapchain_state.extent.y, 1}}}}),
          gpu::Filter::Linear);
    }
  }
  device->submit_frame(device.self, swapchain).unwrap();
}

void SSBO::uninit(GpuContext &ctx)
{
  ctx.device->uninit_descriptor_set(ctx.device.self, descriptor);
  ctx.device->uninit_buffer(ctx.device.self, buffer);
}

void SSBO::reserve(GpuContext &ctx, u64 p_size)
{
  p_size = max(p_size, (u64) 1);
  if (buffer != nullptr && size >= p_size)
  {
    return;
  }

  ctx.device->uninit_buffer(ctx.device.self, buffer);

  buffer = ctx.device
               ->create_buffer(
                   ctx.device.self,
                   gpu::BufferInfo{.label       = label,
                                   .size        = p_size,
                                   .host_mapped = true,
                                   .usage = gpu::BufferUsage::TransferSrc |
                                            gpu::BufferUsage::TransferDst |
                                            gpu::BufferUsage::UniformBuffer |
                                            gpu::BufferUsage::StorageBuffer})
               .unwrap();

  if (descriptor == nullptr)
  {
    descriptor =
        ctx.device->create_descriptor_set(ctx.device.self, ctx.ssbo_layout, {})
            .unwrap();
  }

  ctx.device->update_descriptor_set(
      ctx.device.self,
      gpu::DescriptorSetUpdate{
          .set     = descriptor,
          .binding = 0,
          .element = 0,
          .buffers = span({gpu::BufferBinding{
              .buffer = buffer, .offset = 0, .size = p_size}})});

  size = p_size;
}

void SSBO::copy(GpuContext &ctx, Span<u8 const> src)
{
  reserve(ctx, (u64) src.size());
  u8 *data = (u8 *) map(ctx);
  mem::copy(src, data);
  flush(ctx);
  unmap(ctx);
}

void *SSBO::map(GpuContext &ctx)
{
  return ctx.device->map_buffer_memory(ctx.device.self, buffer).unwrap();
}

void SSBO::unmap(GpuContext &ctx)
{
  ctx.device->unmap_buffer_memory(ctx.device.self, buffer);
}

void SSBO::flush(GpuContext &ctx)
{
  ctx.device
      ->flush_mapped_buffer_memory(ctx.device.self, buffer,
                                   gpu::MemoryRange{0, gpu::WHOLE_SIZE})
      .unwrap();
}

void SSBO::release(GpuContext &ctx)
{
  ctx.release(buffer);
  ctx.release(descriptor);
  buffer     = nullptr;
  size       = 0;
  descriptor = nullptr;
}

}        // namespace ash