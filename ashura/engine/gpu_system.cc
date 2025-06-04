/// SPDX-License-Identifier: MIT
#include "ashura/engine/gpu_system.h"
#include "ashura/std/sformat.h"
#include "ashura/std/str.h"
#include "ashura/std/trace.h"

namespace ash
{

GpuBuffer::GpuBuffer(Str label, gpu::BufferUsage usage) :
  label_{label},
  usage_{usage}
{
}

bool GpuBuffer::is_valid() const
{
  return buffer_ != nullptr;
}

void GpuBuffer::release(GpuSystem & gpu)
{
  gpu.release(buffer_);
  buffer_   = nullptr;
  size_     = 0;
  capacity_ = 0;
}

void GpuBuffer::reserve_exact(GpuSystem & gpu, u64 target_capacity, bool defer)
{
  target_capacity = max(target_capacity, (u64) 1);

  if (capacity_ == target_capacity)
  {
    return;
  }

  if (defer)
  {
    gpu.release(buffer_);
  }
  else
  {
    gpu.device_->uninit(buffer_);
  }

  buffer_ = gpu.device_
              ->create_buffer(gpu::BufferInfo{.label       = label_,
                                              .size        = target_capacity,
                                              .host_mapped = true,
                                              .usage       = usage_})
              .unwrap();

  size_     = 0;
  capacity_ = target_capacity;
}

void GpuBuffer::assign(GpuSystem & gpu, Span<u8 const> src)
{
  CHECK(src.size() <= capacity_, "");
  size_     = src.size();
  u8 * data = (u8 *) map(gpu);
  mem::copy(src, data);
  flush(gpu);
  unmap(gpu);
}

void * GpuBuffer::map(GpuSystem & gpu)
{
  return gpu.device_->map_buffer_memory(buffer_).unwrap();
}

void GpuBuffer::unmap(GpuSystem & gpu)
{
  gpu.device_->unmap_buffer_memory(buffer_);
}

void GpuBuffer::flush(GpuSystem & gpu)
{
  gpu.device_
    ->flush_mapped_buffer_memory(buffer_, gpu::MemoryRange{0, gpu::WHOLE_SIZE})
    .unwrap();
}

ShaderBuffer::ShaderBuffer(Str label, gpu::BufferUsage usage) :
  label_{label},
  usage_{usage}
{
}

bool ShaderBuffer::is_valid() const
{
  return buffer_ != nullptr;
}

void ShaderBuffer::release(GpuSystem & gpu)
{
  gpu.release(descriptor_);
  gpu.release(buffer_);
  buffer_     = nullptr;
  size_       = 0;
  capacity_   = 0;
  descriptor_ = nullptr;
}

void ShaderBuffer::reserve_exact(GpuSystem & gpu, u64 target_capacity,
                                 bool defer)
{
  target_capacity = max(target_capacity, (u64) 1);

  if (capacity_ == target_capacity)
  {
    return;
  }

  if (defer)
  {
    gpu.release(buffer_);
  }
  else
  {
    gpu.device_->uninit(buffer_);
  }

  buffer_ = gpu.device_
              ->create_buffer(gpu::BufferInfo{.label       = label_,
                                              .size        = target_capacity,
                                              .host_mapped = true,
                                              .usage       = usage_})
              .unwrap();

  if (descriptor_ == nullptr)
  {
    descriptor_ =
      gpu.device_
        ->create_descriptor_set(gpu::DescriptorSetInfo{
          .label = label_, .layout = gpu.sb_layout_, .variable_lengths = {}})
        .unwrap();
  }

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = descriptor_,
    .binding = 0,
    .element = 0,
    .buffers = span({gpu::BufferBinding{
      .buffer = buffer_, .offset = 0, .size = gpu::WHOLE_SIZE}}
      )
  });

  size_     = 0;
  capacity_ = target_capacity;
}

void ShaderBuffer::assign(GpuSystem & gpu, Span<u8 const> src)
{
  CHECK(src.size() <= capacity_, "");
  u8 * data = (u8 *) map(gpu);
  mem::copy(src, data);
  flush(gpu);
  unmap(gpu);
}

void * ShaderBuffer::map(GpuSystem & gpu)
{
  return gpu.device_->map_buffer_memory(buffer_).unwrap();
}

void ShaderBuffer::unmap(GpuSystem & gpu)
{
  gpu.device_->unmap_buffer_memory(buffer_);
}

void ShaderBuffer::flush(GpuSystem & gpu)
{
  gpu.device_
    ->flush_mapped_buffer_memory(buffer_, gpu::MemoryRange{0, gpu::WHOLE_SIZE})
    .unwrap();
}

GpuQueries GpuQueries::create(AllocatorRef allocator, gpu::Device & dev,
                              f32 time_period, u32 num_timespans,
                              u32 num_statistics)
{
  auto timestamps = dev.create_timestamp_query(num_timespans * 2).unwrap();
  auto stats      = dev.create_statistics_query(num_statistics).unwrap();

  return GpuQueries{allocator,     time_period, timestamps,
                    num_timespans, stats,       num_statistics};
}

void GpuQueries::uninit(gpu::Device & dev)
{
  dev.uninit(timestamps_);
  dev.uninit(statistics_);
}

void GpuQueries::begin_frame(gpu::Device & dev, gpu::CommandEncoder & enc)
{
  cpu_timestamps_.clear();
  cpu_statistics_.clear();

  u32 const num_frame_timestamps = timespan_labels_.size() * 2;
  u32 const num_frame_stats      = statistics_labels_.size();

  dev
    .get_timestamp_query_result(timestamps_, Slice32{0, num_frame_timestamps},
                                cpu_timestamps_)
    .unwrap();
  dev
    .get_statistics_query_result(statistics_, Slice32{0, num_frame_stats},
                                 cpu_statistics_)
    .unwrap();

  enc.reset_timestamp_query(timestamps_, Slice32{0, timespans_capacity_ * 2});
  enc.reset_statistics_query(statistics_, Slice32{0, statistics_capacity_});

  for (auto [i, label] : enumerate<u32>(timespan_labels_))
  {
    u64 const         ts0 = cpu_timestamps_[i * 2];
    u64 const         ts1 = cpu_timestamps_[i * 2 + 1];
    nanoseconds const t0{static_cast<nanoseconds::rep>(ts0 * time_period_)};
    nanoseconds const t1{static_cast<nanoseconds::rep>(ts1 * time_period_)};

    TraceRecord record{.label = label, .begin = t0, .end = t1};

    trace_sink->trace(TraceEvent{.label = "gpu.timeline"}, span({record}));
  }

  for (auto const [i, label] : enumerate<u32>(statistics_labels_))
  {
    gpu::PipelineStatistics const & stat      = cpu_statistics_[i];
    Tuple<Str, TraceRecord> const   records[] = {
      {"gpu.input_assembly_vertices"_str,
       {.label = label, .i = (i64) stat.input_assembly_vertices}    },
      {"gpu.vertex_shader_invocations"_str,
       {.label = label, .i = (i64) stat.vertex_shader_invocations}  },
      {"gpu.clipping_invocations"_str,
       {.label = label, .i = (i64) stat.clipping_invocations}       },
      {"gpu.clipping_primitives"_str,
       {.label = label, .i = (i64) stat.clipping_primitives}        },
      {"gpu.fragment_shader_invocations"_str,
       {.label = label, .i = (i64) stat.fragment_shader_invocations}},
      {"gpu.compute_shader_invocations"_str,
       {.label = label, .i = (i64) stat.compute_shader_invocations} }
    };

    for (auto const & [event, record] : records)
    {
      trace_sink->trace(TraceEvent{.label = event}, span({record}));
    }
  }

  timespan_labels_.clear();
  statistics_labels_.clear();
}

Option<u32> GpuQueries::begin_timespan(gpu::CommandEncoder & enc, Str label)
{
  auto const id = timespan_labels_.size32();

  if (id + 1 > timespans_capacity_)
  {
    return none;
  }

  timespan_labels_.push(label).unwrap();

  enc.write_timestamp(timestamps_, gpu::PipelineStages::BottomOfPipe, id * 2);

  return id;
}

void GpuQueries::end_timespan(gpu::CommandEncoder & enc, u32 id)
{
  enc.write_timestamp(timestamps_, gpu::PipelineStages::BottomOfPipe,
                      id * 2 + 1);
}

Option<u32> GpuQueries::begin_statistics(gpu::CommandEncoder & enc, Str label)
{
  auto const id = statistics_labels_.size32();

  if (id + 1 > statistics_capacity_)
  {
    return none;
  }

  statistics_labels_.push(label).unwrap();

  enc.begin_statistics(statistics_, id);

  return id;
}

void GpuQueries::end_statistics(gpu::CommandEncoder & enc, u32 id)
{
  enc.end_statistics(statistics_, id);
}

u32 FrameGraph::push_ssbo(Span<u8 const> data)
{
  CHECK(!uploaded_, "");
  auto const offset = sb_data_.size();
  sb_data_.extend(data).unwrap();
  auto const size = data.size();
  auto const idx  = sb_entries_.size();
  CHECK(sb_data_.size() <= U32_MAX, "");
  sb_entries_.push(Slice32{(u32) offset, (u32) size}).unwrap();
  auto const aligned_size =
    align_offset<usize>(gpu::BUFFER_OFFSET_ALIGNMENT, sb_data_.size());
  sb_data_.resize_uninit(aligned_size).unwrap();

  return (u32) idx;
}

Tuple<StructuredBuffer, Slice32> FrameGraph::get_structured_buffer(u32 id)
{
  CHECK(uploaded_, "");
  Slice32 slice = sb_entries_.try_get(id).unwrap();
  return {frame_data_[ring_index_].sb, slice};
}

void FrameGraph::add_pass(Pass pass)
{
  passes_.push(std::move(pass)).unwrap();
}

void FrameGraph::FrameData::release(GpuSystem & gpu)
{
  sb.release(gpu);
  staging.release(gpu);
}

void FrameGraph::execute(GpuSystem & gpu)
{
  auto & f = frame_data_[ring_index_];

  // these buffers are expected to be very large so we should reset them on every frame when they aren't being used
  f.staging.reserve_exact(gpu, staging_data_.size(), false);
  f.staging.assign(gpu, staging_data_);
  staging_data_.reset();

  f.sb.reserve_exact(gpu, sb_data_.size(), false);
  f.sb.assign(gpu, sb_data_);
  sb_data_.reset();

  uploaded_ = true;

  auto const timespan = gpu.begin_timespan("gpu.frame"_str);

  {
    auto const timespan = gpu.begin_timespan("gpu.uploads"_str);
    for (auto const & upload : uploads_)
    {
      upload.encoder(gpu.encoder(), f.staging.buffer_, upload.slice);
    }
    timespan.match([&](auto i) { gpu.end_timespan(i); });
  }

  for (auto const & task : tasks_)
  {
    task();
  }

  {
    auto const timespan = gpu.begin_timespan("gpu.passes"_str);
    for (auto const & pass : passes_)
    {
      auto const timespan = gpu.begin_timespan(pass.label);
      auto const stat     = gpu.begin_statistics(pass.label);
      pass.encoder(*this, gpu.encoder());
      stat.match([&](auto i) { gpu.end_statistics(i); });
      timespan.match([&](auto i) { gpu.end_timespan(i); });
    }
    timespan.match([&](auto i) { gpu.end_timespan(i); });
  }

  timespan.match([&](auto i) { gpu.end_timespan(i); });

  ring_index_ = (ring_index_ + 1) % frame_data_.size32();
  uploaded_   = false;
  sb_entries_.clear();
  uploads_.clear();
  tasks_.clear();
  passes_.clear();
  arena_.reclaim();
}

void FrameGraph::acquire(GpuSystem & gpu)
{
  frame_data_.resize(gpu.buffering_).unwrap();
}

void FrameGraph::release(GpuSystem & gpu)
{
  for (FrameData & f : frame_data_)
  {
    f.release(gpu);
  }
}

static Option<gpu::Format> select_hdr_format(gpu::Device & dev)
{
  for (auto fmt : ColorTexture::HDR_FORMATS)
  {
    gpu::FormatProperties props = dev.get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, ColorTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

static Option<gpu::Format> select_sdr_format(gpu::Device & dev)
{
  for (auto fmt : ColorTexture::SDR_FORMATS)
  {
    gpu::FormatProperties props = dev.get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, ColorTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

static Option<gpu::Format> select_depth_format(gpu::Device & dev)
{
  for (auto fmt : DepthStencilTexture::FORMATS)
  {
    gpu::FormatProperties props = dev.get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, DepthStencilTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

GpuSystem GpuSystem::create(AllocatorRef allocator, gpu::Device & device,
                            Span<u8 const> pipeline_cache_data, bool use_hdr,
                            u32 buffering, gpu::SampleCount sample_count,
                            Vec2U initial_extent)
{
  CHECK(buffering <= gpu::MAX_FRAME_BUFFERING, "");
  CHECK(initial_extent.x > 0 && initial_extent.y > 0, "");

  gpu::Format color_fmt = gpu::Format::Undefined;

  if (use_hdr)
  {
    select_hdr_format(device).match(
      [&](auto fmt) { color_fmt = fmt; },
      [&]() {
        warn("HDR mode requested but Device does not support "
             "HDR formats, trying SDR formats"_str);

        color_fmt = select_sdr_format(device).unwrap(
          "Device doesn't support any known color format"_str);
      });
  }
  else
  {
    color_fmt = select_sdr_format(device).unwrap(
      "Device doesn't support any known color format"_str);
  }

  gpu::Format depth_fmt = select_depth_format(device).unwrap(
    "Device doesn't support any known depth-stencil formats"_str);

  trace("Selected color format: {}"_str, color_fmt);

  trace("Selected depth stencil format: {}"_str, depth_fmt);

  gpu::PipelineCache pipeline_cache =
    device
      .create_pipeline_cache(gpu::PipelineCacheInfo{
        .label        = "Pipeline Cache"_str,
        .initial_data = pipeline_cache_data,
      })
      .unwrap();

  gpu::DescriptorSetLayout ubo_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "ConstantBuffer Layout"_str,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynamicUniformBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  gpu::DescriptorSetLayout ssbo_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "StructuredBuffer Layout"_str,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynamicStorageBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  gpu::DescriptorSetLayout textures_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
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
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label = "Samplers Layout"_str,
        .bindings =
          span({gpu::DescriptorBindingInfo{.type = gpu::DescriptorType::Sampler,
                                           .count = NUM_SAMPLER_SLOTS,
                                           .is_variable_length = true}}
          )
  })
      .unwrap();

  gpu::DescriptorSet textures =
    device
      .create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = "Texture Views"_str,
        .layout           = textures_layout,
        .variable_lengths = span<u32>({NUM_TEXTURE_SLOTS})})
      .unwrap();

  gpu::DescriptorSet samplers =
    device
      .create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = "Samplers"_str,
        .layout           = samplers_layout,
        .variable_lengths = span<u32>({NUM_SAMPLER_SLOTS})})
      .unwrap();

  gpu::Image default_image =
    device
      .create_image(gpu::ImageInfo{
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

  InplaceVec<GpuQueries, gpu::MAX_FRAME_BUFFERING> queries;

  for (u32 i = 0; i < buffering; i++)
  {
    queries
      .push(GpuQueries::create(allocator, device,
                               device.get_properties().timestamp_period,
                               NUM_FRAME_TIMESPANS, NUM_FRAME_STATISTICS))
      .unwrap();
  }

  GpuSystem gpu{allocator,
                device,
                device.get_properties(),
                pipeline_cache,
                buffering,
                sample_count,
                color_fmt,
                depth_fmt,
                ubo_layout,
                ssbo_layout,
                textures_layout,
                samplers_layout,
                textures,
                samplers,
                default_image,
                {},
                std::move(released_objects),
                FrameGraph{allocator},
                std::move(queries)};

  {
    static constexpr Tuple<Str, TextureId, gpu::ComponentMapping> mappings[] = {
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

    for (auto const [mapping, view] : zip(mappings, gpu.default_image_views_))
    {
      view = device
               .create_image_view(
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

      CHECK(mapping.v1 == gpu.alloc_texture_id(view), "");
    }
  }

  {
    static constexpr SamplerId default_ids[NUM_DEFAULT_SAMPLERS] = {
      SamplerId::LinearBlack, SamplerId::NearestBlack, SamplerId::LinearClamped,
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
      CHECK(gpu.create_sampler(info).id == expected_id, "");
    }
  }

  gpu.frame_graph_.acquire(gpu);
  gpu.recreate_framebuffers(initial_extent);

  return gpu;
}

void GpuSystem::shutdown(Vec<u8> & cache)
{
  frame_graph_.release(*this);
  device_->get_pipeline_cache_data(pipeline_cache_, cache).unwrap();
  release(textures_);
  for (gpu::ImageView v : default_image_views_)
  {
    release(v);
  }
  release(default_image_);
  release(samplers_);
  release(cb_layout_);
  release(sb_layout_);
  release(textures_layout_);
  release(samplers_layout_);
  release(fb_);
  for (auto & tex : scratch_color_)
  {
    release(tex);
  }
  for (auto & tex : scratch_depth_stencil_)
  {
    release(tex);
  }
  for (auto const & [_, sampler] : sampler_cache_)
  {
    release(sampler.sampler);
  }
  release(pipeline_cache_);

  textures_            = {};
  default_image_views_ = {};
  default_image_       = {};
  samplers_            = {};
  cb_layout_           = {};
  sb_layout_           = {};
  textures_layout_     = {};
  samplers_layout_     = {};
  fb_                  = {};
  fill(scratch_color_, ColorTexture{});
  fill(scratch_depth_stencil_, DepthStencilTexture{});
  sampler_cache_  = {};
  pipeline_cache_ = {};

  idle_reclaim();
}

static ColorTexture create_color_texture(GpuSystem & gpu, Vec2U new_extent,
                                         Str prefix, usize index)
{
  auto label =
    snformat<gpu::MAX_LABEL_SIZE>("Color Texture: {} [{}]"_str, prefix, index)
      .unwrap();

  gpu::ImageInfo info{
    .label  = label,
    .type   = gpu::ImageType::Type2D,
    .format = gpu.color_format_,
    .usage  = gpu::ImageUsage::ColorAttachment | gpu::ImageUsage::Sampled |
             gpu::ImageUsage::Storage | gpu::ImageUsage::TransferDst |
             gpu::ImageUsage::TransferSrc,
    .aspects = gpu::ImageAspects::Color,
    .extent{new_extent.x, new_extent.y, 1},
    .mip_levels   = 1,
    .array_layers = 1,
    .sample_count = gpu::SampleCount::C1
  };

  gpu::Image image = gpu.device_->create_image(info).unwrap();

  auto view_label = snformat<gpu::MAX_LABEL_SIZE>(
                      "Color Texture View: {} [{}]"_str, prefix, index)
                      .unwrap();

  gpu::ImageViewInfo view_info{.label             = view_label,
                               .image             = image,
                               .view_type         = gpu::ImageViewType::Type2D,
                               .view_format       = info.format,
                               .mapping           = {},
                               .aspects           = gpu::ImageAspects::Color,
                               .first_mip_level   = 0,
                               .num_mip_levels    = 1,
                               .first_array_layer = 0,
                               .num_array_layers  = 1};

  gpu::ImageView view = gpu.device_->create_image_view(view_info).unwrap();

  auto texture_label = snformat<gpu::MAX_LABEL_SIZE>(
                         "Color Texture Descriptor: {} [{}]"_str, prefix, index)
                         .unwrap();

  gpu::DescriptorSet texture = gpu.device_
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label            = texture_label,
                                   .layout           = gpu.textures_layout_,
                                   .variable_lengths = span<u32>({1})})
                                 .unwrap();

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = texture,
    .binding = 0,
    .element = 0,
    .images  = span({gpu::ImageBinding{.image_view = view}})});

  info.label      = {};
  view_info.label = {};

  return ColorTexture{.info      = info,
                      .view_info = view_info,
                      .image     = image,
                      .view      = view,
                      .texture   = texture};
}

static Option<ColorMsaaTexture> create_color_msaa_texture(GpuSystem & gpu,
                                                          Vec2U new_extent,
                                                          Str   prefix,
                                                          usize index)
{
  if (gpu.sample_count_ == gpu::SampleCount::C1)
  {
    return none;
  }

  auto label = snformat<gpu::MAX_LABEL_SIZE>("MSAA Color Texture: {} [{}]"_str,
                                             prefix, index)
                 .unwrap();

  gpu::ImageInfo info{
    .label  = label,
    .type   = gpu::ImageType::Type2D,
    .format = gpu.color_format_,
    .usage  = gpu::ImageUsage::ColorAttachment | gpu::ImageUsage::TransferSrc |
             gpu::ImageUsage::TransferDst,
    .aspects = gpu::ImageAspects::Color,
    .extent{new_extent.x, new_extent.y, 1},
    .mip_levels   = 1,
    .array_layers = 1,
    .sample_count = gpu.sample_count_
  };

  gpu::Image image = gpu.device_->create_image(info).unwrap();

  auto view_label = snformat<gpu::MAX_LABEL_SIZE>(
                      "MSAA Color Texture View: {} [{}]"_str, prefix, index)
                      .unwrap();

  gpu::ImageViewInfo view_info{.label             = view_label,
                               .image             = image,
                               .view_type         = gpu::ImageViewType::Type2D,
                               .view_format       = info.format,
                               .mapping           = {},
                               .aspects           = gpu::ImageAspects::Color,
                               .first_mip_level   = 0,
                               .num_mip_levels    = 1,
                               .first_array_layer = 0,
                               .num_array_layers  = 1};

  gpu::ImageView view = gpu.device_->create_image_view(view_info).unwrap();

  info.label      = {};
  view_info.label = {};

  return ColorMsaaTexture{
    .info = info, .view_info = view_info, .image = image, .view = view};
}

static DepthStencilTexture create_depth_texture(GpuSystem & gpu,
                                                Vec2U new_extent, Str prefix,
                                                usize index)
{
  auto label = snformat<gpu::MAX_LABEL_SIZE>(
                 "Depth-Stencil Texture: {} [{}]"_str, prefix, index)
                 .unwrap();

  gpu::ImageInfo info{
    .label  = label,
    .type   = gpu::ImageType::Type2D,
    .format = gpu.depth_format_,
    .usage  = gpu::ImageUsage::DepthStencilAttachment |
             gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
             gpu::ImageUsage::TransferSrc,
    .aspects = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
    .extent{new_extent.x, new_extent.y, 1},
    .mip_levels   = 1,
    .array_layers = 1,
    .sample_count = gpu::SampleCount::C1
  };

  gpu::Image image = gpu.device_->create_image(info).unwrap();

  auto view_label = snformat<gpu::MAX_LABEL_SIZE>(
                      "Depth Texture View - Depth: {} [{}]"_str, prefix, index)
                      .unwrap();

  gpu::ImageViewInfo view_info{.label             = view_label,
                               .image             = image,
                               .view_type         = gpu::ImageViewType::Type2D,
                               .view_format       = info.format,
                               .mapping           = {},
                               .aspects           = gpu::ImageAspects::Depth,
                               .first_mip_level   = 0,
                               .num_mip_levels    = 1,
                               .first_array_layer = 0,
                               .num_array_layers  = 1};

  gpu::ImageView view = gpu.device_->create_image_view(view_info).unwrap();

  auto stencil_view_label =
    snformat<gpu::MAX_LABEL_SIZE>("Depth Texture View - Stencil: {} [{}]"_str,
                                  prefix, index)
      .unwrap();

  gpu::ImageViewInfo stencil_view_info{.label     = stencil_view_label,
                                       .image     = image,
                                       .view_type = gpu::ImageViewType::Type2D,
                                       .view_format = info.format,
                                       .mapping     = {},
                                       .aspects = gpu::ImageAspects::Stencil,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = 1,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1};

  gpu::ImageView stencil_view =
    gpu.device_->create_image_view(stencil_view_info).unwrap();

  auto texture_label =
    snformat<gpu::MAX_LABEL_SIZE>(
      "Depth Texture Descriptor - Depth: {} [{}]"_str, prefix, index)
      .unwrap();

  gpu::DescriptorSet texture = gpu.device_
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label            = texture_label,
                                   .layout           = gpu.textures_layout_,
                                   .variable_lengths = span<u32>({1})})
                                 .unwrap();

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = texture,
    .binding = 0,
    .element = 0,
    .images  = span({gpu::ImageBinding{.image_view = view}})});

  auto stencil_texture_label =
    snformat<gpu::MAX_LABEL_SIZE>(
      "Depth Texture Descriptor - Stencil: {} [{}]"_str, prefix, index)
      .unwrap();

  gpu::DescriptorSet stencil_texture =
    gpu.device_
      ->create_descriptor_set(
        gpu::DescriptorSetInfo{.label            = stencil_texture_label,
                               .layout           = gpu.textures_layout_,
                               .variable_lengths = span<u32>({1})})
      .unwrap();

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set     = stencil_texture,
    .binding = 0,
    .element = 0,
    .images  = span({gpu::ImageBinding{.image_view = stencil_view}})});

  info.label              = {};
  view_info.label         = {};
  stencil_view_info.label = {};

  return DepthStencilTexture{.info              = info,
                             .view_info         = view_info,
                             .stencil_view_info = stencil_view_info,
                             .image             = image,
                             .view              = view,
                             .stencil_view      = stencil_view,
                             .texture           = texture,
                             .stencil_texture   = stencil_texture};
}

void GpuSystem::recreate_framebuffers(Vec2U new_extent)
{
  idle_reclaim();
  release(fb_.color);
  release(fb_.depth_stencil);
  fb_.color_msaa.match([&](auto & c) { release(c); });

  for (auto & scratch_color : scratch_color_)
  {
    release(scratch_color);
  }

  for (auto & scratch_depth : scratch_depth_stencil_)
  {
    release(scratch_depth);
  }

  fb_.color         = create_color_texture(*this, new_extent, "Main"_str, 0);
  fb_.depth_stencil = create_depth_texture(*this, new_extent, "Main"_str, 0);
  fb_.color_msaa = create_color_msaa_texture(*this, new_extent, "Main"_str, 0);

  for (auto [i, tex] : enumerate(scratch_color_))
  {
    tex = create_color_texture(*this, new_extent, "Scratch"_str, i);
  }

  for (auto [i, tex] : enumerate(scratch_depth_stencil_))
  {
    tex = create_depth_texture(*this, new_extent, "Scratch"_str, i);
  }
}

gpu::CommandEncoder & GpuSystem::encoder()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return *ctx.encoders[ctx.ring_index];
}

u32 GpuSystem::ring_index()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return ctx.ring_index;
}

gpu::FrameId GpuSystem::frame_id()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return ctx.current;
}

gpu::FrameId GpuSystem::tail_frame_id()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return ctx.tail;
}

Sampler GpuSystem::create_sampler(gpu::SamplerInfo const & info)
{
  Option cached = sampler_cache_.try_get(info);
  if (cached)
  {
    return cached.v();
  }

  gpu::Sampler sampler = device_->create_sampler(info).unwrap();
  SamplerId    id      = alloc_sampler_id(sampler);

  Sampler entry{.id = id, .sampler = sampler};

  sampler_cache_.insert(info, entry).unwrap();

  return entry;
}

TextureId GpuSystem::alloc_texture_id(gpu::ImageView view)
{
  usize i = texture_slots_.view().find_clear_bit();
  CHECK(i < size_bits(texture_slots_), "Out of Texture Slots");
  texture_slots_.set_bit(i);

  add_task([i, this, view]() {
    device_->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set     = textures_,
      .binding = 0,
      .element = (u32) i,
      .images  = span({gpu::ImageBinding{.image_view = view}})});
  });

  return TextureId{(u32) i};
}

void GpuSystem::release_texture_id(TextureId id)
{
  u32 const i = (u32) id;
  texture_slots_.clear_bit(i);

  add_task([i, this]() {
    device_->unbind_descriptor_set(textures_, 0, Slice32{i, 1});
  });
}

SamplerId GpuSystem::alloc_sampler_id(gpu::Sampler sampler)
{
  usize i = sampler_slots_.view().find_clear_bit();
  CHECK(i < size_bits(sampler_slots_), "Out of Sampler Slots");
  sampler_slots_.set_bit(i);

  add_task([i, this, sampler]() {
    device_->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set     = samplers_,
      .binding = 0,
      .element = (u32) i,
      .images  = span({gpu::ImageBinding{.sampler = sampler}})});
  });

  return SamplerId{(u32) i};
}

void GpuSystem::release_sampler_id(SamplerId id)
{
  u32 const i = (u32) id;
  sampler_slots_.clear_bit(i);

  add_task([i, this]() {
    device_->unbind_descriptor_set(samplers_, 0, Slice32{i, 1});
  });
}

void GpuSystem::release(gpu::Object object)
{
  if (object.v0_ == nullptr)
  {
    return;
  }

  released_objects_[ring_index()].push(object).unwrap();
}

void GpuSystem::release(ColorTexture tex)
{
  release(tex.texture);
  release(tex.view);
  release(tex.image);
}

void GpuSystem::release(ColorMsaaTexture tex)
{
  release(tex.view);
  release(tex.image);
}

void GpuSystem::release(DepthStencilTexture tex)
{
  release(tex.texture);
  release(tex.stencil_texture);
  release(tex.view);
  release(tex.stencil_view);
  release(tex.image);
}

void GpuSystem::release(Framebuffer tex)
{
  release(tex.color);
  tex.color_msaa.match([&](auto const & f) { release(f); });
  release(tex.depth_stencil);
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
              [&](gpu::Swapchain) { CHECK_UNREACHABLE(); });
  }
}

void GpuSystem::idle_reclaim()
{
  device_->wait_idle().unwrap();
  for (auto & objects : released_objects_)
  {
    uninit_objects(*device_, objects);
    objects.clear();
  }
}

void GpuSystem::frame(gpu::Swapchain swapchain)
{
  ScopeTrace trace;
  device_->begin_frame(swapchain).unwrap();
  uninit_objects(*device_, released_objects_[ring_index()]);
  released_objects_[ring_index()].clear();

  auto & enc = encoder();

  queries_[ring_index()].begin_frame(*device_, enc);

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

  clear_color(fb_.color.image);
  fb_.color_msaa.match([&](auto & c) { clear_color(c.image); });
  clear_depth(fb_.depth_stencil.image);

  frame_graph_.execute(*this);

  if (swapchain != nullptr)
  {
    auto swapchain_state = device_->get_swapchain_state(swapchain).unwrap();

    swapchain_state.current_image.match([&](auto idx) {
      enc.blit_image(
        fb_.color.image, swapchain_state.images[idx],
        span({
          gpu::ImageBlit{
                         .src_layers = {.aspects           = gpu::ImageAspects::Color,
                           .mip_level         = 0,
                           .first_array_layer = 0,
                           .num_array_layers  = 1},
                         .src_area   = {{0, 0, 0}, fb_.extent()},
                         .dst_layers = {.aspects           = gpu::ImageAspects::Color,
                           .mip_level         = 0,
                           .first_array_layer = 0,
                           .num_array_layers  = 1},
                         .dst_area   = {{0, 0, 0}, vec3u(swapchain_state.extent, 1)}}
      }),
        gpu::Filter::Linear);
    });
  }

  device_->submit_frame(swapchain).unwrap();
}

Option<u32> GpuSystem::begin_timespan(Str label)
{
  return queries_[ring_index()].begin_timespan(encoder(), label);
}

void GpuSystem::end_timespan(u32 id)
{
  queries_[ring_index()].end_timespan(encoder(), id);
}

Option<u32> GpuSystem::begin_statistics(Str label)
{
  return queries_[ring_index()].begin_statistics(encoder(), label);
}

void GpuSystem::end_statistics(u32 id)
{
  return queries_[ring_index()].end_statistics(encoder(), id);
}

}    // namespace ash
