/// SPDX-License-Identifier: MIT
#include "ashura/engine/gpu_system.h"

namespace ash
{

GpuSystem GpuSystem::create(AllocatorImpl allocator, gpu::Device * device,
                            bool use_hdr, u32 buffering,
                            gpu::SampleCount sample_count,
                            gpu::Extent      initial_extent)
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
        device->get_format_properties(HDR_COLOR_FORMATS[sel_hdr_color_format])
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
        device->get_format_properties(SDR_COLOR_FORMATS[sel_sdr_color_format])
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
        ->get_format_properties(DEPTH_STENCIL_FORMATS[sel_depth_stencil_format])
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
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "UBO Layout"_str,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynamicUniformBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  gpu::DescriptorSetLayout ssbo_layout =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "SSBO Layout"_str,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynamicStorageBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  gpu::DescriptorSetLayout textures_layout =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "Textures Layout"_str,
        .bindings = span(
          {gpu::DescriptorBindingInfo{.type = gpu::DescriptorType::SampledImage,
                                      .count              = NUM_TEXTURE_SLOTS,
                                      .is_variable_length = true}}
            )
  })
      .unwrap();

  gpu::DescriptorSetLayout samplers_layout =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label = "Samplers Layout"_str,
        .bindings =
          span({gpu::DescriptorBindingInfo{.type = gpu::DescriptorType::Sampler,
                                           .count = NUM_SAMPLER_SLOTS,
                                           .is_variable_length = true}}
          )
  })
      .unwrap();

  gpu::DescriptorSet texture_views =
    device
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = "Texture Views"_str,
        .layout           = textures_layout,
        .variable_lengths = span<u32>({NUM_TEXTURE_SLOTS})})
      .unwrap();

  gpu::DescriptorSet samplers =
    device
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = "Samplers"_str,
        .layout           = samplers_layout,
        .variable_lengths = span<u32>({NUM_SAMPLER_SLOTS})})
      .unwrap();

  gpu::Image default_image =
    device
      ->create_image(gpu::ImageInfo{
        .label  = "Default Image"_str,
        .type   = gpu::ImageType::Type2D,
        .format = gpu::Format::B8G8R8A8_UNORM,
        .usage  = gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
                 gpu::ImageUsage::Storage | gpu::ImageUsage::Storage,
        .aspects      = gpu::ImageAspects::Color,
        .extent       = {1, 1, 1},
        .mip_levels   = 1,
        .array_layers = 1,
        .sample_count = gpu::SampleCount::C1
  })
      .unwrap();

  InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects;

  for (u32 i = 0; i < buffering; i++)
  {
    released_objects.push(Vec<gpu::Object>{allocator}).unwrap();
  }

  GpuSystem gpu{allocator,
                device,
                pipeline_cache,
                buffering,
                sample_count,
                color_format,
                depth_stencil_format,
                ubo_layout,
                ssbo_layout,
                textures_layout,
                samplers_layout,
                texture_views,
                samplers,
                default_image,
                {},
                std::move(released_objects)};

  {
    static constexpr Tuple<Span<char const>, TextureId, gpu::ComponentMapping>
      mappings[] = {
        {"Default White Texture"_str,
         TextureId::White,
         {.r = gpu::ComponentSwizzle::One,
          .g = gpu::ComponentSwizzle::One,
          .b = gpu::ComponentSwizzle::One,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Black Texture"_str,
         TextureId::Black,
         {.r = gpu::ComponentSwizzle::Zero,
          .g = gpu::ComponentSwizzle::Zero,
          .b = gpu::ComponentSwizzle::Zero,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Transparent Texture"_str,
         TextureId::Transparent,
         {.r = gpu::ComponentSwizzle::Zero,
          .g = gpu::ComponentSwizzle::Zero,
          .b = gpu::ComponentSwizzle::Zero,
          .a = gpu::ComponentSwizzle::Zero}},
        {"Default Alpha Texture"_str,
         TextureId::Alpha,
         {.r = gpu::ComponentSwizzle::Zero,
          .g = gpu::ComponentSwizzle::Zero,
          .b = gpu::ComponentSwizzle::Zero,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Red Texture"_str,
         TextureId::Red,
         {.r = gpu::ComponentSwizzle::One,
          .g = gpu::ComponentSwizzle::Zero,
          .b = gpu::ComponentSwizzle::Zero,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Green Texture"_str,
         TextureId::Green,
         {.r = gpu::ComponentSwizzle::Zero,
          .g = gpu::ComponentSwizzle::One,
          .b = gpu::ComponentSwizzle::Zero,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Blue Texture"_str,
         TextureId::Blue,
         {.r = gpu::ComponentSwizzle::Zero,
          .g = gpu::ComponentSwizzle::Zero,
          .b = gpu::ComponentSwizzle::One,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Magenta Texture"_str,
         TextureId::Magenta,
         {.r = gpu::ComponentSwizzle::One,
          .g = gpu::ComponentSwizzle::Zero,
          .b = gpu::ComponentSwizzle::One,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Cyan Texture"_str,
         TextureId::Cyan,
         {.r = gpu::ComponentSwizzle::Zero,
          .g = gpu::ComponentSwizzle::One,
          .b = gpu::ComponentSwizzle::One,
          .a = gpu::ComponentSwizzle::One} },
        {"Default Yellow Texture"_str,
         TextureId::Yellow,
         {.r = gpu::ComponentSwizzle::One,
          .g = gpu::ComponentSwizzle::One,
          .b = gpu::ComponentSwizzle::Zero,
          .a = gpu::ComponentSwizzle::One} }
    };

    static_assert(size(mappings) == NUM_DEFAULT_TEXTURES);

    for (auto const [mapping, view] : zip(mappings, gpu.default_image_views))
    {
      view = device
               ->create_image_view(
                 gpu::ImageViewInfo{.label       = mapping.v0,
                                    .image       = default_image,
                                    .view_type   = gpu::ImageViewType::Type2D,
                                    .view_format = gpu::Format::B8G8R8A8_UNORM,
                                    .mapping     = mapping.v2,
                                    .aspects     = gpu::ImageAspects::Color,
                                    .first_mip_level   = 0,
                                    .num_mip_levels    = 1,
                                    .first_array_layer = 0,
                                    .num_array_layers  = 1})
               .unwrap();

      CHECK(mapping.v1 == gpu.alloc_texture_id(view));
    }
  }

  {
    static constexpr SamplerId default_ids[NUM_DEFAULT_SAMPLERS] = {
      SamplerId::Linear, SamplerId::Nearest, SamplerId::LinearClamped,
      SamplerId::NearestClamped};

    static constexpr gpu::SamplerInfo infos[NUM_DEFAULT_SAMPLERS] = {
      {.label                    = "Linear+Repeat Sampler"_str,
       .mag_filter               = gpu::Filter::Linear,
       .min_filter               = gpu::Filter::Linear,
       .mip_map_mode             = gpu::SamplerMipMapMode::Linear,
       .address_mode_u           = gpu::SamplerAddressMode::Repeat,
       .address_mode_v           = gpu::SamplerAddressMode::Repeat,
       .address_mode_w           = gpu::SamplerAddressMode::Repeat,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false},
      {.label                    = "Nearest+Repeat Sampler"_str,
       .mag_filter               = gpu::Filter::Nearest,
       .min_filter               = gpu::Filter::Nearest,
       .mip_map_mode             = gpu::SamplerMipMapMode::Nearest,
       .address_mode_u           = gpu::SamplerAddressMode::Repeat,
       .address_mode_v           = gpu::SamplerAddressMode::Repeat,
       .address_mode_w           = gpu::SamplerAddressMode::Repeat,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false},
      {.label                    = "Linear+EdgeClamped Sampler"_str,
       .mag_filter               = gpu::Filter::Linear,
       .min_filter               = gpu::Filter::Linear,
       .mip_map_mode             = gpu::SamplerMipMapMode::Linear,
       .address_mode_u           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_v           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_w           = gpu::SamplerAddressMode::ClampToEdge,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false},
      {.label                    = "Nearest+EdgeClamped Sampler"_str,
       .mag_filter               = gpu::Filter::Nearest,
       .min_filter               = gpu::Filter::Nearest,
       .mip_map_mode             = gpu::SamplerMipMapMode::Nearest,
       .address_mode_u           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_v           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_w           = gpu::SamplerAddressMode::ClampToEdge,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false}
    };

    for (auto const [expected_id, info] : zip(default_ids, infos))
    {
      CHECK(gpu.create_sampler(info).id == expected_id);
    }
  }

  gpu.recreate_framebuffers(initial_extent);

  return gpu;
}

static void recreate_framebuffer(GpuSystem & gpu, Framebuffer & fb,
                                 gpu::Extent new_extent)
{
  gpu.release(fb);
  fb                = Framebuffer{};
  gpu::Device & dev = *gpu.device;

  gpu::ImageInfo info{
    .label  = "Resolved Framebuffer Color Image"_str,
    .type   = gpu::ImageType::Type2D,
    .format = gpu.color_format,
    .usage  = gpu::ImageUsage::ColorAttachment | gpu::ImageUsage::Sampled |
             gpu::ImageUsage::Storage | gpu::ImageUsage::TransferDst |
             gpu::ImageUsage::TransferSrc,
    .aspects      = gpu::ImageAspects::Color,
    .extent       = gpu::Extent3D{new_extent.x, new_extent.y, 1},
    .mip_levels   = 1,
    .array_layers = 1,
    .sample_count = gpu::SampleCount::C1
  };

  gpu::Image image = dev.create_image(info).unwrap();

  gpu::ImageViewInfo view_info{.label =
                                 "Resolved Framebuffer Color Image View"_str,
                               .image             = image,
                               .view_type         = gpu::ImageViewType::Type2D,
                               .view_format       = info.format,
                               .mapping           = {},
                               .aspects           = gpu::ImageAspects::Color,
                               .first_mip_level   = 0,
                               .num_mip_levels    = 1,
                               .first_array_layer = 0,
                               .num_array_layers  = 1};

  gpu::ImageView view = dev.create_image_view(view_info).unwrap();

  gpu::DescriptorSet texture =
    dev
      .create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = "Resolved Framebuffer Color Image Descriptor"_str,
        .layout           = gpu.textures_layout,
        .variable_lengths = span<u32>({1})})
      .unwrap();

  dev.update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = texture,
    .binding = 0,
    .element = 0,
    .images  = span({gpu::ImageBinding{.image_view = view}})});

  fb.color = Framebuffer::Color{.info      = info,
                                .view_info = view_info,
                                .image     = image,
                                .view      = view,
                                .texture   = texture};

  if (gpu.sample_count != gpu::SampleCount::C1)
  {
    gpu::ImageInfo info{
      .label  = "Framebuffer MSAA Color Image"_str,
      .type   = gpu::ImageType::Type2D,
      .format = gpu.color_format,
      .usage = gpu::ImageUsage::ColorAttachment | gpu::ImageUsage::TransferSrc |
               gpu::ImageUsage::TransferDst,
      .aspects      = gpu::ImageAspects::Color,
      .extent       = gpu::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gpu.sample_count
    };

    gpu::Image image = dev.create_image(info).unwrap();

    gpu::ImageViewInfo view_info{.label =
                                   "Framebuffer MSAA Color Image View"_str,
                                 .image           = image,
                                 .view_type       = gpu::ImageViewType::Type2D,
                                 .view_format     = info.format,
                                 .mapping         = {},
                                 .aspects         = gpu::ImageAspects::Color,
                                 .first_mip_level = 0,
                                 .num_mip_levels  = 1,
                                 .first_array_layer = 0,
                                 .num_array_layers  = 1};

    gpu::ImageView view = dev.create_image_view(view_info).unwrap();

    fb.color_msaa = Framebuffer::ColorMsaa{
      .info = info, .view_info = view_info, .image = image, .view = view};
  }

  {
    gpu::ImageInfo info{
      .label  = "Framebuffer Depth & Stencil Image"_str,
      .type   = gpu::ImageType::Type2D,
      .format = gpu.depth_stencil_format,
      .usage  = gpu::ImageUsage::DepthStencilAttachment |
               gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
               gpu::ImageUsage::TransferSrc,
      .aspects      = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
      .extent       = gpu::Extent3D{new_extent.x, new_extent.y, 1},
      .mip_levels   = 1,
      .array_layers = 1,
      .sample_count = gpu::SampleCount::C1
    };

    gpu::Image image = dev.create_image(info).unwrap();

    gpu::ImageViewInfo view_info{.label = "Framebuffer Depth Image View"_str,
                                 .image = image,
                                 .view_type       = gpu::ImageViewType::Type2D,
                                 .view_format     = info.format,
                                 .mapping         = {},
                                 .aspects         = gpu::ImageAspects::Depth,
                                 .first_mip_level = 0,
                                 .num_mip_levels  = 1,
                                 .first_array_layer = 0,
                                 .num_array_layers  = 1};

    gpu::ImageView view = dev.create_image_view(view_info).unwrap();

    gpu::ImageViewInfo stencil_view_info{
      .label             = "Framebuffer Stencil Image View"_str,
      .image             = image,
      .view_type         = gpu::ImageViewType::Type2D,
      .view_format       = info.format,
      .mapping           = {},
      .aspects           = gpu::ImageAspects::Stencil,
      .first_mip_level   = 0,
      .num_mip_levels    = 1,
      .first_array_layer = 0,
      .num_array_layers  = 1};

    gpu::ImageView stencil_view =
      dev.create_image_view(stencil_view_info).unwrap();

    gpu::DescriptorSet texture =
      dev
        .create_descriptor_set(gpu::DescriptorSetInfo{
          .label            = "Framebuffer Depth Image Descriptor"_str,
          .layout           = gpu.textures_layout,
          .variable_lengths = span<u32>({1})})
        .unwrap();

    dev.update_descriptor_set(gpu::DescriptorSetUpdate{
      .set     = texture,
      .binding = 0,
      .element = 0,
      .images  = span({gpu::ImageBinding{.image_view = view}})});

    gpu::DescriptorSet stencil_texture =
      dev
        .create_descriptor_set(gpu::DescriptorSetInfo{
          .label            = "Framebuffer Stencil Image Descriptor"_str,
          .layout           = gpu.textures_layout,
          .variable_lengths = span<u32>({1})})
        .unwrap();

    dev.update_descriptor_set(gpu::DescriptorSetUpdate{
      .set     = stencil_texture,
      .binding = 0,
      .element = 0,
      .images  = span({gpu::ImageBinding{.image_view = stencil_view}})});

    fb.depth = Framebuffer::Depth{.info              = info,
                                  .view_info         = view_info,
                                  .stencil_view_info = stencil_view_info,
                                  .image             = image,
                                  .view              = view,
                                  .stencil_view      = stencil_view,
                                  .texture           = texture,
                                  .stencil_texture   = stencil_texture};
  }
}

void GpuSystem::recreate_framebuffers(gpu::Extent new_extent)
{
  idle_reclaim();
  recreate_framebuffer(*this, fb, new_extent);
  recreate_framebuffer(*this, scratch_fb, new_extent);
}

gpu::CommandEncoder & GpuSystem::encoder()
{
  gpu::FrameContext ctx = device->get_frame_context();
  return *ctx.encoders[ctx.ring_index];
}

u32 GpuSystem::ring_index()
{
  gpu::FrameContext ctx = device->get_frame_context();
  return ctx.ring_index;
}

gpu::FrameId GpuSystem::frame_id()
{
  gpu::FrameContext ctx = device->get_frame_context();
  return ctx.current;
}

gpu::FrameId GpuSystem::tail_frame_id()
{
  gpu::FrameContext ctx = device->get_frame_context();
  return ctx.tail;
}

CachedSampler GpuSystem::create_sampler(gpu::SamplerInfo const & info)
{
  OptionRef cached = sampler_cache.try_get(info);
  if (cached)
  {
    return cached.value();
  }

  gpu::Sampler sampler = device->create_sampler(info).unwrap();
  SamplerId    id      = alloc_sampler_id(sampler);

  CachedSampler entry{.sampler = sampler, .id = id};

  sampler_cache.insert(info, entry).unwrap();

  return entry;
}

TextureId GpuSystem::alloc_texture_id(gpu::ImageView view)
{
  usize i = find_clear_bit(texture_slots);
  CHECK_DESC(i < size_bits(texture_slots), "Out of Texture Slots");
  set_bit(texture_slots, i);
  device->wait_idle().unwrap();
  device->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = texture_views,
    .binding = 0,
    .element = (u32) i,
    .images  = span({gpu::ImageBinding{.image_view = view}})});
  return TextureId{(u32) i};
}

void GpuSystem::release_texture_id(TextureId id)
{
  clear_bit(texture_slots, (u32) id);
}

SamplerId GpuSystem::alloc_sampler_id(gpu::Sampler sampler)
{
  usize i = find_clear_bit(sampler_slots);
  CHECK_DESC(i < size_bits(sampler_slots), "Out of Sampler Slots");
  set_bit(sampler_slots, i);
  device->wait_idle().unwrap();
  device->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = samplers,
    .binding = 0,
    .element = (u32) i,
    .images  = span({gpu::ImageBinding{.sampler = sampler}})});
  return SamplerId{(u32) i};
}

void GpuSystem::release_sampler_id(SamplerId id)
{
  clear_bit(sampler_slots, (u32) id);
}

void GpuSystem::release(gpu::Object object)
{
  if (object.v18_ == nullptr)
  {
    return;
  }

  released_objects[ring_index()].push(object).unwrap();
}

static void uninit_objects(gpu::Device & d, Span<gpu::Object const> objects)
{
  for (gpu::Object obj : objects)
  {
    obj.match([](gpu::Instance *) { CHECK_UNREACHABLE(); },
              [](gpu::Device *) { CHECK_UNREACHABLE(); },
              [](gpu::CommandEncoder *) { CHECK_UNREACHABLE(); },
              [&](gpu::Buffer r) { d.uninit(r); },
              [&](gpu::BufferView r) { d.uninit(r); },
              [&](gpu::Image r) { d.uninit(r); },
              [&](gpu::ImageView r) { d.uninit(r); },
              [&](gpu::Sampler r) { d.uninit(r); },
              [&](gpu::Shader r) { d.uninit(r); },
              [&](gpu::DescriptorSetLayout r) { d.uninit(r); },
              [&](gpu::DescriptorSet r) { d.uninit(r); },
              [&](gpu::PipelineCache r) { d.uninit(r); },
              [&](gpu::ComputePipeline r) { d.uninit(r); },
              [&](gpu::GraphicsPipeline r) { d.uninit(r); },
              [&](gpu::TimeStampQuery r) { d.uninit(r); },
              [&](gpu::StatisticsQuery r) { d.uninit(r); },
              [&](gpu::Surface) { CHECK_UNREACHABLE(); },
              [&](gpu::Swapchain) { CHECK_UNREACHABLE(); },
              [](void *) { CHECK_UNREACHABLE(); });
  }
}

void GpuSystem::idle_reclaim()
{
  device->wait_idle().unwrap();
  for (auto & objects : released_objects)
  {
    uninit_objects(*device, objects);
    objects.clear();
  }
}

void GpuSystem::begin_frame(gpu::Swapchain swapchain)
{
  device->begin_frame(swapchain).unwrap();
  uninit_objects(*device, released_objects[ring_index()]);
  released_objects[ring_index()].clear();

  gpu::CommandEncoder & enc = encoder();

  auto clear_color = [&](gpu::Image image) {
    enc.clear_color_image(
      image,
      gpu::Color{
    },
      span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Color,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = 1,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1}}));
  };

  auto clear_depth = [&](gpu::Image image) {
    enc.clear_depth_stencil_image(
      image,
      gpu::DepthStencil{
    },
      span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Depth |
                                                  gpu::ImageAspects::Stencil,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = 1,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1}}));
  };

  clear_color(fb.color.image);
  fb.color_msaa.match(
    [&](Framebuffer::ColorMsaa const & c) { clear_color(c.image); });
  clear_depth(fb.depth.image);

  clear_color(scratch_fb.color.image);
  scratch_fb.color_msaa.match(
    [&](Framebuffer::ColorMsaa const & c) { clear_color(c.image); });
  clear_depth(scratch_fb.depth.image);
}

void GpuSystem::submit_frame(gpu::Swapchain swapchain)
{
  gpu::CommandEncoder & enc = encoder();
  if (swapchain != nullptr)
  {
    gpu::SwapchainState swapchain_state =
      device->get_swapchain_state(swapchain).unwrap();

    if (swapchain_state.current_image.is_some())
    {
      enc.blit_image(
        fb.color.image,
        swapchain_state.images[swapchain_state.current_image.unwrap()],
        span({
          gpu::ImageBlit{.src_layers  = {.aspects   = gpu::ImageAspects::Color,
                                         .mip_level = 0,
                                         .first_array_layer = 0,
                                         .num_array_layers  = 1},
                         .src_offsets = {{0, 0, 0}, fb.extent3()},
                         .dst_layers  = {.aspects   = gpu::ImageAspects::Color,
                                         .mip_level = 0,
                                         .first_array_layer = 0,
                                         .num_array_layers  = 1},
                         .dst_offsets = {{0, 0, 0},
                                         {swapchain_state.extent.x,
                                          swapchain_state.extent.y, 1}}}
      }),
        gpu::Filter::Linear);
    }
  }
  device->submit_frame(swapchain).unwrap();
}

void SSBO::uninit(GpuSystem & gpu)
{
  gpu.device->uninit(descriptor);
  gpu.device->uninit(buffer);
}

void SSBO::reserve(GpuSystem & gpu, u64 p_size)
{
  p_size = max(p_size, (u64) 1);
  if (buffer != nullptr && size >= p_size)
  {
    return;
  }

  gpu.device->uninit(buffer);

  buffer =
    gpu.device
      ->create_buffer(gpu::BufferInfo{.label       = label,
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
      gpu.device
        ->create_descriptor_set(gpu::DescriptorSetInfo{
          .label = label, .layout = gpu.ssbo_layout, .variable_lengths = {}})
        .unwrap();
  }

  gpu.device->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = descriptor,
    .binding = 0,
    .element = 0,
    .buffers = span(
      {gpu::BufferBinding{.buffer = buffer, .offset = 0, .size = p_size}}
        )
  });

  size = p_size;
}

void SSBO::copy(GpuSystem & gpu, Span<u8 const> src)
{
  reserve(gpu, (u64) src.size());
  u8 * data = (u8 *) map(gpu);
  mem::copy(src, data);
  flush(gpu);
  unmap(gpu);
}

void * SSBO::map(GpuSystem & gpu)
{
  return gpu.device->map_buffer_memory(buffer).unwrap();
}

void SSBO::unmap(GpuSystem & gpu)
{
  gpu.device->unmap_buffer_memory(buffer);
}

void SSBO::flush(GpuSystem & gpu)
{
  gpu.device
    ->flush_mapped_buffer_memory(buffer, gpu::MemoryRange{0, gpu::WHOLE_SIZE})
    .unwrap();
}

void SSBO::release(GpuSystem & gpu)
{
  gpu.release(buffer);
  gpu.release(descriptor);
  buffer     = nullptr;
  size       = 0;
  descriptor = nullptr;
}

}    // namespace ash
